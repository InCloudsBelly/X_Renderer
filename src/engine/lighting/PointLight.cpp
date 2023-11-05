#include "PointLight.h"

namespace X {

    namespace Lighting {

        Shadow* PointLight::s_pointLightsArrayShadow = nullptr;
        uint32_t PointLight::s_pointLightsShadowCount = 0;
        std::vector<ShadowComponent> PointLight::s_pointLightShadowComponents(MAX_POINTLIGHT_SHADOW_COUNT);

        PointLight::PointLight(int32_t mobility, float radius) : Light(X_POINT_LIGHT, mobility), radius(radius) {

        }

        PointLight::~PointLight() {

            if (shadow) delete shadow;

            if (castShadow && hasShadow) {
                s_pointLightsShadowCount--;
                hasShadow = false;
            }

            if (s_pointLightsShadowCount == 0) {
                delete s_pointLightsArrayShadow;
                s_pointLightShadowComponents.clear();
            }

        }

        void PointLight::AddShadow(float bias, int32_t resolution) {

            if (shadow)
                delete shadow;
            shadow = new Shadow(0.0f, bias, resolution, true);

            if (castShadow && !hasShadow)
            {
                if (s_pointLightsShadowCount == 0)
                    s_pointLightsArrayShadow = new Shadow(0.0f, bias, resolution, true, MAX_POINTLIGHT_SHADOW_COUNT);
                s_pointLightsShadowCount ++;
                hasShadow = true;
            }

            Update(nullptr);

        }

        void PointLight::RemoveShadow() {

            delete shadow;
            shadow = nullptr;

            if (castShadow && hasShadow) {
                s_pointLightsShadowCount--;
                hasShadow = false;
            }

        }

        void PointLight::AddVolumetric(RenderResolution resolution, int32_t sampleCount, float scattering, float scatteringFactor) {

            volumetric = new Volumetric(resolution, sampleCount);

        }

        void PointLight::RemoveVolumetric() {

            delete volumetric;
            volumetric = nullptr;

        }

        void PointLight::Update(Camera* camera) {

            if (castShadow && !hasShadow)
            {
                s_pointLightsShadowCount++;
                hasShadow = true;
            }

            if (!castShadow && hasShadow) {
                s_pointLightsShadowCount--;
                hasShadow = false;
            }

            if (castShadow && hasShadow) {
                s_pointLightsArrayShadow->Update();

                mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, radius);
                vec3 faces[] = { vec3(1.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f),
                                 vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f),
                                 vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f) };

                vec3 ups[] = { vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f),
                               vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f),
                               vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) };

                for (uint8_t i = 0; i < 6; i++) {
                    mat4 viewMatrix = glm::lookAt(location, location + faces[i], ups[i]);


                    ShadowComponent component;
                    component.projectionMatrix = projectionMatrix;
                    component.viewMatrix = viewMatrix;
                    component.frustumMatrix = projectionMatrix * viewMatrix;

                    s_pointLightShadowComponents.push_back(component);
                }
            }

          

            if (mobility == X_MOVABLE_LIGHT && shadow) {
                shadow->Update();

                mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, radius);
                vec3 faces[] = { vec3(1.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f),
                                 vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f),
                                 vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f) };

                vec3 ups[] = { vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f),
                               vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f),
                               vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) };

                for (uint8_t i = 0; i < 6; i++) {
                    auto viewMatrix = glm::lookAt(location, location + faces[i], ups[i]);
                    shadow->components[i].projectionMatrix = projectionMatrix;
                    shadow->components[i].viewMatrix = viewMatrix;
                    shadow->components[i].frustumMatrix = projectionMatrix * viewMatrix;
                }
            }

          


        }

    }

}