#include "VolumetricRenderer.h"
#include "../lighting/DirectionalLight.h"
#include "../common/RandomHelper.h"

namespace X {

    namespace Renderer {

        void VolumetricRenderer::Init(Graphics::GraphicsDevice *device) {

            this->device = device;

            const int32_t filterSize = 4;
            blurFilter.CalculateBoxFilter(filterSize);

            volumetricPipelineConfig = PipelineConfig("Volumetric_Pipeline", "volumetric/volumetric.csh");

            horizontalBlurPipelineConfig = PipelineConfig("Volumetric_BlurHori_Pipeline","bilateralBlur.csh",
                {"HORIZONTAL", "BLUR_RGB", "DEPTH_WEIGHT"});
            verticalBlurPipelineConfig = PipelineConfig("Volumetric_BlurVert_Pipeline","bilateralBlur.csh",
                {"VERTICAL", "BLUR_RGB", "DEPTH_WEIGHT"});

            resolvePipelineConfig = PipelineConfig("Volumetric_Resolve_Pipeline","volumetric/volumetricResolve.csh");


            froxelInjectionPipelineConfig = PipelineConfig("Froxel_Injection_Pipeline", "volumetric/froxelInjection.csh");
            froxelScatteriongPipelineConfig = PipelineConfig("Froxel_Scattering_Pipeline", "volumetric/froxelScattering.csh");

            volumetricUniformBuffer = Buffer::UniformBuffer(sizeof(VolumetricUniforms));
            resolveUniformBuffer = Buffer::UniformBuffer(sizeof(ResolveUniforms));
            blurWeightsUniformBuffer = Buffer::UniformBuffer(sizeof(float) * (size_t(filterSize) + 1));
            froxelUniformBuffer = Buffer::UniformBuffer(sizeof(FroxelUniforms));

            for (int i = 0; i < 8; ++i) {
                std::string noisePath = "BlueNoise_256_px/LDR_LLL1_" + std::to_string(i) + ".png";
                BlueNoiseTextures[i] = Texture::Texture2D(noisePath);
            }

            auto samplerDesc = Graphics::SamplerDesc {
                .filter = VK_FILTER_NEAREST,
                .mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .compareEnabled = true
            };
            shadowSampler = device->CreateSampler(samplerDesc);

        }

