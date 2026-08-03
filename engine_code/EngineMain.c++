#include "EngineDescriptors.hpp"
#include "EngineFrameInfo.hpp"
#include "EngineGameObject.hpp"
#include "EngineSwapChain.hpp"
#include <glm/detail/qualifier.hpp>
#include "EngineMain.hpp"
#include "EngineBuffer.hpp"
#include "SimpleRenderSystem.hpp"
#include "KeyboardMovementController.hpp"
#include "EngineCamera.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>
#include <chrono>
namespace engine{
    EngineMain::EngineMain(){
        globalPool = EngineDescriptorPool::Builder(etDevice)
            .setMaxSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }
    EngineMain::~EngineMain(){}
    void EngineMain::run(){
         std::vector<std::unique_ptr<EngineBuffer>> uboBuffers(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < uboBuffers.size(); i++){
          uboBuffers[i] = std::make_unique<EngineBuffer>(etDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);  
            uboBuffers[i]->map();
        } 
        auto globalSetLayout = EngineDescriptorSetLayout::Builder(etDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();
        std::vector<VkDescriptorSet> globalDescriptorSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++){
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            EngineDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }
        SimpleRenderSystem simpleRenderSystem{etDevice, etRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        EngineCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
        auto viewerObject = EngineGameObject::createGameObject();
        viewerObject.transform.rotation = {-0.5f, 0.f, 0.f};
        KeyboardMovementController cameraController{};
        //std::shared_ptr<EngineModel> etModel = EngineModel::createModelFromFile(etDevice, "assets/engine/EngineIdle.obj");
        EngineGameObject& etRef = gameObjects.back();
        KeyboardMovementController etController{};
        GameState state = GameState::Login;
        auto currentTime = std::chrono::high_resolution_clock::now();
        while(!etWindow.shouldClose()){
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            float aspect = etRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(20.f), aspect, 1, 30);
            if(auto commandBuffer = etRenderer.beginFrame()){
                int frameIndex = etRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                etRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(gameObjects, frameInfo);
                etRenderer.endSwapChainRenderPass(commandBuffer);
                etRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(etDevice.device());
    }
    void EngineMain::loadGameObjects(){
        std::shared_ptr<EngineModel> etModel = EngineModel::createModelFromFile(etDevice, "../assets/scenes/Forest.obj");
        auto Forest = EngineGameObject::createGameObject();
        Forest.model = etModel;
        Forest.name = "Forest";
        Forest.transform.translation = {0.0f, 7.0f, 13.f};
        Forest.transform.scale = {0.5f, 0.5f, 0.5f};
        gameObjects.push_back((std::move(Forest)));
    }
}