#include "EnginePhysics.hpp"
#include "EngineGameObject.hpp"
#include <iostream>
#include <algorithm>
namespace engine{
    float EnginePhysics::AIR_DENSITY = 1.2250;
    float EnginePhysics::GRAVITY = 9.8;
    void EnginePhysics::DictateGravity(std::vector<EngineGameObject*>& sceen){
        for(EngineGameObject* object : sceen){
            if(!object->usesPhysics) continue;
            object->yAcceleration = GRAVITY;
        }
    }
    void EnginePhysics::DictateVelocity(std::vector<EngineGameObject*>& sceen, int& fps){
        for(EngineGameObject* object : sceen){
            if(!object->usesPhysics || fps == 0) continue;
            object->xVelocity += (object->xAcceleration / fps) / 2;
            object->yVelocity += (object->yAcceleration / fps) / 2;
            object->zVelocity += (object->zAcceleration / fps) / 2;
            if(object->terminalVelocity != 0){
                float maxVelocity = (object->terminalVelocity / fps) / 2;
                object->xVelocity = std::clamp((object->xVelocity + object->xAcceleration / fps) / 2, -maxVelocity, maxVelocity);
                object->yVelocity = std::clamp((object->yVelocity + object->yAcceleration / fps) / 2, -maxVelocity, maxVelocity);
                object->zVelocity = std::clamp((object->zVelocity + object->zAcceleration / fps) / 2, -maxVelocity, maxVelocity);
            }
        }
    }
    void EnginePhysics::ApplyVelocity(std::vector<EngineGameObject*>& sceen, int &fps){
        for(EngineGameObject* object : sceen){
            if(!object->usesPhysics || fps == 0) continue;
            std::cout<<"velocity: "<<object->yVelocity<<std::endl;
            std::cout<<"location: "<<object->transform.translation.y<<std::endl;
            object->transform.translation = {
                object->transform.translation.x + object->xVelocity,
                object->transform.translation.y + object->yVelocity,
                object->transform.translation.z + object->zVelocity
            };
        }
    }
}