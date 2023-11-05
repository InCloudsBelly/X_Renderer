#ifndef X_LIGHTCULLINGRENDERER_H
#define X_LIGHTCULLINGRENDERER_H

#include "../System.h"
#include "Renderer.h"
#include "buffer/VertexArray.h"

#include <unordered_map>

namespace X {

    namespace Renderer {

        class LightCullingRenderer : public Renderer {

        public:
            LightCullingRenderer() = default;

            void Init(Graphics::GraphicsDevice* device);

            void Render(Viewport* viewport, RenderTarget* target, Camera* camera, Scene::Scene* scene,
                Graphics::CommandList* commandList);

        private:
            struct alignas(16) LightCullingUniformBuffer {
                float cameraFar;
                float cameraNear;
                vec2 padding1;
                vec3 clusterNum;
                float padding2;
            };

            PipelineConfig pipelineConfig;

            Buffer::Buffer pointLightIndicesBuffer;
            Buffer::UniformBuffer lightCullingUniformBuffer;



        };

    }

}

#endif