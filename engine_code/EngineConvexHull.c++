#include "EngineConvexHull.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>
namespace engine{
    constexpr float EPSILON = 0.00001f;
    struct Face{
        uint32_t a,b,c;
        glm::vec3 normal;
        float distance;
        std::vector<uint32_t> outside;
    };
    struct Edge{
        uint32_t a,b;
        uint32_t count = 1;
    };
    uint64_t EdgeKey(uint32_t a,uint32_t b){
        if(a > b) std::swap(a,b);
        return (static_cast<uint64_t>(a) << 32) | b;
    }
    Face MakeFace(uint32_t a,uint32_t b,uint32_t c,const std::vector<glm::vec3>& points,const glm::vec3& inside){
        Face face{};
        glm::vec3 normal = glm::cross(points[b] - points[a],points[c] - points[a]);
        float length = glm::length(normal);
        if(length < EPSILON) return face;
        normal /= length;
        if(glm::dot(normal,inside - points[a]) > 0.f){
            std::swap(b,c);
            normal = -normal;
        }
        face.a = a;
        face.b = b;
        face.c = c;
        face.normal = normal;
        face.distance = glm::dot(normal,points[a]);
        return face;
    }
    float Distance(const Face& face,const glm::vec3& point){return glm::dot(face.normal,point) - face.distance;}
    void ConvexHull::Generate(const std::vector<glm::vec3>& input){
        vertices.clear();
        faces.clear();
        std::vector<glm::vec3> points = input;
        std::sort(points.begin(),points.end(),[](const glm::vec3& a,const glm::vec3& b){
            if(a.x != b.x) return a.x < b.x;
            if(a.y != b.y) return a.y < b.y;
            return a.z < b.z;
        });
        points.erase(std::unique(points.begin(),points.end(),[](const glm::vec3& a,const glm::vec3& b){
            return glm::dot(a - b,a - b) < EPSILON * EPSILON;
        }),points.end());
        if(points.size() < 4) return;
        uint32_t p0 = 0;
        uint32_t p1 = 0;
        for(uint32_t i = 1; i < points.size(); i++){
            if(points[i].x < points[p0].x) p0 = i;
            if(points[i].x > points[p1].x) p1 = i;
        }
        glm::vec3 line = points[p1] - points[p0];
        float lineLength = glm::length(line);
        if(lineLength < EPSILON) return;
        uint32_t p2 = p0;
        float maxDistance = 0.f;
        for(uint32_t i = 0; i < points.size(); i++){
            float distance = glm::length(glm::cross(points[i] - points[p0],line)) / lineLength;
            if(distance > maxDistance){
                maxDistance = distance;
                p2 = i;
            }
        }
        if(maxDistance < EPSILON) return;
        glm::vec3 planeNormal = glm::normalize(glm::cross(points[p1] - points[p0],points[p2] - points[p0]));
        uint32_t p3 = p0;
        maxDistance = 0.f;
        for(uint32_t i = 0; i < points.size(); i++){
            float distance = std::abs(glm::dot(planeNormal,points[i] - points[p0]));
            if(distance > maxDistance){
                maxDistance = distance;
                p3 = i;
            }
        }
        if(maxDistance < EPSILON) return;
        glm::vec3 inside = (points[p0] + points[p1] + points[p2] + points[p3]) * 0.25f;
        std::vector<Face> hull;
        hull.push_back(MakeFace(p0,p1,p2,points,inside));
        hull.push_back(MakeFace(p0,p3,p1,points,inside));
        hull.push_back(MakeFace(p1,p3,p2,points,inside));
        hull.push_back(MakeFace(p2,p3,p0,points,inside));
        std::array<uint32_t,4> initial{p0,p1,p2,p3};
        for(uint32_t i = 0; i < points.size(); i++){
            if(std::find(initial.begin(),initial.end(),i) != initial.end()) continue;
            float bestDistance = EPSILON;
            int bestFace = -1;
            for(int j = 0; j < hull.size(); j++){
                float distance = Distance(hull[j],points[i]);
                if(distance > bestDistance){
                    bestDistance = distance;
                    bestFace = j;
                }
            }
            if(bestFace >= 0) hull[bestFace].outside.push_back(i);
        }
        while(true){
            int selectedFace = -1;
            uint32_t eye = 0;
            float farthest = EPSILON;
            for(int i = 0; i < hull.size(); i++)
                for(uint32_t point : hull[i].outside){
                    float distance = Distance(hull[i],points[point]);
                    if(distance > farthest){
                        farthest = distance;
                        selectedFace = i;
                        eye = point;
                    }
                }
            if(selectedFace < 0) break;
            std::vector<uint32_t> visible;
            for(uint32_t i = 0; i < hull.size(); i++) if(Distance(hull[i],points[eye]) > EPSILON) visible.push_back(i);
            std::unordered_map<uint64_t,Edge> edges;
            for(uint32_t index : visible){
                const Face& face = hull[index];
                std::array<std::pair<uint32_t,uint32_t>,3> faceEdges{{
                    {face.a,face.b},
                    {face.b,face.c},
                    {face.c,face.a}
                }};
                for(auto [a,b] : faceEdges){
                    uint64_t key = EdgeKey(a,b);
                    auto it = edges.find(key);
                    if(it == edges.end()) edges.emplace(key,Edge{a,b,1});
                    else it->second.count++;
                }
            }
            std::vector<uint32_t> reassigned;
            for(uint32_t index : visible)
                for(uint32_t point : hull[index].outside) if(point != eye) reassigned.push_back(point);
            std::sort(reassigned.begin(),reassigned.end());
            reassigned.erase(std::unique(reassigned.begin(),reassigned.end()),reassigned.end());
            std::vector<Face> remaining;
            for(uint32_t i = 0; i < hull.size(); i++)
                if(std::find(visible.begin(),visible.end(),i) == visible.end()) remaining.push_back(std::move(hull[i]));
            std::vector<uint32_t> newFaces;
            for(const auto& [key,edge] : edges){
                if(edge.count != 1) continue;
                Face face = MakeFace(edge.b,edge.a,eye,points,inside);
                if(glm::length(face.normal) < EPSILON) continue;
                newFaces.push_back(static_cast<uint32_t>(remaining.size()));
                remaining.push_back(std::move(face));
            }
            for(uint32_t point : reassigned){
                float bestDistance = EPSILON;
                int bestFace = -1;
                for(uint32_t index : newFaces){
                    float distance = Distance(remaining[index],points[point]);
                    if(distance > bestDistance){
                        bestDistance = distance;
                        bestFace = static_cast<int>(index);
                    }
                }
                if(bestFace >= 0) remaining[bestFace].outside.push_back(point);
            }
            hull = std::move(remaining);
        }
        std::vector<uint32_t> remap(points.size(),UINT32_MAX);
        for(const auto& face : hull){
            for(uint32_t index : {face.a,face.b,face.c}){
                if(remap[index] == UINT32_MAX){
                    remap[index] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(points[index]);
                }
            }
        }
        for(const auto& face : hull)
            faces.push_back({
                remap[face.a],
                remap[face.b],
                remap[face.c],
                face.normal
            });
    }
}