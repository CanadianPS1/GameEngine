#include <iostream>
#include "EngineGameObject.hpp"
#include "EngineMain.hpp"
namespace engine{
    void start(){
        Sceen sceen1("sceen1");
        EngineGameObject& forest = sceen1.CreateObejct("../assets/scenes/Forest.obj", "Forest", glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f});
        EngineMain::LoadGameObjects(sceen1);
    }
    void EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
    }
}