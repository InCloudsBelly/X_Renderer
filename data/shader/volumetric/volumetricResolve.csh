#include <../globals.hsh>
#include <../common/convert.hsh>
#include <../common/utility.hsh>
#include <../common/flatten.hsh>
#include <../structures>

#include <fog.hsh>

layout (local_size_x = 8, local_size_y = 8) in;

layout(set = 3, binding = 0, rgba16f) uniform image2D resolveImage;

layout(set = 3, binding = 2) uniform sampler2D lowResDepthTexture;
layout(set = 3, binding = 4) uniform sampler2D depthTexture;



#ifdef CLOUDS
layout(set = 3, binding = 3) uniform sampler2D lowResVolumetricCloudsTexture;
#endif

layout(set = 3, binding = 5) uniform  UniformBuffer {
    Fog fog;
    int downsampled2x;
    int cloudsEnabled;
    int fogEnabled;
    float innerCloudRadius;
    float planetRadius;
    float cloudDistanceLimit;
} uniforms;

#ifdef RENDER_VOLUME
#ifdef RAYMARCHING_VOLUME
layout(set = 3, binding = 1) uniform sampler2D lowResVolumetricTexture;
#endif

#ifdef FROXEL_VOLUME
layout(std140, set = 3, binding = 6) uniform FroxelUniformBuffer{
	vec4 bias_near_far_pow;
	vec4 aniso_density_multipler_absorption;
	vec4 frustumRays[4];

	vec4 windDir_Speed;
	vec4 fogParams;
    bool enableJitter;
	bool enableTemperalAccumulating;
	Light light;

} froxelUniforms;

layout(set = 3, binding = 7) uniform sampler3D u_FroxelGrid;



// https://gist.github.com/Fewes/59d2c831672040452aa77da6eaab2234
vec4 sampleTricubic(sampler3D tex, vec3 coord)
{
	// Shift the coordinate from [0,1] to [-0.5, texture_size-0.5]
    vec3 texture_size = vec3(textureSize(tex, 0));
	vec3 coord_grid = coord * texture_size - 0.5;
	vec3 index = floor(coord_grid);
	vec3 fraction = coord_grid - index;
	vec3 one_frac = 1.0 - fraction;

	vec3 w0 = 1.0/6.0 * one_frac*one_frac*one_frac;
	vec3 w1 = 2.0/3.0 - 0.5 * fraction*fraction*(2.0-fraction);
	vec3 w2 = 2.0/3.0 - 0.5 * one_frac*one_frac*(2.0-one_frac);
	vec3 w3 = 1.0/6.0 * fraction*fraction*fraction;

	vec3 g0 = w0 + w1;
	vec3 g1 = w2 + w3;
	vec3 mult = 1.0 / texture_size;
	vec3 h0 = mult * ((w1 / g0) - 0.5 + index); //h0 = w1/g0 - 1, move from [-0.5, texture_size-0.5] to [0,1]
	vec3 h1 = mult * ((w3 / g1) + 1.5 + index); //h1 = w3/g1 + 1, move from [-0.5, texture_size-0.5] to [0,1]

	// Fetch the eight linear interpolations
	// Weighting and fetching is interleaved for performance and stability reasons
	vec4 tex000 = texture(tex, h0);
	vec4 tex100 = texture(tex, vec3(h1.x, h0.y, h0.z));
	tex000 = mix(tex100, tex000, g0.x); // Weigh along the x-direction

	vec4 tex010 = texture(tex, vec3(h0.x, h1.y, h0.z));
	vec4 tex110 = texture(tex, vec3(h1.x, h1.y, h0.z));
	tex010 = mix(tex110, tex010, g0.x); // Weigh along the x-direction
	tex000 = mix(tex010, tex000, g0.y); // Weigh along the y-direction

	vec4 tex001 = texture(tex, vec3(h0.x, h0.y, h1.z));
	vec4 tex101 = texture(tex, vec3(h1.x, h0.y, h1.z));
	tex001 = mix(tex101, tex001, g0.x); // Weigh along the x-direction

	vec4 tex011 = texture(tex, vec3(h0.x, h1.y, h1.z));
	vec4 tex111 = texture(tex, vec3(h1));
	tex011 = mix(tex111, tex011, g0.x); // Weigh along the x-direction
	tex001 = mix(tex011, tex001, g0.y); // Weigh along the y-direction

	return mix(tex001, tex000, g0.z); // Weigh along the z-direction
	//return mix(tex000, tex001, g0.z); // Weigh along the z-direction
}


