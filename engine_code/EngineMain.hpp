#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include "EngineDescriptors.hpp"
#include "EngineGameObject.hpp"
#include "EngineRenderer.hpp"
#include "EngineWindow.hpp"
#include "EngineDevice.hpp"
#include "EngineSceen.hpp"
namespace engine{
    struct GlobalUbo{
        glm::mat4 projectionView{1.f};
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f,-3.f,-1.f});
    };
    struct UiVertex{
        glm::vec2 pos;
        glm::vec2 uv;
    };
    enum class GameState{
        Login,
        InGame
    };
    class EngineMain{
        public:
            static std::vector<Sceen> Sceens;
            static constexpr int WIDTH = 1400;
            static constexpr int HEIGHT = 788;
            static const std::string NAME;
            EngineMain();
            ~EngineMain();
            EngineMain(const EngineMain &) = delete;
            EngineMain &operator=(const EngineMain &) = delete;
            void run();
        private:
            void LoadGameObjects(Sceen& sceen);
            void UnloadGameObjects();
            EngineWindow engineWindow{WIDTH, HEIGHT, "NAME"};
            EngineDevice engineDevice{engineWindow};
            EngineRenderer engineRenderer{engineWindow, engineDevice};
            std::unique_ptr<EngineDescriptorPool> globalPool{};
            std::vector<EngineGameObject> gameObjects;
    };
}