        void VolumetricRenderer::Render(Viewport* viewport, RenderTarget* target,
            Camera* camera, Scene::Scene* scene, Graphics::CommandList* commandList) {




            Graphics::Profiler::BeginQuery("Render volumetric");

            auto lowResDepthTexture = target->GetData(target->GetVolumetricResolution())->depthTexture;
            auto depthTexture = target->GetData(FULL_RES)->depthTexture;

            commandList->ImageMemoryBarrier(target->volumetricTexture.image,
                VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT);

            commandList->BindImage(target->volumetricTexture.image, 3, 0);
            commandList->BindImage(lowResDepthTexture->image, lowResDepthTexture->sampler, 3, 1);

            auto lights = scene->GetLights();
            if (scene->sky.sun) {
                lights.push_back(scene->sky.sun.get());
            }

            ivec2 res = ivec2(target->volumetricTexture.width, target->volumetricTexture.height);

            auto& fog = scene->fog;
            assert((fog->useFroxelVolume && !fog->useRayMarchingVolume) || (!fog->useFroxelVolume && fog->useRayMarchingVolume));

            if (fog->useFroxelVolume)
            {
                {
                    Graphics::Profiler::BeginQuery("Froxel_Injection");

                    std::vector<Graphics::BufferBarrier> bufferBarriers;
                    std::vector<Graphics::ImageBarrier> imageBarriers;

                    commandList->ImageMemoryBarrier(target->froxelInjectionGrid.image,
                        VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT);
                    commandList->ImageMemoryBarrier(target->histroyFroxelInjectionGrid.image,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);

                    auto light = scene->sky.sun.get();

                    glm::uvec3 froxelFog_RayInjectionWorkGroups{ 1 };

                    uint32_t LOCAL_SIZE_X = 16u;
                    uint32_t LOCAL_SIZE_Y = 2u;
                    uint32_t LOCAL_SIZE_Z = 16u;

                    froxelFog_RayInjectionWorkGroups.x = static_cast<uint32_t>(ceil(float(target->VOXEL_GRID_SIZE_X) / float(LOCAL_SIZE_X)));
                    froxelFog_RayInjectionWorkGroups.y = static_cast<uint32_t>(ceil(float(target->VOXEL_GRID_SIZE_Y) / float(LOCAL_SIZE_Y)));
                    froxelFog_RayInjectionWorkGroups.z = static_cast<uint32_t>(ceil(float(target->VOXEL_GRID_SIZE_Z) / float(LOCAL_SIZE_Z)));


                    auto volumetric = light->GetVolumetric();
                    auto shadow = light->GetShadow();

                    if (light->type != X_DIRECTIONAL_LIGHT || !volumetric || !shadow)  assert(false, "volumetric light assert");

                    auto directionalLight = (Lighting::DirectionalLight*)light;
                    vec3 direction = normalize(directionalLight->direction);


                    auto fog = scene->fog;

                    FroxelUniforms uniforms;
                    uniforms.bias_near_far_pow = glm::vec4(0.002f, camera->nearPlane, camera->farPlane, 1.0f);
                    uniforms.aniso_density_multipler_absorption = glm::vec4(fog->froxelFogAnisotropy, fog->froxelFogGlobalDensity, fog->froxelFogLightMul, 0.0f);

                    std::vector<vec3> corners = camera->frustum.GetCorners();
                    uniforms.frustumRays[0] = vec4(corners[4] - camera->location, 0.0f);
                    uniforms.frustumRays[1] = vec4(corners[5] - camera->location, 0.0f);
                    uniforms.frustumRays[2] = vec4(corners[6] - camera->location, 0.0f);
                    uniforms.frustumRays[3] = vec4(corners[7] - camera->location, 0.0f);


                    float heightFogExponent = 0.0f;
                    float heightFogOffset = 0.0f;
                    float heightFogAmount = 0.0f;

                    uniforms.fogParams = { fog->froxelFogGlobalDensity , fog->froxelHeightFogExponent, fog->froxelHeightFogOffset, fog->froxelHeightFogAmount };

                    uniforms.light.direction = vec4(direction, 0.0);
                    uniforms.light.color = vec4(light->color, 0.0);
                    uniforms.light.intensity = light->intensity;
                    uniforms.light.shadow.cascadeCount = shadow->componentCount;

                    commandList->BindImage(shadow->maps.image, shadowSampler, 3, 2);

                    auto& shadowUniform = uniforms.light.shadow;
                    for (int32_t i = 0; i < MAX_SHADOW_CASCADE_COUNT + 1; i++) {
                        auto& cascadeUniform = shadowUniform.cascades[i];
                        auto cascadeString = "light.shadow.cascades[" + std::to_string(i) + "]";
                        if (i < shadow->componentCount) {
                            auto cascade = &shadow->components[i];
                            cascadeUniform.distance = cascade->farDistance;
                            cascadeUniform.cascadeSpace = cascade->projectionMatrix *
                                cascade->viewMatrix * camera->invViewMatrix;
                        }
                        else {
                            cascadeUniform.distance = camera->farPlane;
                        }
                    }


                    froxelUniformBuffer.SetData(&uniforms, 0);
                    commandList->BindBuffer(froxelUniformBuffer.Get(), 3, 4);
                    commandList->BindImage(target->froxelInjectionGrid.image, 3, 0);

                    static int FroxelIndex = 0;

                    if (++FroxelIndex == 8)
                        FroxelIndex = 0;
                    commandList->BindImage(BlueNoiseTextures[FroxelIndex].image, BlueNoiseTextures[FroxelIndex].sampler, 3, 5); //random noise index;
                    commandList->BindImage(target->histroyFroxelInjectionGrid.image, target->histroyFroxelInjectionGrid.sampler, 3, 6); //random noise index;


                    froxelInjectionPipelineConfig.ManageMacro("FROXEL_JITTER", fog->froxelEnableJitter);
                    froxelInjectionPipelineConfig.ManageMacro("FROXEL_TEMPERAL_ACCUMULATING", fog->froxelEnableTemperalAccumulating);

                    auto injectionPipeline = PipelineManager::GetPipeline(froxelInjectionPipelineConfig);


                    commandList->BindPipeline(injectionPipeline);

                    commandList->Dispatch(froxelFog_RayInjectionWorkGroups.x, froxelFog_RayInjectionWorkGroups.y, froxelFog_RayInjectionWorkGroups.z);


                    Graphics::Profiler::EndQuery();


                    imageBarriers = {
                        {target->froxelInjectionGrid.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT},
                        {target->histroyFroxelInjectionGrid.image ,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT },
                    };
                    commandList->PipelineBarrier(imageBarriers, bufferBarriers,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
                    commandList->CopyImage(target->froxelInjectionGrid.image, target->histroyFroxelInjectionGrid.image);

                    imageBarriers = {
                       {target->froxelInjectionGrid.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
                       {target->histroyFroxelInjectionGrid.image ,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
                    };
                    commandList->PipelineBarrier(imageBarriers, bufferBarriers,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);


                    {
                        Graphics::Profiler::BeginQuery("Froxel_Scattering");

                        imageBarriers = {
                           {target->froxelInjectionGrid.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
                           {target->froxelScatteringGrid.image, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT},
                        };
                        commandList->PipelineBarrier(imageBarriers, bufferBarriers);

                        commandList->BindImage(target->froxelInjectionGrid.image, target->froxelInjectionGrid.sampler, 3, 2);
                        commandList->BindImage(target->froxelScatteringGrid.image, 3, 0);
                        auto scatterPipeline = PipelineManager::GetPipeline(froxelScatteriongPipelineConfig);

                        commandList->BindPipeline(scatterPipeline);

                        uint32_t LOCAL_SIZE_X = 8u;
                        uint32_t LOCAL_SIZE_Y = 8u;
                        uint32_t LOCAL_SIZE_Z = 1u;


                        uint32_t groupX = static_cast<uint32_t>(ceil(float(target->VOXEL_GRID_SIZE_X) / float(LOCAL_SIZE_X)));
                        uint32_t groupY = static_cast<uint32_t>(ceil(float(target->VOXEL_GRID_SIZE_Y) / float(LOCAL_SIZE_Y)));

                        commandList->Dispatch(groupX, groupX, 1);

                        imageBarriers = {
                         {target->froxelScatteringGrid.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
                         {target->lightingTexture.image, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT},

                        };
                        commandList->PipelineBarrier(imageBarriers, bufferBarriers);

                        Graphics::Profiler::EndQuery();

                    }
                }
            }
            else if (fog->useRayMarchingVolume)
            {
                {
                    Graphics::Profiler::BeginQuery("Ray marching");

                    for (auto& light : lights) {
                        const int32_t groupSize = 8;

                        res = ivec2(target->GetWidth(), target->GetHeight());

                        ivec2 groupCount = res / groupSize;
                        groupCount.x += ((res.x % groupSize == 0) ? 0 : 1);
                        groupCount.y += ((res.y % groupSize == 0) ? 0 : 1);

                        auto volumetric = light->GetVolumetric();
                        auto shadow = light->GetShadow();

                        if (light->type != X_DIRECTIONAL_LIGHT || !volumetric || !shadow) continue;

                        auto directionalLight = (Lighting::DirectionalLight*)light;
                        vec3 direction = normalize(vec3(camera->viewMatrix * vec4(directionalLight->direction, 0.0f)));

                        VolumetricUniforms uniforms;
                        uniforms.sampleCount = volumetric->sampleCount;
                        uniforms.intensity = volumetric->intensity * light->intensity;
                        uniforms.seed = Common::Random::SampleFastUniformFloat();

                        uniforms.light.direction = vec4(direction, 0.0);
                        uniforms.light.color = vec4(light->color, 0.0);
                        uniforms.light.shadow.cascadeCount = shadow->componentCount;

                        commandList->BindImage(shadow->maps.image, shadowSampler, 3, 2);

                        auto& shadowUniform = uniforms.light.shadow;
                        for (int32_t i = 0; i < MAX_SHADOW_CASCADE_COUNT + 1; i++) {
                            auto& cascadeUniform = shadowUniform.cascades[i];
                            auto cascadeString = "light.shadow.cascades[" + std::to_string(i) + "]";
                            if (i < shadow->componentCount) {
                                auto cascade = &shadow->components[i];
                                cascadeUniform.distance = cascade->farDistance;
                                cascadeUniform.cascadeSpace = cascade->projectionMatrix *
                                    cascade->viewMatrix * camera->invViewMatrix;
                            }
                            else {
                                cascadeUniform.distance = camera->farPlane;
                            }
                        }

                        auto fog = scene->fog;
                        bool fogEnabled = fog && fog->enable;

                        uniforms.fogEnabled = fogEnabled ? 1 : 0;

                        if (fogEnabled) {
                            auto& fogUniform = uniforms.fog;
                            fogUniform.color = vec4(fog->color, 1.0f);
                            fogUniform.density = fog->density;
                            fogUniform.heightFalloff = fog->heightFalloff;
                            fogUniform.height = fog->height;
                            fogUniform.scatteringAnisotropy = glm::clamp(fog->scatteringAnisotropy, -0.999f, 0.999f);
                        }

                        auto clouds = scene->sky.clouds;
                        bool cloudShadowsEnabled = clouds && clouds->enable && clouds->castShadow;

                        if (cloudShadowsEnabled) {
                            auto& cloudShadowUniform = uniforms.cloudShadow;

                            clouds->shadowTexture.Bind(commandList, 3, 3);

                            clouds->GetShadowMatrices(camera, directionalLight->direction,
                                cloudShadowUniform.vMatrix, cloudShadowUniform.pMatrix);

                            cloudShadowUniform.vMatrix = cloudShadowUniform.vMatrix * camera->invViewMatrix;

                            cloudShadowUniform.ivMatrix = glm::inverse(cloudShadowUniform.vMatrix);
                            cloudShadowUniform.ipMatrix = glm::inverse(cloudShadowUniform.pMatrix);
                        }

                        volumetricUniformBuffer.SetData(&uniforms, 0);
                        commandList->BindBuffer(volumetricUniformBuffer.Get(), 3, 4);

                        volumetricPipelineConfig.ManageMacro("CLOUD_SHADOWS", cloudShadowsEnabled);
                        auto volumetricPipeline = PipelineManager::GetPipeline(volumetricPipelineConfig);

                        commandList->BindPipeline(volumetricPipeline);

                        commandList->Dispatch(groupCount.x, groupCount.y, 1);
                    }

                    Graphics::Profiler::EndQuery();

                    std::vector<Graphics::BufferBarrier> bufferBarriers;
                    std::vector<Graphics::ImageBarrier> imageBarriers;

                    {
                        Graphics::Profiler::BeginQuery("Bilateral blur");

                        const int32_t groupSize = 256;

                        std::vector<float> kernelWeights;
                        std::vector<float> kernelOffsets;

                        blurFilter.GetLinearized(&kernelWeights, &kernelOffsets, false);

                        auto mean = (kernelWeights.size() - 1) / 2;
                        kernelWeights = std::vector<float>(kernelWeights.begin() + mean, kernelWeights.end());
                        kernelOffsets = std::vector<float>(kernelOffsets.begin() + mean, kernelOffsets.end());

                        auto kernelSize = int32_t(kernelWeights.size() - 1);

                        auto horizontalBlurPipeline = PipelineManager::GetPipeline(horizontalBlurPipelineConfig);
                        auto verticalBlurPipeline = PipelineManager::GetPipeline(verticalBlurPipelineConfig);

                        blurWeightsUniformBuffer.SetData(kernelWeights.data(), 0);

                        commandList->BindImage(lowResDepthTexture->image, lowResDepthTexture->sampler, 3, 2);
                        commandList->BindBuffer(blurWeightsUniformBuffer.Get(), 3, 4);

                        ivec2 groupCount = ivec2(res.x / groupSize, res.y);
                        groupCount.x += ((res.x % groupSize == 0) ? 0 : 1);
                        imageBarriers = {
                            {target->volumetricTexture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
                            {target->swapVolumetricTexture.image, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT},
                        };
                        commandList->PipelineBarrier(imageBarriers, bufferBarriers);

                        commandList->BindPipeline(horizontalBlurPipeline);
                        commandList->PushConstants("constants", &kernelSize);

                        commandList->BindImage(target->swapVolumetricTexture.image, 3, 0);
                        commandList->BindImage(target->volumetricTexture.image, target->volumetricTexture.sampler, 3, 1);

                        commandList->Dispatch(groupCount.x, groupCount.y, 1);

                        groupCount = ivec2(res.x, res.y / groupSize);
                        groupCount.y += ((res.y % groupSize == 0) ? 0 : 1);

                        imageBarriers = {
                            {target->swapVolumetricTexture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
                            {target->volumetricTexture.image, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT},
                        };
                        commandList->PipelineBarrier(imageBarriers, bufferBarriers);

                        commandList->BindPipeline(verticalBlurPipeline);
                        commandList->PushConstants("constants", &kernelSize);

                        commandList->BindImage(target->volumetricTexture.image, 3, 0);
                        commandList->BindImage(target->swapVolumetricTexture.image, target->swapVolumetricTexture.sampler, 3, 1);

                        commandList->Dispatch(groupCount.x, groupCount.y, 1);

                        Graphics::Profiler::EndQuery();
                    }

                    imageBarriers = {
                        {target->volumetricTexture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
                        {target->lightingTexture.image, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT},
                    };
                    commandList->PipelineBarrier(imageBarriers, bufferBarriers);

                }
            }

            {
                Graphics::Profiler::BeginQuery("Resolve");

                const int32_t groupSize = 8;

                res = ivec2(target->GetWidth(), target->GetHeight());

                ivec2 groupCount = res / groupSize;
                groupCount.x += ((res.x % groupSize == 0) ? 0 : 1);
                groupCount.y += ((res.y % groupSize == 0) ? 0 : 1);

                auto clouds = scene->sky.clouds;
                auto cloudsEnabled = clouds && clouds->enable;

                auto fog = scene->fog;
                bool fogEnabled = fog && fog->enable;

                resolvePipelineConfig.ManageMacro("CLOUDS", cloudsEnabled);
                resolvePipelineConfig.ManageMacro("FOG", fogEnabled);

                resolvePipelineConfig.ManageMacro("RENDER_VOLUME", fog->renderVolume);
                resolvePipelineConfig.ManageMacro("FROXEL_VOLUME", fog->useFroxelVolume);
                resolvePipelineConfig.ManageMacro("RAYMARCHING_VOLUME", fog->useRayMarchingVolume);


                auto resolvePipeline = PipelineManager::GetPipeline(resolvePipelineConfig);
                commandList->BindPipeline(resolvePipeline);

                commandList->BindImage(target->lightingTexture.image, 3, 0);
                if(fog->useRayMarchingVolume)
                    commandList->BindImage(target->volumetricTexture.image, target->volumetricTexture.sampler, 3, 1);

                commandList->BindImage(lowResDepthTexture->image, lowResDepthTexture->sampler, 3, 2);
                commandList->BindImage(depthTexture->image, depthTexture->sampler, 3, 4);

                if (fog->useFroxelVolume) {
                    commandList->BindBuffer(froxelUniformBuffer.Get(), 3, 6);
                    commandList->BindImage(target->froxelScatteringGrid.image, target->froxelScatteringGrid.sampler, 3, 7);
                }


                ResolveUniforms uniforms;
                uniforms.cloudsEnabled = cloudsEnabled ? 1 : 0;
                uniforms.fogEnabled = fogEnabled ? 1 : 0;
                uniforms.downsampled2x = target->GetVolumetricResolution() == RenderResolution::HALF_RES ? 1 : 0;

                if (fogEnabled) {
                    auto& fogUniform = uniforms.fog;
                    fogUniform.color = vec4(fog->color, 1.0f);
                    fogUniform.density = fog->density;
                    fogUniform.heightFalloff = fog->heightFalloff;
                    fogUniform.height = fog->height;
                    fogUniform.scatteringAnisotropy = glm::clamp(fog->scatteringAnisotropy, -0.999f, 0.999f);
                }

                if (cloudsEnabled) {
                    uniforms.innerCloudRadius = scene->sky.planetRadius + clouds->minHeight;
                    uniforms.planetRadius = scene->sky.planetRadius;
                    uniforms.cloudDistanceLimit = clouds->distanceLimit;
                    commandList->BindImage(target->volumetricCloudsTexture.image, target->volumetricCloudsTexture.sampler, 3, 3);
                }

                resolveUniformBuffer.SetData(&uniforms, 0);
                commandList->BindBuffer(resolveUniformBuffer.Get(), 3, 5);

                commandList->Dispatch(groupCount.x, groupCount.y, 1);

                Graphics::Profiler::EndQuery();
            }


            commandList->ImageMemoryBarrier(target->lightingTexture.image,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);

            Graphics::Profiler::EndQuery();

        }

    }

}