vec3 add_inscattered_light(vec3 color, vec3 uv, float depth)
{
	vec4  scattered_light = sampleTricubic(u_FroxelGrid, uv) ;
	//vec4  scattered_light = textureLod(u_FroxelGrid, uv, 0.0f);
    
    // If hit the sky , only addd scattered Light
    float transmittance   = depth !=1 ? scattered_light.a : 1.0;
    return color * transmittance + scattered_light.rgb;
}
#endif
#endif

// (localSize / 2 + 2)^2
shared float depths[36];
shared vec3 volumetrics[36];
shared vec4 clouds[36];

const uint depthDataSize = (gl_WorkGroupSize.x / 2 + 2) * (gl_WorkGroupSize.y / 2 + 2);
const ivec2 unflattenedDepthDataSize = ivec2(gl_WorkGroupSize) / 2 + 2;

vec3 planetCenter = -vec3(0.0, uniforms.planetRadius, 0.0);

void LoadGroupSharedData() {

    ivec2 workGroupOffset = ivec2(gl_WorkGroupID) * ivec2(gl_WorkGroupSize) / 2 - ivec2(1);

    // We assume data size is smaller than gl_WorkGroupSize.x + gl_WorkGroupSize.y
    if (gl_LocalInvocationIndex < depthDataSize) {
        ivec2 offset = Unflatten2D(int(gl_LocalInvocationIndex), unflattenedDepthDataSize);
        offset += workGroupOffset;
        offset = clamp(offset, ivec2(0), textureSize(lowResDepthTexture, 0));
        depths[gl_LocalInvocationIndex] = texelFetch(lowResDepthTexture, offset, 0).r;
#ifdef RENDER_VOLUME
#ifdef RAYMARCHING_VOLUME
        volumetrics[gl_LocalInvocationIndex] = texelFetch(lowResVolumetricTexture, offset, 0).rgb;
#endif
#endif

#ifdef CLOUDS
        clouds[gl_LocalInvocationIndex] = texelFetch(lowResVolumetricCloudsTexture, offset, 0);
#endif
    }

    barrier();

}

const ivec2 offsets[9] = ivec2[9](
    ivec2(-1, -1),
    ivec2(0, -1),
    ivec2(1, -1),
    ivec2(-1, 0),
    ivec2(0, 0),
    ivec2(1, 0),
    ivec2(-1, 1),
    ivec2(0, 1),
    ivec2(1, 1)
);

int NearestDepth(float referenceDepth, float[9] depthVec) {

    int idx = 0;
    float nearest = distance(referenceDepth, depthVec[0]);
    for (int i = 1; i < 9; i++) {
        float dist = distance(referenceDepth, depthVec[i]);
        if (dist < nearest) {
            nearest = dist;
            idx = i;
        }
    }
    return idx;

}

void Upsample2x(float referenceDepth, vec2 texCoord, out vec4 volumetric, out vec4 volumetricClouds) {

    ivec2 pixel = ivec2(gl_LocalInvocationID) / 2 + ivec2(1);

    float invocationDepths[9];

    float minWeight = 1.0;

    for (uint i = 0; i < 9; i++) {
        int sharedMemoryOffset = Flatten2D(pixel + offsets[i], unflattenedDepthDataSize);

        float depth = depths[sharedMemoryOffset];

        float depthDiff = abs(referenceDepth - depth);
        float depthWeight = min(exp(-depthDiff * 32.0), 1.0);
        minWeight = min(minWeight, depthWeight);

        invocationDepths[i] = depth;
    }

    int idx = NearestDepth(referenceDepth, invocationDepths);
    int offset = Flatten2D(pixel + offsets[idx], unflattenedDepthDataSize);

#ifdef RAYMARCHING_VOLUME
    volumetric = vec4(volumetrics[offset], 1.0);
#endif

#ifdef CLOUDS
    vec4 bilinearCloudScattering = texture(lowResVolumetricCloudsTexture, texCoord);
    volumetricClouds = mix(clouds[offset], bilinearCloudScattering, minWeight);
#endif
}

