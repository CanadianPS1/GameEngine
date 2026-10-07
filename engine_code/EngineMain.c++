#include <algorithm>
#include <type_traits>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <chrono>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>
#include <glm/detail/qualifier.hpp>
#include "KeyboardMovementController.hpp"
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
    EngineMain* EngineMain::instance = nullptr;
    EngineDevice* Sceen::engineDevice = nullptr;
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
        EngineCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
        auto viewerObject = EngineGameObject::createGameObject();
        viewerObject.transform.rotation = {-0.5f, 0.f, 0.f};
        KeyboardMovementController cameraController{};
        Sceen::engineDevice = &engineDevice;
        //EngineGameObject& engineRef = gameObjects.back();
        KeyboardMovementController engineController{};
        //GameState state = GameState::Login;
        try{SetInitialMethods();}catch(std::string e){};
        auto currentTime = std::chrono::high_resolution_clock::now();
        if(startMethods.size() > 0) for(std::function<void()> method : startMethods) method();
        while(!engineWindow.shouldClose()){
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            if(updateMethods.size() > 0) for(std::function<void()> method : updateMethods) method();
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            float aspect = engineRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(20.f), aspect, 1, 30);
            if(auto commandBuffer = engineRenderer.beginFrame()){
                int frameIndex = engineRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                engineRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(gameObjects, frameInfo);
                engineRenderer.endSwapChainRenderPass(commandBuffer);
                engineRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(engineDevice.device());
    }
    void EngineMain::LoadGameObjects(Sceen& sceen){for(int i = 0; i < sceen.gameObjects.size(); i++) instance->gameObjects.push_back(std::move(sceen.gameObjects[i]));}
    void EngineMain::UnloadGameObjects(){instance->gameObjects.clear();}
    void EngineMain::MakeMethodOnStart(std::function<void()> method){startMethods.push_back(method);}
    void EngineMain::MakeMethodOnUpdate(std::function<void()> method){updateMethods.push_back(method);}
}