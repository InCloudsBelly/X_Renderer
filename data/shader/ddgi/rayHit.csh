// Based on DDGI: 
// Majercik, Zander, et al. "Dynamic diffuse global illumination with ray-traced irradiance fields."
// Journal of Computer Graphics Techniques Vol 8.2 (2019).
// Majercik, Zander, et al. "Scaling Probe-Based Real-Time Dynamic Global Illumination for Production."
// arXiv preprint arXiv:2009.10796 (2020).
#ifdef USE_SHADOW_MAP
#define SHADOW_FILTER_1x1
#endif

#include <../raytracer/lights.hsh>
#include <../raytracer/tracing.hsh>
#include <../raytracer/direct.hsh>

#include <../common/random.hsh>
#include <../common/utility.hsh>
#include <../common/flatten.hsh>
#include <../common/PI.hsh>

#include <../brdf/brdfEval.hsh>
#include <../brdf/importanceSample.hsh>
#include <../brdf/surface.hsh>

#include <../shadow.hsh>
#include <../lightculling.hsh>

#include <ddgi.hsh>

layout (local_size_x = 32) in;

layout(set = 3, binding = 0) uniform sampler2DArrayShadow cascadeMaps;

// Instead of write ray array use hits array
layout(std430, set = 3, binding = 1) buffer RayHits {
    PackedRayHit hits[];
};

layout(std140, set = 3, binding = 2) uniform UniformBuffer {
    float seed;
    Shadow shadow;
} Uniforms;

vec3 EvaluateHit(inout Ray ray);
vec3 EvaluateDirectLight(inout Surface surface);
bool CheckVisibility(Surface surface, float lightDistance);

void main() {
    
    if (IsRayInvocationValid()) {    
        Ray ray = ReadRay();
        
        vec3 radiance = EvaluateHit(ray);
        
        RayHit hit;
        hit.radiance = radiance;
        hit.direction = ray.direction;
        hit.hitDistance = ray.hitDistance;

        hits[ray.ID] = PackRayHit(hit);
    }

}

vec3 EvaluateHit(inout Ray ray) {

    vec3 radiance = vec3(0.0);
    
    // If we didn't find a triangle along the ray,
    // we add the contribution of the environment map
    if (ray.hitID == -1) {
        return SampleEnvironmentMap(ray.direction, 4).rgb;
    }
    
    // Unpack the compressed triangle and extract surface parameters
    Triangle tri = UnpackTriangle(triangles[ray.hitID]);
    bool backfaceHit;    
    Surface surface = GetSurfaceParameters(tri, ray, false, backfaceHit, 4);

    // Indicates backface hit
    if (backfaceHit) {
        ray.hitDistance = -ray.hitDistance;
        return vec3(0.0);
    }
    
#ifdef RT_ENHANCE_EMISSION
    radiance +=  10 * surface.material.emissiveColor;
#else
    radiance += surface.material.emissiveColor;
#endif

    // Evaluate direct light
    radiance += EvaluateDirectLight(surface);

    // Need to sample the volume later for infinite bounces:
    vec3 indirect = EvaluateIndirectDiffuseBRDF(surface) *
        GetLocalIrradiance(surface.P, surface.V, surface.N).rgb;
    radiance += IsInsideVolume(surface.P) ? indirect : vec3(0.0);
    return radiance;

}

vec3 EvaluateDirectLight(inout Surface surface) {

    vec3 ret = vec3(0.0);
    if (GetLightCount() == 0)
        return vec3(0.0);

    float curSeed = Uniforms.seed;
    float raySeed = float(gl_GlobalInvocationID.x);

    float lightPdf;
    Light light = GetLight(surface, raySeed, curSeed, lightPdf);

    float solidAngle, lightDistance;
    SampleLight(light, surface, raySeed, curSeed, solidAngle, lightDistance);
    
    // Evaluate the BRDF
    vec3 reflectance = EvaluateDiffuseBRDF(surface);
    reflectance *= surface.material.opacity;
    vec3 radiance = light.radiance * solidAngle;

    // Check for visibilty. This is important to get an
    // estimate of the solid angle of the light from point P
    // on the surface.
#ifdef USE_SHADOW_MAP
    radiance *= CalculateShadowWorldSpace(Uniforms.shadow, cascadeMaps, surface.P,
        surface.geometryNormal, saturate(dot(surface.L, surface.geometryNormal)));
#else
    radiance *= CheckVisibility(surface, lightDistance) ? 1.0 : 0.0;
#endif

    ret += reflectance * radiance * surface.NdotL / lightPdf;


   for(uint i = 0; i < pointLightUniforms.pointLightCount; i++)
    {
        PointLightInfo pointlight = pointLightUniforms.pointLights[i];
        
        vec3 worldPointLightPos = pointlight.location.xyz;

        if(length(worldPointLightPos - surface.P) > pointlight.radius)
            continue;
        
        surface.L = normalize(worldPointLightPos - surface.P);
        UpdateSurface(surface);
            
        vec3 pointDirectDiffuse = EvaluateDiffuseBRDF(surface);

        float lightDistance = length(worldPointLightPos - surface.P);
        float attenuation = clamp(1.0 - (lightDistance * lightDistance) / (pointlight.radius * pointlight.radius), 0.0, 1.0);
        vec3 Lradiance = pointlight.color.xyz * pointlight.intensity * attenuation;

        // PointShadow
        float pointShadowFactor = PCF_PointLight(pointLightsShadowArray, pointlight.location.xyz, pointlight.minRadius, pointlight.radius, surface.P, i);

        ret += pointDirectDiffuse * Lradiance *  surface.NdotL * pointShadowFactor; 
    }
    return ret;
}

bool CheckVisibility(Surface surface, float lightDistance) {

    if (surface.NdotL > 0.0) {
        Ray ray;
        ray.direction = surface.L;
        ray.origin = surface.P + surface.N * EPSILON;
        ray.inverseDirection = 1.0 / ray.direction;
        return HitAny(ray, 0.0, lightDistance - 2.0 * EPSILON) == false;
    }
    else {
        return false;
    }

}