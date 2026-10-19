#include "SimpleRenderSystem.hpp"
#include "EngineModel.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <iostream>
#include "SimpleRenderSystem.hpp"
#include "EnginePipeline.hpp"
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <glm/gtc/constants.hpp>
namespace engine{
    struct SimplePushConstantData{
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };
    SimpleRenderSystem::SimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout) : engineDevice{device}{
        createPipelineLayout(globalSetLayout, textureSetLayout);
        createPipeline(renderPass);
    }
    SimpleRenderSystem::~SimpleRenderSystem(){vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);}
    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout){
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, textureSetLayout};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(engineDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout");
    }
    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass){
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
        PipelineConfigInfo pipelineConfig{};
        EnginePipeline::defultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        engineThreeDPipeline = std::make_unique<EnginePipeline>(
            engineDevice,
            "../shaders/ThreeDVert.vert.spv",
            "../shaders/Frag.frag.spv",
            pipelineConfig
        );
        engineTwoDPipeline = std::make_unique<EnginePipeline>(
            engineDevice,
            "../shaders/TwoDVert.vert.spv",
            "../shaders/Frag.frag.spv",
            pipelineConfig
        );
    }
    void SimpleRenderSystem::renderGameObjects(std::vector<EngineGameObject*> gameObjects, FrameInfo& frameInfo){
        engineTwoDPipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        for(auto& obj : gameObjects){
            if(!obj->twoDementional) engineThreeDPipeline->bind(frameInfo.commandBuffer);
            else engineTwoDPipeline->bind(frameInfo.commandBuffer);
            SimplePushConstantData push{};
            push.modelMatrix = obj->transform.mat4();
            push.normalMatrix = obj->transform.normalMatrix();
            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(SimplePushConstantData), &push);
            obj->model->bind(frameInfo.commandBuffer);
            obj->model->draw(frameInfo.commandBuffer, pipelineLayout);
        }
    }
}