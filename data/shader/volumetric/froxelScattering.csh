#ifdef X_TEXTURE_SHADOW_LOD
#extension GL_EXT_texture_shadow_lod : require
#endif
#include <../structures>


#define LOCAL_SIZE_X 8
#define LOCAL_SIZE_Y 8
#define LOCAL_SIZE_Z 1

#define VOLUME_DEPTH 128


#define M_PI 3.14159265359
#define EPSILON 0.0001f


layout(set = 3, binding = 2) uniform sampler3D i_VoxelGrid;
// OutPut 
layout(set = 3, binding = 0, rgba16f) uniform writeonly image3D o_VoxelGrid;

layout(std140, set = 3, binding = 4) uniform UniformBuffer{
	vec4 bias_near_far_pow;
	vec4 aniso_density_multipler_absorption;
	vec4 frustumRays[4];

	vec4 windDir_Speed;
	vec4 fogParams;
	bool enableJitter;
	bool enableTemperalAccumulating;
	Light light;

} uniforms;



float slice_distance(int z)
{
    float n = uniforms.bias_near_far_pow.y;
    float f = uniforms.bias_near_far_pow.z;

    return n * pow(f / n, (float(z) + 0.5f) / float(VOLUME_DEPTH));
}

// ------------------------------------------------------------------

float slice_thickness(int z)
{
    return slice_distance(z + 1) - slice_distance(z);
}

vec4 ScatterStep(int z, vec3 accumulatedLight, float accumulatedTransmittance, vec3 sliceLight, float sliceDensity)
{
	float thickness = slice_thickness(z);
	float delta = thickness /(uniforms.bias_near_far_pow.z - uniforms.bias_near_far_pow.y);
	float  sliceTransmittance = exp(-sliceDensity * delta);

	// Seb Hillaire's improved transmission by calculating an integral over slice depth instead of
	// constant per slice value. Light still constant per slice, but that's acceptable. See slide 28 of
	// Physically-based & Unified Volumetric Rendering in Frostbite
	// http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/
	vec3 sliceLightIntegral = sliceLight * (1.0 - sliceTransmittance) ;
	// vec3 sliceLightIntegral = sliceLight * sliceTransmittance;


	accumulatedLight += sliceLightIntegral * accumulatedTransmittance;
	accumulatedTransmittance *= sliceTransmittance;
	
	return vec4(accumulatedLight, accumulatedTransmittance);
}



layout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y, local_size_z = LOCAL_SIZE_Z) in;

void main()
{
	vec4 accum = vec4(0, 0, 0, 1);
	ivec3 pos = ivec3(ivec2(gl_WorkGroupID.xy) * ivec2(LOCAL_SIZE_X, LOCAL_SIZE_Y) + ivec2(gl_LocalInvocationID.xy), 0);

	uint steps = VOLUME_DEPTH;

	for(int z = 0; z < steps; z++)
	{
		pos.z = z;
		vec4 slice = texelFetch(i_VoxelGrid, pos, 0);

		accum = ScatterStep(pos.z, accum.rgb, accum.a, slice.rgb, slice.a);

		imageStore(o_VoxelGrid, pos, accum);
	}
}


