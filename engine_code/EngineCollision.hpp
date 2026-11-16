#pragma once
#include "EngineGameObject.hpp"
namespace engine{
	class Collision{
		public:
			static bool CheckCollision(const EngineGameObject& object,const EngineGameObject& collider);
	};
}