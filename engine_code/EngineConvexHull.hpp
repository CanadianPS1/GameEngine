#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
namespace engine{
	struct HullFace{
		uint32_t a,b,c;
		glm::vec3 normal;
	};
	class ConvexHull{
		public:
			std::vector<glm::vec3> vertices;
			std::vector<HullFace> faces;
			void Generate(const std::vector<glm::vec3>& points);
	};
}