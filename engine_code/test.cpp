#include <iostream>
#include "EngineGameObject.hpp"
#include "EngineMain.hpp"
#include "EngineCamera.hpp"
#include "EngineInputController.hpp"
namespace engine{
    EngineGameObject* forest;
    void start(){
        Sceen sceen1("sceen1");
        forest = sceen1.CreateObject("mesh", "../assets/scenes/Forest.glb", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,3.14159f});
        
        EngineGameObject* forest2 = sceen1.CreateObject("mesh", "../assets/scenes/Forest.glb", "Forest2", 
            glm::vec3{10.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,3.14159f});
        forest->CreateSquareHitbox(0, 5, 5, 2);
        EngineMain::LoadGameObjects(sceen1);
        EngineCamera* camera = EngineCamera::CreateCamera(0, glm::vec3(-1.f, -2.f, 2.f), 
        glm::vec3{0.f, 0.f, 0.f}, glm::vec3{-0.5f, 0.f, 0.f}, 20, 8, 30);
        EngineCamera::SetMainCamera(camera);
        camera->far = 1000;
        camera->near = 0.1;
    }
    void update(){
        std::cout<<InputController::GetEvent()<<std::endl;
    }
    void gPressed(){forest->usesPhysics = true;}
    // void sPressed(){std::cout<<"s pressed"<<std::endl;}
    void EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
        fpsLimited = false;
        fpsMax = 60;
        //SetLightLevel(6.5f);
        // MakeMethodOnUpdate(update);
        InputController::SetDownInput(GLFW_KEY_G, gPressed);
        // InputController::SetDownInput(GLFW_KEY_S, sPressed);
    }
}