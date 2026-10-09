#include <iostream>
#include "EngineGameObject.hpp"
#include "EngineMain.hpp"
#include "EngineCamera.hpp"
#include "EngineInputController.hpp"
namespace engine{
    void start(){
        Sceen sceen1("sceen1");
        EngineGameObject* forest = sceen1.CreateObject("../assets/scenes/Forest.obj", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,0.f});
        EngineMain::LoadGameObjects(sceen1);
        EngineCamera* camera = EngineCamera::CreateCamera(0, glm::vec3(-1.f, -2.f, 2.f), 
        glm::vec3{0.f, 0.f, 2.5f}, glm::vec3{-0.5f, 0.f, 0.f}, 20, 8, 30);
        EngineCamera::SetMainCamera(camera);
    }
    void update(){
        //std::cout<<InputController::GetEvent()<<std::endl;
    }
    void wPressed(){std::cout<<"w pressed"<<std::endl;}
    void sPressed(){std::cout<<"s pressed"<<std::endl;}
    void EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
        MakeMethodOnUpdate(update);
        InputController::SetDownInput(GLFW_KEY_W, wPressed);
        InputController::SetDownInput(GLFW_KEY_S, sPressed);
    }
}