vec2 IntersectSphere(vec3 origin, vec3 direction, vec3 pos, float radius) {

    vec3 L = pos - origin;
    float DT = dot(L, direction);
    float r2 = radius * radius;

    float ct2 = dot(L, L) - DT * DT;

    if (ct2 > r2)
    return vec2(-1.0);

    float AT = sqrt(r2 - ct2);
    float BT = AT;

    float AO = DT - AT;
    float BO = DT + BT;

    float minDist = min(AO, BO);
    float maxDist = max(AO, BO);

    return vec2(minDist, maxDist);
}

void main() {

    if (uniforms.downsampled2x > 0) LoadGroupSharedData();

    ivec2 pixel = ivec2(gl_GlobalInvocationID);
    if (pixel.x > imageSize(resolveImage).x ||
        pixel.y > imageSize(resolveImage).y)
        return;

    vec2 texCoord = (vec2(pixel) + 0.5) / vec2(imageSize(resolveImage));

    float depth = texelFetch(depthTexture, pixel, 0).r;

    vec4 volumetric;
    vec4 cloudScattering;
    if (uniforms.downsampled2x > 0) {
        Upsample2x(depth, texCoord, volumetric, cloudScattering);
    }
    else {

#ifdef RENDER_VOLUME
#ifdef RAYMARCHING_VOLUME
        volumetric = vec4(textureLod(lowResVolumetricTexture, texCoord, 0).rgb, 0.0);
#endif
#endif

#ifdef CLOUDS
        cloudScattering = texture(lowResVolumetricCloudsTexture, texCoord);
#endif
    }

    vec3 viewPosition = ConvertDepthToViewSpace(depth, texCoord);
    float viewLength = length(viewPosition);


    vec3 worldDirection = normalize(vec3(globalData.ivMatrix * vec4(viewPosition, 0.0)));

    vec2 intersectDists = IntersectSphere(globalData.cameraLocation.xyz, worldDirection,
        planetCenter, uniforms.innerCloudRadius);
    vec2 planetDists = IntersectSphere(globalData.cameraLocation.xyz, worldDirection,
        planetCenter, uniforms.planetRadius);

    // x => first intersection with sphere, y => second intersection with sphere
    float cloudFadeout = 1.0;
    float cloudDist = intersectDists.y;
    float planetDist = planetDists.x < 0.0 ? planetDists.y : planetDists.x;
    if (depth == 1.0) {
        float maxDist = max(cloudDist, planetDist);
        // If we don't hit at all we don't want any fog
        viewPosition *= intersectDists.y > 0.0 ? max(maxDist / viewLength, 1.0) : 0.0;

        cloudFadeout = intersectDists.x < 0.0 ? saturate((uniforms.cloudDistanceLimit
            - cloudDist) / (uniforms.cloudDistanceLimit)) : cloudFadeout;
    }

    vec3 worldPosition = vec3(globalData.ivMatrix * vec4(viewPosition, 1.0));

    vec4 resolve = imageLoad(resolveImage, pixel);

    float fogAmount = uniforms.fogEnabled > 0 ? saturate(1.0 - ComputeVolumetricFog(uniforms.fog, globalData.cameraLocation.xyz, worldPosition)) : 0.0;

#ifdef CLOUDS
    if (uniforms.cloudsEnabled > 0) {
        cloudScattering.rgb *= cloudFadeout;
        cloudScattering.a = mix(1.0, cloudScattering.a, cloudFadeout);

        float alpha = cloudScattering.a;
        fogAmount = intersectDists.x < 0.0 ? fogAmount : fogAmount * alpha;
        resolve = alpha * resolve + cloudScattering;
    }
#endif

#ifdef RENDER_VOLUME

#ifdef RAYMARCHING_VOLUME
    resolve = uniforms.fogEnabled > 0 ? mix(resolve, uniforms.fog.color, fogAmount) + volumetric : resolve + volumetric;
#endif

#ifdef FROXEL_VOLUME
        float linearDepth = - viewPosition.z;
        vec3 uv;
        uv.xy = texCoord.xy;

        float n = froxelUniforms.bias_near_far_pow.y;
        float f = froxelUniforms.bias_near_far_pow.z;
        vec2 params = vec2(1.0 / log2(f/n), -(log2(n))/log2(f/n));
        uv.z = max(log2(linearDepth)* params.x + params.y, 0.0f);
        resolve.xyz = add_inscattered_light(resolve.xyz, uv, depth);
        resolve = uniforms.fogEnabled > 0 ? mix(resolve, uniforms.fog.color, fogAmount)  : resolve ;
#endif

#endif


    imageStore(resolveImage, pixel, resolve);

}