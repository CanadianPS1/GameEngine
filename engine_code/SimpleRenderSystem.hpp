#pragma once
#include "EnginePipeline.hpp"
#include "EngineDevice.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "EngineGameObject.hpp"
#include "EngineFrameInfo.hpp"
namespace engine{
    class SimpleRenderSystem{
        public:
            SimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
            ~SimpleRenderSystem();
            SimpleRenderSystem(const SimpleRenderSystem &) = delete;
            SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;
            void renderGameObjects(std::vector<EngineGameObject*> gameObjects, FrameInfo& frameInfo);
            VkDescriptorSetLayout getMeshSetLayout() const {return meshSetLayout->getDescriptorSetLayout();}
            void setMeshDescriptorPool(EngineDescriptorPool& pool);
        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
            void createPipeline(VkRenderPass renderPass);
            EngineDevice& engineDevice;
            std::unique_ptr<EnginePipeline> engineThreeDPipeline;
            std::unique_ptr<EnginePipeline> engineTwoDPipeline;
            std::unique_ptr<EnginePipeline> engineMeshPipeline;
            std::unique_ptr<EnginePipeline> engineTessPipeline;
            VkPipelineLayout pipelineLayout;
            VkDescriptorSetLayout textureSetLayout;
            std::unique_ptr<EngineDescriptorSetLayout> meshSetLayout;
    };
}