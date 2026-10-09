#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>
namespace engine{
    class InputController{
        static std::unordered_map<int, std::function<void()>> keyDownMethods;
        static std::unordered_map<int, std::function<void()>> keyUpMethods;
        public:
            static int GetEvent();
            static GLFWwindow* window;
            static void CallKeyMethods();
            static void SetDownInput(int key, std::function<void()> method);
            static void SetUpInput(int key, std::function<void()> method);
    };
}