#include <iostream>
#include "EngineGameObject.hpp"
#include "EngineMain.hpp"
#include "EngineCamera.hpp"
#include "EngineInputController.hpp"
namespace engine{
    void start(){
        Sceen sceen1("sceen1");
        EngineGameObject* forest = sceen1.CreateObject("twoD", "../assets/scenes/paintingOneInspo.png", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{0.f,3.14159f,3.14159f});
        EngineMain::LoadGameObjects(sceen1);
        EngineCamera* camera = EngineCamera::CreateCamera(true, glm::vec3(-1.f, -2.f, 2.f), 
        glm::vec3{0.f, 0.f, 0.f}, glm::vec3{-0.5f, 0.f, 0.f}, 20, 8, 30);
        EngineCamera::SetMainCamera(camera);
        camera->far = 50;
    }
    void update(){
        std::cout<<InputController::GetEvent()<<std::endl;
    }
    // void wPressed(){std::cout<<"w pressed"<<std::endl;}
    // void sPressed(){std::cout<<"s pressed"<<std::endl;}
    void EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
        //fpsLimited = true;
        fpsMax = 60;
        //SetLightLevel(6.5f);
        // MakeMethodOnUpdate(update);
        // InputController::SetDownInput(GLFW_KEY_W, wPressed);
        // InputController::SetDownInput(GLFW_KEY_S, sPressed);
    }
}