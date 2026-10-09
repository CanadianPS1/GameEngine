#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "EngineGameObject.hpp"
namespace engine{
    class EngineCamera{
        public:
            void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
            void setPerspectiveProjection(float fovy, float aspect, float near, float far);
            void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
            const glm::mat4& getProjection() const {return projectionMatrix;}
            const glm::mat4& getView() const {return viewMatrix;}
            static EngineCamera* CreateCamera(bool twoDementional, glm::vec3 position, glm::vec3 target, glm::vec3 rotation,
                float fealdOfView, float nearClipingPlain, float farClipingPlain);
            static EngineCamera* mainCamera;
            static void SetMainCamera(EngineCamera* camera);
            bool twoDementional;
            float near;
            float far;
            float fov;
            EngineGameObject* viewerObject = EngineGameObject::createGameObject();
        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
            EngineCamera(bool twoD);
    };
}