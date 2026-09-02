#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <optional>
#include "EngineModel.hpp"
#include "EngineDevice.hpp"
#include "EngineGameObject.hpp"
namespace engine{
    struct Sceen{
        std::vector<EngineGameObject> gameObjects;
        std::string name;
        Sceen(std::string nameTemp);
        static EngineDevice* engineDevice;
        bool AddObject(EngineGameObject object);
        bool RemoveObject(std::optional<EngineGameObject> object, std::optional<std::string> name);
        std::optional<EngineGameObject> CreateObejct(std::string modelPath, std::string modelName, std::optional<glm::vec3> threeDementionalTranslation = std::nullopt, 
        std::optional<glm::vec3> threeDementionalScale = std::nullopt, std::optional<glm::vec2> twoDementionalTranslation = std::nullopt, 
        std::optional<glm::vec2> twoDementionalScale = std::nullopt);
        ~Sceen();
        Sceen(const Sceen &) = delete;
        Sceen &operator=(const Sceen &) = delete;
            
    };
}