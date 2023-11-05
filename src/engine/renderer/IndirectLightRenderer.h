#ifndef X_INDIRECTLIGHTRENDERER_H
#define X_INDIRECTLIGHTRENDERER_H

#include "../System.h"
#include "Renderer.h"

namespace X {

    namespace Renderer {

        class IndirectLightRenderer : public Renderer {

        public:
            IndirectLightRenderer() = default;

            void Init(Graphics::GraphicsDevice* device);

            void Render(Viewport* viewport, RenderTarget* target, Camera* camera,
                Scene::Scene* scene, Graphics::CommandList* commandList);

        private:
            struct Uniforms {
                int aoEnabled;
                int aoDownsampled2x;
                int reflectionEnabled;
                float aoStrength;
                int specularProbeMipLevels;
            };

            PipelineConfig pipelineConfig;

            Buffer::UniformBuffer uniformBuffer;

        };

    }

}

#endif