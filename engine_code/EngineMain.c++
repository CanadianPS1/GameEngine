#include <thread>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <chrono>
#include <functional>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
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
    float EngineMain::defultLight = -1;
    bool EngineMain::fpsLimited = false;
    float EngineMain::fpsMax = 60;
    EngineMain* EngineMain::instance = nullptr;
    EngineDevice* Sceen::engineDevice = nullptr;
    std::vector<EngineGameObject>* EngineMain::*gameObjects = nullptr;
    EngineMain::EngineMain(){ 
        instance = GetSelf();
        globalPool = EngineDescriptorPool::Builder(engineDevice)
            .setMaxSets(1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 998)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2000)
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
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT)
            .build();
        std::vector<VkDescriptorSet> globalDescriptorSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++){
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            EngineDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }
        auto textureSetLayout = EngineDescriptorSetLayout::Builder(engineDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        EngineModel::setTextureDescriptors(*textureSetLayout, *globalPool);
        SimpleRenderSystem simpleRenderSystem{engineDevice, engineRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};
        simpleRenderSystem.setMeshDescriptorPool(*globalPool);
        KeyboardMovementController cameraController{};
        Sceen::engineDevice = &engineDevice;
        KeyboardMovementController engineController{};
        try{SetInitialMethods();}catch(std::string e){};
        auto currentTime = std::chrono::high_resolution_clock::now();
        if(startMethods.size() > 0) for(std::function<void()> method : startMethods) method();
        InputController::window = engineWindow.getGLFWwindow();
        int fps = 0;
        int framesLeftInSecond;
        float oldTime;
        while(!engineWindow.shouldClose()){
            glfwPollEvents();
            fps++;
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            static auto lastSecond = std::chrono::steady_clock::now();
            auto newFPSTime = std::chrono::steady_clock::now();
            if(std::chrono::duration_cast<std::chrono::seconds>(newFPSTime - lastSecond).count() >= 1){
                lastSecond = newFPSTime;
                std::cout<<fps<<std::endl;
                fps = 0;
                if(fpsLimited) framesLeftInSecond = fpsMax;
            }
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
                ubo.defultLight = defultLight;
                if(EngineCamera::mainCamera != nullptr) ubo.projectionView = EngineCamera::mainCamera->getProjection() * EngineCamera::mainCamera->getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                engineRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(gameObjects, frameInfo);
                if(fpsLimited){
                    framesLeftInSecond--;
                    int timeToWait =  (1.0f / framesLeftInSecond - frameTime) * 1000;
                    std::this_thread::sleep_for(std::chrono::milliseconds(timeToWait));
                }
                engineRenderer.endSwapChainRenderPass(commandBuffer);
                engineRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(engineDevice.device());
    }
    void EngineMain::SetLightLevel(float level){defultLight = level;}
    void EngineMain::LoadGameObjects(Sceen& sceen){instance->gameObjects = sceen.gameObjects;}
    void EngineMain::UnloadGameObjects(){instance->gameObjects.clear();}
    void EngineMain::MakeMethodOnStart(std::function<void()> method){startMethods.push_back(method);}
    void EngineMain::MakeMethodOnUpdate(std::function<void()> method){updateMethods.push_back(method);}
}