#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "EngineInputController.hpp"
namespace engine{
    GLFWwindow* InputController::window = nullptr;
    std::unordered_map<int, std::function<void()>> InputController::keyDownMethods;
    std::unordered_map<int, std::function<void()>> InputController::keyUpMethods;
    int InputController::GetEvent(){
        for(int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) if(glfwGetKey(window,key) == GLFW_PRESS) return key;
        for(int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; jid++){
            if(!glfwJoystickPresent(jid)) continue;
            int count;
            const unsigned char* buttons = glfwGetJoystickButtons(jid,&count);
            for(int i = 0; i < count; i++) if(buttons[i] == GLFW_PRESS) return i;
        }
        return -1;
    }
    void InputController::CallKeyMethods(){
        try{
            for(int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++){
                if(glfwGetKey(window,key) == GLFW_PRESS) if(keyDownMethods.find(key) != keyDownMethods.end()) keyDownMethods.at(key)();
                if(glfwGetKey(window,key) == GLFW_RELEASE) if(keyUpMethods.find(key) != keyUpMethods.end()) keyUpMethods.at(key)();
            }
            for(int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; jid++){
                if(!glfwJoystickPresent(jid)) continue;
                int count;
                const unsigned char* buttons = glfwGetJoystickButtons(jid,&count);
                for(int i = 0; i < count; i++){ 
                    if(buttons[i] == GLFW_PRESS) if(keyDownMethods.find(i) != keyDownMethods.end()) keyDownMethods.at(i)();
                    if(buttons[i] == GLFW_RELEASE) if(keyUpMethods.find(i) != keyUpMethods.end()) keyUpMethods.at(i)();
                }
            }
        }catch(std::string e){std::cout<<"Key not bound {"<<e<<"}"<<std::endl;}
    }
    void InputController::SetDownInput(int key, std::function<void()> method){keyDownMethods[key] = method;}
    void InputController::SetUpInput(int key, std::function<void()> method){keyUpMethods[key] = method;}
}