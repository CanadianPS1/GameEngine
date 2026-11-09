#pragma once
#include "EngineGameObject.hpp"
#include "EngineSceen.hpp"
#include <cassert>
namespace engine{
    class EnginePhysics{
        public:
            static float AIR_DENSITY;
            static float AIR_TEMP;
            static float SPEED_OF_SOUND;
            static float GRAVITY;
            static void DictateGravity(std::vector<EngineGameObject*>& sceen);
            static void DetectCollision(std::vector<EngineGameObject*>& sceen);
            static void ApplyVelocity(std::vector<EngineGameObject*>& sceen, int& fps);
            static void DictateVelocity(std::vector<EngineGameObject*>& sceen, int& fps);
    };
}