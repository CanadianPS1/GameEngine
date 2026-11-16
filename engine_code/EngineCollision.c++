#include "EngineCollision.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace engine{
	namespace{
		constexpr float EPSILON = 0.000001f;
		bool AddAxis(std::vector<glm::vec3>& axes,const glm::vec3& axis){
			float length = glm::length(axis);
			if(length < EPSILON) return false;
			glm::vec3 normalized = axis / length;
			for(const auto& existing : axes)
				if(std::abs(glm::dot(existing,normalized)) > 0.99999f) return false;
			axes.push_back(normalized);
			return true;
		}
		void Project(const std::vector<glm::vec3>& vertices,const glm::vec3& axis,float& min,float& max){
			min = max = glm::dot(vertices[0],axis);
			for(size_t i = 1; i < vertices.size(); i++){
				float projection = glm::dot(vertices[i],axis);
				min = std::min(min,projection);
				max = std::max(max,projection);
			}
		}
	}
	bool Collision::CheckCollision(const EngineGameObject& object,const EngineGameObject& collider){
		if(!object.model || !collider.model) return false;
		glm::mat4 objectMatrix = object.transform.mat4();
		glm::mat4 colliderMatrix = collider.transform.mat4();
		for(const auto& objectPrimitive : object.model->primitives){
			const auto& objectHull = objectPrimitive.convexHull;
			if(objectHull.vertices.empty() || objectHull.faces.empty()) continue;
			std::vector<glm::vec3> objectVertices;
			std::vector<glm::vec3> objectEdges;
			std::vector<glm::vec3> axes;
			for(const auto& vertex : objectHull.vertices)
				objectVertices.push_back(glm::vec3(objectMatrix * glm::vec4(vertex,1.f)));
			for(const auto& face : objectHull.faces){
				const auto& v0 = objectVertices[face.a];
				const auto& v1 = objectVertices[face.b];
				const auto& v2 = objectVertices[face.c];
				AddAxis(axes,glm::cross(v1 - v0,v2 - v0));
				objectEdges.push_back(v1 - v0);
				objectEdges.push_back(v2 - v1);
				objectEdges.push_back(v0 - v2);
			}
			for(const auto& colliderPrimitive : collider.model->primitives){
				const auto& colliderHull = colliderPrimitive.convexHull;
				if(colliderHull.vertices.empty() || colliderHull.faces.empty()) continue;
				std::vector<glm::vec3> colliderVertices;
				std::vector<glm::vec3> colliderEdges;
				std::vector<glm::vec3> collisionAxes = axes;
				for(const auto& vertex : colliderHull.vertices)
					colliderVertices.push_back(glm::vec3(colliderMatrix * glm::vec4(vertex,1.f)));
				for(const auto& face : colliderHull.faces){
					const auto& v0 = colliderVertices[face.a];
					const auto& v1 = colliderVertices[face.b];
					const auto& v2 = colliderVertices[face.c];
					AddAxis(collisionAxes,glm::cross(v1 - v0,v2 - v0));
					colliderEdges.push_back(v1 - v0);
					colliderEdges.push_back(v2 - v1);
					colliderEdges.push_back(v0 - v2);
				}
				for(const auto& objectEdge : objectEdges)
					for(const auto& colliderEdge : colliderEdges)
						AddAxis(collisionAxes,glm::cross(objectEdge,colliderEdge));
				bool separated = false;
				for(const auto& axis : collisionAxes){
					float objectMin,objectMax,colliderMin,colliderMax;
					Project(objectVertices,axis,objectMin,objectMax);
					Project(colliderVertices,axis,colliderMin,colliderMax);
					if(objectMax < colliderMin - EPSILON || colliderMax < objectMin - EPSILON){
						separated = true;
						break;
					}
				}
				if(!separated) return true;
			}
		}
		return false;
	}
}