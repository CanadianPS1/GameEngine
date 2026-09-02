#pragma once
#include "EngineModel.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
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
            static EngineGameObject createGameObject(){
                static id_t currentId = 0;
                return EngineGameObject{currentId++};
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
            TransformComponent transform{};
            std::string name;
        private:
            EngineGameObject(id_t objId) : id{objId}{}
            id_t id;
    };
}