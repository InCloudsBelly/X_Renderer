#ifndef X_VOLUMETRICRENDERER_H
#define X_VOLUMETRICRENDERER_H

#include "../System.h"
#include "../Filter.h"
#include "Renderer.h"

namespace X {

    namespace Renderer {

        class VolumetricRenderer : public Renderer {

        public:
            VolumetricRenderer() = default;

            void Init(Graphics::GraphicsDevice* device);

            void Render(Viewport* viewport, RenderTarget* target, Camera* camera,
                Scene::Scene* scene, Graphics::CommandList* commandList);

        private:
            struct alignas(16) Fog {
                float density;
                float heightFalloff;
                float height;
                float scatteringAnisotropy;
                vec4 color;
            };

            struct alignas(16) VolumetricUniforms {
                int sampleCount;
                float intensity;
                float seed;
                int fogEnabled;
                Fog fog;
                Light light;
                CloudShadow cloudShadow;
            };

            struct alignas(16) FroxelUniforms {
                glm::vec4 bias_near_far_pow;
                glm::vec4 aniso_density_multipler_absorption;
                glm::vec4 frustumRays[4];

                glm::vec4 windDir_Speed{ 0.0f };
                glm::vec4 fogParams{ 0.0f }; //constant , Height Fog Exponent, Height Fog Offset, Height Fog Amount
                Light light;
            };

            struct alignas(16) BlurConstants {

            };

            struct alignas(16) ResolveUniforms {
                Fog fog;
                int downsampled2x;
                int cloudsEnabled;
                int fogEnabled;
                float innerCloudRadius;
                float planetRadius;
                float cloudDistanceLimit;
            };

            Filter blurFilter;

            PipelineConfig volumetricPipelineConfig;

            PipelineConfig horizontalBlurPipelineConfig;
            PipelineConfig verticalBlurPipelineConfig;

            PipelineConfig resolvePipelineConfig;

            PipelineConfig froxelInjectionPipelineConfig;
            PipelineConfig froxelScatteriongPipelineConfig;
            PipelineConfig froxelResolvePipelineConfig;

            Texture::Texture2D BlueNoiseTextures[8];

            Buffer::UniformBuffer volumetricUniformBuffer;
            Buffer::UniformBuffer blurWeightsUniformBuffer;
            Buffer::UniformBuffer resolveUniformBuffer;

            Buffer::UniformBuffer froxelUniformBuffer;


            Ref<Graphics::Sampler> shadowSampler;

        };

    }

}

#endif