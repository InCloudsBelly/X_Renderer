#include "LightCullingRenderer.h"
#include "helper/GeometryHelper.h"
#include "lighting/PointLight.h"

#define TILT_SIZE 64

namespace X {

    namespace Renderer {

        void LightCullingRenderer::Init(Graphics::GraphicsDevice* device) {

            this->device = device;
            pipelineConfig = PipelineConfig("LightCulling_Pipeline", "lightculling.csh");
            pointLightIndicesBuffer = Buffer::Buffer(Buffer::BufferUsageBits::StorageBufferBit, sizeof(int));
            lightCullingUniformBuffer = Buffer::UniformBuffer(sizeof(LightCullingUniformBuffer));

        }

        void LightCullingRenderer::Render(Viewport* viewport, RenderTarget* target, Camera* camera,
            Scene::Scene* scene, Graphics::CommandList* commandList) {

            Graphics::Profiler::BeginQuery("LightCulling");

            auto pipeline = PipelineManager::GetPipeline(pipelineConfig);
            commandList->BindPipeline(pipeline);

            auto rtData = target->GetData(FULL_RES);
            auto velocityTexture = rtData->velocityTexture;
            auto depthTexture = rtData->depthTexture;

          
            LightCullingUniformBuffer uniforms;
            uniforms.cameraFar = camera->farPlane;
            uniforms.cameraNear = camera->nearPlane;

            glm::ivec3 groupCount;
            
            groupCount.x = (target->GetWidth() + TILT_SIZE -1) / TILT_SIZE;
            groupCount.y = (target->GetHeight() + TILT_SIZE -1) / TILT_SIZE;
            groupCount.z = 32;

            uniforms.clusterNum = vec3(groupCount);

            pointLightIndicesBuffer.SetSize(groupCount.x * groupCount.y * groupCount.z * MAX_POINTLIGHT_COUNT);
            lightCullingUniformBuffer.SetData(&uniforms, 0);

            commandList->BindBuffer(pointLightIndicesBuffer.Get(), 3, 17);
            commandList->BindBuffer(lightCullingUniformBuffer.Get(), 3, 16);

            commandList->Dispatch(groupCount.x, groupCount.y, groupCount.z);

            commandList->BufferMemoryBarrier(pointLightIndicesBuffer.Get(), VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

            Graphics::Profiler::EndQuery();

        }

    }

}