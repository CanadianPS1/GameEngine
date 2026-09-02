#include <vector>
#include <algorithm>
#include <utility>
#include "EngineSceen.hpp"
namespace engine{
    engine::Sceen::Sceen(std::string tempName){name = tempName;}
    engine::Sceen::~Sceen(){
        gameObjects.clear();
    }
    std::optional<EngineGameObject> Sceen::CreateObejct(std::string modelPath, std::string modelName, std::optional<glm::vec3> threeDementionalTranslation, 
    std::optional<glm::vec3> threeDementionalScale, std::optional<glm::vec2> twoDementionalTranslation, std::optional<glm::vec2> twoDementionalScale){
        for(int i = 0; i < gameObjects.size(); i++) if(gameObjects[i].name == modelName) std::nullopt;
        std::shared_ptr<EngineModel> engineModel = EngineModel::createModelFromFile(*Sceen::engineDevice, modelPath);
        EngineGameObject object = EngineGameObject::createGameObject();
        object.model = engineModel;
        object.name = modelName;
        if(threeDementionalTranslation) object.transform.translation = *threeDementionalTranslation;
        // else if(twoDementionalTranslation) object.transform.translation = *twoDementionalTranslation;
        // else std::nullopt;
        if(threeDementionalScale) object.transform.scale = *threeDementionalScale;
        // else if(twoDementionalScale) object.transform.scale = *twoDementionalScale;
        // else std::nullopt;
        gameObjects.push_back(std::move(object));
        return object;
    }
    bool Sceen::AddObject(EngineGameObject object){
        try{
            for(int i = 0; i < gameObjects.size(); i++) if(gameObjects[i].name == object.name) return false;
            gameObjects.push_back(std::move(object));
            return true;
        }catch(const char* msg){return false;}
    }
    bool Sceen::RemoveObject(std::optional<EngineGameObject> object, std::optional<std::string> name){
        try{
            if(object) for(int i = 0; i < gameObjects.size(); i++) if(gameObjects[i] == *object) gameObjects.erase(gameObjects.begin() + i);
            else if(name) for(int i = 0; i < gameObjects.size(); i++) if(gameObjects[i].name == *name) gameObjects.erase(gameObjects.begin() + i);
            else return false;
            return true;
        }catch(const char* msg){return false;}
    }

}