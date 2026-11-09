#pragma once
#include "EngineModel.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <functional>
namespace engine{
    struct TransformComponent{
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };
    class EngineGameObject{
        public:
            using id_t = unsigned int;
            static EngineGameObject* CreateGameObject(){
                static id_t currentId = 0;
                return new EngineGameObject{currentId++};
            }
            EngineGameObject(const EngineGameObject &) = delete;
            EngineGameObject &operator=(const EngineGameObject &) = delete;
            EngineGameObject(EngineGameObject&&) = default;
            EngineGameObject &operator=(EngineGameObject&&) noexcept = default;
            bool operator==(const EngineGameObject& other) const {
                if(other.name == name) return true;
                return false;
            }
            id_t getId(){return id;}
            std::shared_ptr<EngineModel> model{};
            glm::vec3 color{};
            bool visible = true;
            TransformComponent transform{};
            std::string name;
            bool twoDementional = false;
            std::string renderType;
            bool usesPhysics = false;
            bool hasHitbox = false;
            float xAcceleration = 0.f;
            float yAcceleration = 0.f;
            float zAcceleration = 0.f;
            float xVelocity = 0.f;
            float yVelocity = 0.f;
            float zVelocity = 0.f;
            float weight;
            float dragCoefficient = 1.05;
            float terminalVelocity = 0.5;
            std::vector<std::vector<glm::vec3>> hitBoxs;
            void CreateSquareHitbox(int x, int y, int z, int height);
            void CreateSphearHitbox(int radius);
            std::function<void(EngineGameObject colider)>* OnColistion = nullptr;
        private:
            EngineGameObject(id_t objId) : id{objId}{}
            EngineGameObject(id_t objId, bool twoD) : id{objId}, twoDementional{twoD}{}
            id_t id;
    };
}