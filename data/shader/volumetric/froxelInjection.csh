#ifdef X_TEXTURE_SHADOW_LOD
#extension GL_EXT_texture_shadow_lod : require
#endif


#include <../globals.hsh>
#include <../structures>
#include <../common/ign.hsh>
#include <../common/convert.hsh>
#include <../common/utility.hsh>
#include <../common/random.hsh>
#include <../clouds/shadow.hsh>
#include <../lightculling.hsh>
#include <../shadow.hsh>

#include <fog.hsh>

#define VOXEL_GRID_SIZE_X 160 
#define VOXEL_GRID_SIZE_Y 90
#define VOXEL_GRID_SIZE_Z 128

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 2
#define LOCAL_SIZE_Z 16

#define BLUE_NOISE_TEXTURE_SIZE 256


#define M_PI 3.14159265359
#define EPSILON 0.0001f

layout(set = 3, binding = 0, rgba16f) writeonly uniform image3D o_VoxelGrid;
layout(set = 3, binding = 2) uniform sampler2DArrayShadow cascadeMaps;

layout(std140, set = 3, binding = 4) uniform UniformBuffer{
	vec4 bias_near_far_pow;
	vec4 aniso_density_multipler_absorption;
	vec4 frustumRays[4];

	vec4 windDir_Speed;
	vec4 fogParams;
	Light light;
} uniforms;

layout(set = 3, binding = 5) uniform sampler2D u_BlueNoise;
layout(set = 3, binding = 6) uniform sampler3D u_History;



vec3 id_to_uv_with_jitter(ivec3 id, float n, float f, float jitter)
{
	// Exponential View-Z
	float view_z = n * pow(f / n, (float(id.z) + 0.5f + jitter) / float(VOXEL_GRID_SIZE_Z));

	return vec3((float(id.x) + 0.5f) / float(VOXEL_GRID_SIZE_X),
		(float(id.y) + 0.5f) / float(VOXEL_GRID_SIZE_Y),
		view_z - n);
}


// Henyey-Greenstein
float phase_function(vec3 Wo, vec3 Wi, float g)
{
	float cos_theta = dot(Wo, Wi);
	float denom = 1.0f + g * g - 2.0f * g * cos_theta;
	return (1.0f / (4.0f * M_PI)) * (1.0f - g * g) / max(pow(denom, 1.5f), EPSILON);
}

vec3 FrustumRay(vec2 uv, vec4 frustumRays[4])
{
	vec3 ray0 = mix(frustumRays[1].xyz, frustumRays[0].xyz, uv.x);
	vec3 ray1 = mix(frustumRays[3].xyz, frustumRays[2].xyz, uv.x);
	return mix(ray1, ray0, uv.y);
}


// float cal_density(vec3 worldPos)
// {
// 	float density = uniforms.fogParams.x;  // constant density

// 	density += max(exp(uniforms.fogParams.y * (-worldPos.y + uniforms.fogParams.z)) * uniforms.fogParams.w, 0.0);  //height fog density

// 	for (int i = 0; i < u_FogVoulumes.FogVolumeCount; i++)
// 	{
// 		vec4 posLocal = u_FogVoulumes.FogVolumes[i].worldToLocal * vec4(worldPos, 1.0f);
// 		if (all(bvec3(abs(posLocal.x) < 0.5, abs(posLocal.y) < 0.5, abs(posLocal.z) < 0.5)))
// 			density += u_FogVoulumes.FogVolumes[i].density;
// 	}
// 	return density;
// }


