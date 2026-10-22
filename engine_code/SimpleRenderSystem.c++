#include "SimpleRenderSystem.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "SimpleRenderSystem.hpp"
#include "EnginePipeline.hpp"
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <glm/gtc/constants.hpp>
namespace engine{
    struct SimplePushConstantData{
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
        uint32_t firstVertex = 0;
        uint32_t firstIndex = 0;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
        uint32_t trianglesPerTask = 1;
        uint32_t firstBatch;
    };
    SimpleRenderSystem::SimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout) : engineDevice{device}{
        meshSetLayout = EngineDescriptorSetLayout::Builder(engineDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT)
            .build();
        createPipelineLayout(globalSetLayout, textureSetLayout);
        createPipeline(renderPass);
    }
    SimpleRenderSystem::~SimpleRenderSystem(){vkDestroyPipelineLayout(engineDevice.device(), pipelineLayout, nullptr);}
    void SimpleRenderSystem::setMeshDescriptorPool(EngineDescriptorPool& pool){EngineModel::setMeshDescriptors(*meshSetLayout, pool);}
    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout){
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, textureSetLayout, meshSetLayout->getDescriptorSetLayout()};
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
        engineMeshPipeline = std::make_unique<EnginePipeline>(
        engineDevice,
        "../shaders/Task.task.spv",
        "../shaders/Mesh.mesh.spv",
        "../shaders/MeshFrag.frag.spv",
        pipelineConfig
        );
    }
    void SimpleRenderSystem::renderGameObjects(std::vector<EngineGameObject*> gameObjects, FrameInfo& frameInfo){
        engineMeshPipeline->bind(frameInfo.commandBuffer);
        auto drawMeshTasks = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(engineDevice.device(), "vkCmdDrawMeshTasksEXT"));
        if(!drawMeshTasks) throw std::runtime_error("Failed to load vkCmdDrawMeshTasksEXT");
        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        for(auto* obj : gameObjects){
            if(!obj->visible || !obj->model) continue;
            VkDescriptorSet meshSet = obj->model->getMeshDescriptorSet();
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &meshSet, 0, nullptr);            SimplePushConstantData push{};
            push.modelMatrix = obj->transform.mat4();
            push.normalMatrix = obj->transform.normalMatrix();
            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(SimplePushConstantData), &push);
            for(const auto& primitive : obj->model->getPrimitives()){
                push.firstVertex = primitive.firstVertex;
                push.firstIndex = primitive.firstIndex;
                push.vertexCount = primitive.vertexCount;
                push.indexCount = primitive.indexCount;
                push.firstBatch = primitive.firstBatch;
                uint32_t textureIndex = primitive.textureIndex < 0 ? 0 : static_cast<uint32_t>(primitive.textureIndex + 1);
                VkDescriptorSet textureSet = obj->model->getTextureDescriptorSet(textureIndex);
                vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 1, 1, &textureSet, 0, nullptr);
                const auto& limits = engineDevice.getMeshShaderProperties();
                uint32_t trianglesPerTask = std::min({85u,limits.maxMeshOutputVertices / 3,limits.maxMeshOutputPrimitives});
                if (trianglesPerTask == 0) throw std::runtime_error("Mesh shader limits are insufficient");
                push.trianglesPerTask = trianglesPerTask;
                vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(SimplePushConstantData), &push);
                uint32_t triangleCount = primitive.indexCount > 0 ? primitive.indexCount / 3 : primitive.vertexCount / 3;
                uint32_t taskCount = (triangleCount + trianglesPerTask - 1) / trianglesPerTask;
                drawMeshTasks(frameInfo.commandBuffer, taskCount, 1, 1);            
            }
        }
        // engineTwoDPipeline->bind(frameInfo.commandBuffer);
        // vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        // for(auto& obj : gameObjects){
        //     if(!obj->visible) continue;
        //     if(!obj->twoDementional) engineThreeDPipeline->bind(frameInfo.commandBuffer);
        //     else engineTwoDPipeline->bind(frameInfo.commandBuffer);
        //     SimplePushConstantData push{};
        //     push.modelMatrix = obj->transform.mat4();
        //     push.normalMatrix = obj->transform.normalMatrix();
        //     vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT,
        //     0, sizeof(SimplePushConstantData), &push);
        //     obj->model->bind(frameInfo.commandBuffer);
        //     obj->model->draw(frameInfo.commandBuffer, pipelineLayout);
        // }
    }
}