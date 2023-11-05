#ifndef X_IMPOSTORSHADOWRENDERER_H
#define X_IMPOSTORSHADOWRENDERER_H

#include "../System.h"
#include "Renderer.h"

namespace X {

    namespace Renderer {

        class ImpostorShadowRenderer : public Renderer {

        public:
            ImpostorShadowRenderer() = default;

            void Init(Graphics::GraphicsDevice* device);

            void Render(Ref<Graphics::FrameBuffer>& frameBuffer, RenderList* renderList,
                Graphics::CommandList* commandList, RenderList::Pass* renderPass,
                mat4 lightViewMatrix, mat4 lightProjectionMatrix, vec3 lightLocation);

        private:
            Buffer::VertexArray vertexArray;

            PipelineConfig GetPipelineConfig(Ref<Graphics::FrameBuffer>& frameBuffer,
                bool interpolation, bool pixelDepthOffset);

        };

    }

}

#endif