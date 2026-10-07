#include <iostream>
#include "EngineGameObject.hpp"
#include "EngineMain.hpp"
namespace engine{
    bool run = false;
    void start(){
        if(!run){
            run = true;
            Sceen sceen1("sceen1");
            EngineGameObject* forest = sceen1.CreateObject("../assets/scenes/Forest.obj", "Forest", glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f});
            EngineMain::LoadGameObjects(sceen1);
            forest->transform.translation = (glm::vec3{0.0f, 0.0f, 0.f});
            forest->transform.scale = (glm::vec3{2.0f, 2.0f, 2.f});
        }
    }
    void EngineMain::SetInitialMethods(){
        MakeMethodOnUpdate(start);
    }
}