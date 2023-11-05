#ifndef X_DIRECTLIGHTRENDERER_H
#define X_DIRECTLIGHTRENDERER_H

#include "Renderer.h"

namespace X {

    namespace Renderer {

        class DirectLightRenderer : public Renderer {

        public:
            DirectLightRenderer() = default;

            void Init(Graphics::GraphicsDevice* device);

            void Render(Viewport* viewport, RenderTarget* target, Camera* camera,
                Scene::Scene* scene, Graphics::CommandList* commandList);

        private:
            struct alignas(16) Uniforms {
                Light light;
            };


            PipelineConfig pipelineConfig;

            Buffer::UniformBuffer uniformBuffer;

            Buffer::UniformBuffer cloudShadowUniformBuffer;
            Ref<Graphics::Sampler> shadowSampler;

        };

    }

}

#endif
