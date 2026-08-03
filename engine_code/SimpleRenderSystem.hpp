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
            SimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~SimpleRenderSystem();
            SimpleRenderSystem(const SimpleRenderSystem &) = delete;
            SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;
            void renderGameObjects(std::vector<EngineGameObject> &gameObjects, FrameInfo& frameInfo);
        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);
            EngineDevice& etDevice;
            std::unique_ptr<EnginePipeline> etPipeline;
            VkPipelineLayout pipelineLayout;
    };
}