layout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y, local_size_z = LOCAL_SIZE_Z) in;
void main()
{
	ivec3 outputCoord = ivec3(gl_WorkGroupID.xyz) * ivec3(LOCAL_SIZE_X, LOCAL_SIZE_Y, LOCAL_SIZE_Z) + ivec3(gl_LocalInvocationID.xyz);

	ivec2 noise_coord = (outputCoord.xy + ivec2(0, 1) * outputCoord.z * BLUE_NOISE_TEXTURE_SIZE) % BLUE_NOISE_TEXTURE_SIZE;

	float jitter = 0.0f;

#ifdef FROXEL_JITTER
	jitter = texelFetch(u_BlueNoise, noise_coord, 0).r;
	jitter = (jitter - 0.5f) * 0.999f;
#endif

	vec2 uv = vec2((float(outputCoord.x) + 0.5f) / float(VOXEL_GRID_SIZE_X), (float(outputCoord.y) + 0.5f) / float(VOXEL_GRID_SIZE_Y)) ;//+ vec2(jitter, -jitter) * globalData.;

	float f = uniforms.bias_near_far_pow.z;
	float n = uniforms.bias_near_far_pow.y;

	float viewZ = n * pow(f / n, (float(outputCoord.z) + 0.5f + jitter) / float(VOXEL_GRID_SIZE_Z));

	float k = viewZ / (f - n);
	vec3 rayDir = FrustumRay(uv, uniforms.frustumRays);
	vec3 worldPos = rayDir * k + globalData.cameraLocation.xyz;

	// Get the view direction from the current voxel.
	vec3 Wo = normalize(globalData.cameraLocation.xyz - worldPos.xyz);

	// Density and coefficient estimation.
	float density = uniforms.aniso_density_multipler_absorption.y;

	vec3 lighting = vec3(0.0);


	vec3 viewPos = (globalData.vMatrix * vec4(worldPos, 1.0f)).xyz;
	float distance = -viewPos.z;

	int cascadeIndex = 0;
	int lastCascadeIndex = 0;
	mat4 cascadeMatrix = uniforms.light.shadow.cascades[0].cascadeSpace;


	cascadeIndex = distance >= uniforms.light.shadow.cascades[0].distance ? 1 : cascadeIndex;
	cascadeIndex = distance >= uniforms.light.shadow.cascades[1].distance ? 2 : cascadeIndex;
	cascadeIndex = distance >= uniforms.light.shadow.cascades[2].distance ? 3 : cascadeIndex;
	cascadeIndex = distance >= uniforms.light.shadow.cascades[3].distance ? 4 : cascadeIndex;
	cascadeIndex = distance >= uniforms.light.shadow.cascades[4].distance ? 5 : cascadeIndex;

	cascadeIndex = min(uniforms.light.shadow.cascadeCount - 1, cascadeIndex);

	if (lastCascadeIndex != cascadeIndex) {
		cascadeMatrix = uniforms.light.shadow.cascades[0].cascadeSpace;
		cascadeMatrix = cascadeIndex > 0 ? uniforms.light.shadow.cascades[1].cascadeSpace : cascadeMatrix;
		cascadeMatrix = cascadeIndex > 1 ? uniforms.light.shadow.cascades[2].cascadeSpace : cascadeMatrix;
		cascadeMatrix = cascadeIndex > 2 ? uniforms.light.shadow.cascades[3].cascadeSpace : cascadeMatrix;
		cascadeMatrix = cascadeIndex > 3 ? uniforms.light.shadow.cascades[4].cascadeSpace : cascadeMatrix;
		cascadeMatrix = cascadeIndex > 4 ? uniforms.light.shadow.cascades[5].cascadeSpace : cascadeMatrix;
	}

	lastCascadeIndex = cascadeIndex;

	vec4 cascadeSpace = cascadeMatrix * vec4(viewPos, 1.0);
	cascadeSpace.xyz /= cascadeSpace.w;

	cascadeSpace.xy = cascadeSpace.xy * 0.5 + 0.5;

#ifdef X_TEXTURE_SHADOW_LOD
	// This fixes issues that can occur at cascade borders
	float shadowValue = textureLod(cascadeMaps,
		vec4(cascadeSpace.xy, cascadeIndex, cascadeSpace.z), 0);
#else
	float shadowValue = texture(cascadeMaps,
		vec4(cascadeSpace.xy, cascadeIndex, cascadeSpace.z));
#endif

	lighting += shadowValue * uniforms.light.color.rgb * uniforms.light.intensity * phase_function(Wo, -normalize(uniforms.light.direction.xyz), uniforms.aniso_density_multipler_absorption.x);


	int clusteroffset = GetPointLightBufferOffset(viewPos);

	for(uint i = 0; i < pointLightUniforms.pointLightCount; i++)
	{
		int pointLightIndex = s_VisiblePointLightIndicesBuffer.Indices[clusteroffset + i];
		if (pointLightIndex == -1)
			break;

		PointLightInfo pointlight = pointLightUniforms.pointLights[pointLightIndex];
		vec3 viewPointLightPos = vec3(globalData.vMatrix * vec4(pointlight.location.xyz,1.0f));
		vec3 worldSpacePointLightDir = pointlight.location.xyz - worldPos;
		float lightDistance = length(worldSpacePointLightDir);
		worldSpacePointLightDir = normalize(worldSpacePointLightDir);

		float attenuation = clamp(1.0 - (lightDistance * lightDistance) / (pointlight.radius * pointlight.radius), 0.0, 1.0);
		vec3 Lradiance = pointlight.color.xyz * pointlight.intensity * attenuation;

		// PointShadow
		float pointShadowFactor = PCF_PointLight(pointLightsShadowArray, pointlight.location.xyz, pointlight.minRadius, pointlight.radius, worldPos, pointLightIndex);

		lighting +=  Lradiance * pointShadowFactor * phase_function(Wo, worldSpacePointLightDir , uniforms.aniso_density_multipler_absorption.x); 
	}



	// float den = cal_density(worldPos);
	float den = 2 * uniforms.fogParams.x;

	vec4 color_and_density = vec4(lighting * den * uniforms.aniso_density_multipler_absorption.z, den);


#ifdef FROXEL_TEMPERAL_ACCUMULATING
	float viewZ_without_Jitter = n * pow(f / n, (float(outputCoord.z) + 0.5f) / float(VOXEL_GRID_SIZE_Z));
	vec3 worldPos_without_Jitter = rayDir * (viewZ_without_Jitter / (f - n)) + globalData.cameraLocation.xyz;

	vec4 pNdc_History = globalData.pvMatrixLast * vec4(worldPos_without_Jitter, 1.0f);
	pNdc_History.xyz /= pNdc_History.w;

	vec3 uv_History;
	uv_History.xy = pNdc_History.xy * 0.5f + 0.5f;

	//LinearizeDepth
	float linearDepth_History = globalData.pMatrix[3][2] / (pNdc_History.z + globalData.pMatrix[2][2]);

	vec2 params = vec2(1.0 / log2(f / n), -(log2(n)) / log2(f / n));
	uv_History.z = max(log2(linearDepth_History) * params.x + params.y, 0.0f);

	if (all(greaterThan(uv_History, vec3(0.0f))) && all(lessThan(uv_History, vec3(1.0f))))
	{
			vec4  history = textureLod(u_History, uv_History, 0.0f);
			color_and_density = mix(history, color_and_density, 0.05f);
	}
#endif

	// Write out lighting.
	imageStore(o_VoxelGrid, outputCoord, vec4(color_and_density));
	//imageStore(o_VoxelGrid, outputCoord, vec4(vec3( uniforms.light.color.rgb * uniforms.light.intensity), 1.0f));

}


