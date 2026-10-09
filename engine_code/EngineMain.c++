#include <type_traits>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <chrono>
#include <functional>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>
#include <glm/detail/qualifier.hpp>
#include "KeyboardMovementController.hpp"
#include "EngineInputController.hpp"
#include "SimpleRenderSystem.hpp"
#include "EngineDescriptors.hpp"
#include "EngineGameObject.hpp"
#include "EngineFrameInfo.hpp"
#include "EngineSwapChain.hpp"
#include "EngineCamera.hpp"
#include "EngineBuffer.hpp"
#include "EngineSceen.hpp"
#include "EngineMain.hpp"
namespace engine{
    static int num = 0;
    EngineMain* EngineMain::instance = nullptr;
    EngineDevice* Sceen::engineDevice = nullptr;
    std::vector<EngineGameObject>* EngineMain::*gameObjects = nullptr;
    EngineMain::EngineMain(){ 
        instance = GetSelf();
        globalPool = EngineDescriptorPool::Builder(engineDevice)
            .setMaxSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    }
    EngineMain::~EngineMain(){}
    EngineMain* EngineMain::GetSelf(){return this;}
    void EngineMain::run(){
        std::vector<std::unique_ptr<EngineBuffer>> uboBuffers(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < uboBuffers.size(); i++){
          uboBuffers[i] = std::make_unique<EngineBuffer>(engineDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);  
            uboBuffers[i]->map();
        }
        auto globalSetLayout = EngineDescriptorSetLayout::Builder(engineDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();
        std::vector<VkDescriptorSet> globalDescriptorSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++){
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            EngineDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }
        SimpleRenderSystem simpleRenderSystem{engineDevice, engineRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        KeyboardMovementController cameraController{};
        Sceen::engineDevice = &engineDevice;
        KeyboardMovementController engineController{};
        try{SetInitialMethods();}catch(std::string e){};
        auto currentTime = std::chrono::high_resolution_clock::now();
        if(startMethods.size() > 0) for(std::function<void()> method : startMethods) method();
        InputController::window = engineWindow.getGLFWwindow();
        while(!engineWindow.shouldClose()){
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            cameraController.moveInPlaneXZ(engineWindow.getGLFWwindow(), frameTime, *EngineCamera::mainCamera->viewerObject);
            if(updateMethods.size() > 0) for(std::function<void()> method : updateMethods) method();
            InputController::CallKeyMethods();
            if(EngineCamera::mainCamera != nullptr) EngineCamera::mainCamera->setViewYXZ(EngineCamera::mainCamera->viewerObject->transform.translation, EngineCamera::mainCamera->viewerObject->transform.rotation);
            float aspect = engineRenderer.getAspectRatio();
            if(EngineCamera::mainCamera != nullptr) EngineCamera::mainCamera->setPerspectiveProjection(glm::radians(EngineCamera::mainCamera->fov), 
                aspect, EngineCamera::mainCamera->near, EngineCamera::mainCamera->far);
            if(auto commandBuffer = engineRenderer.beginFrame()){
                int frameIndex = engineRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, *EngineCamera::mainCamera, globalDescriptorSets[frameIndex]};
                GlobalUbo ubo{};
                if(EngineCamera::mainCamera != nullptr) ubo.projectionView = EngineCamera::mainCamera->getProjection() * EngineCamera::mainCamera->getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                engineRenderer.beginSwapChainRenderPass(commandBuffer);
                std::vector<EngineGameObject> gameObjects2;
                simpleRenderSystem.renderGameObjects(gameObjects, frameInfo);
                engineRenderer.endSwapChainRenderPass(commandBuffer);
                engineRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(engineDevice.device());
    }
    void EngineMain::LoadGameObjects(Sceen& sceen){instance->gameObjects = sceen.gameObjects;}
    void EngineMain::UnloadGameObjects(){instance->gameObjects.clear();}
    void EngineMain::MakeMethodOnStart(std::function<void()> method){startMethods.push_back(method);}
    void EngineMain::MakeMethodOnUpdate(std::function<void()> method){updateMethods.push_back(method);}
}