#ifndef X_POINTLIGHT_H
#define X_POINTLIGHT_H

#include "../System.h"
#include "Light.h"

#define MAX_POINTLIGHT_COUNT 1024 
#define MAX_POINTLIGHT_SHADOW_COUNT 16

namespace X {

    namespace Lighting {

        class PointLight : public Light {

        public:
            PointLight(int32_t mobility = X_STATIONARY_LIGHT, float radius = 5.0f);

            ~PointLight();

            void AddShadow(float bias, int32_t resolution);

            void RemoveShadow() override;

            void AddVolumetric(RenderResolution resolution, int32_t sampleCount, float scattering, float scatteringFactor = 1.0f);

            void RemoveVolumetric() override;

            void Update(Camera* camera) override;

            vec3 location = vec3(0.0f, 3.0f, 0.0f);
            vec3 color = vec3(1.0f);
            bool castShadow = true;
            bool hasShadow = false;

            float attenuation = 1.0f;
            float radius = 10.0f;


            static Shadow* s_pointLightsArrayShadow;
            static uint32_t s_pointLightsShadowCount;
            static std::vector<ShadowComponent> s_pointLightShadowComponents;

        };


    }

}

#endif
