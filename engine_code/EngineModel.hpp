#pragma once
#include "EngineDescriptors.hpp"
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <cstring>
#include <memory>
#include <string>
#include "EngineDevice.hpp"
#include "EngineBuffer.hpp"
#include "EngineTexture.hpp"
#include "EngineDescriptors.hpp"
namespace engine{
    class EngineModel{
        public:
            struct DecodedTexture {
                int width;
                int height;
                std::vector<std::uint8_t> pixels;
            };
            struct Vertex{
                glm::vec3 position{};
                glm::vec3 color{};
                glm::vec3 normal{};
                glm::vec2 uv{};
                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
                bool operator==(const Vertex &other) const {
                    return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
                }
            };
            struct Primitive{
                uint32_t firstIndex = 0;
                uint32_t indexCount = 0;
                uint32_t firstVertex = 0;
                uint32_t vertexCount = 0;
                int textureIndex = -1;
            };
            struct Builder{
                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};
                std::vector<DecodedTexture> textures{};
                std::vector<Primitive> primitives{};
                void loadModel(const std::string &filepath);
            };
            static DecodedTexture DecodeTexture(const fastgltf::Asset& asset, const fastgltf::Image& image);
            EngineModel(EngineDevice& device, const EngineModel::Builder &builder);
            ~EngineModel();
            EngineModel(const EngineModel &) = delete;
            EngineModel &operator=(const EngineModel &) = delete;
            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
            void bindTexture(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
            static std::shared_ptr<EngineModel> createModelFromFile(EngineDevice &device, const std::string &filepath);
            static void setTextureDescriptors(EngineDescriptorSetLayout& layout, EngineDescriptorPool& pool);
        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            void createIndexBuffers(const std::vector<uint32_t> &indices);
            EngineDevice& engineDevice;
            std::unique_ptr<EngineBuffer> vertexBuffer;
            uint32_t vertexCount;
            bool hasIndexBuffer = false;
            std::unique_ptr<EngineBuffer> indexBuffer;
            uint32_t indexCount;
            std::vector<std::unique_ptr<EngineTexture>> textures;
            static EngineDescriptorSetLayout* textureSetLayout;
            static EngineDescriptorPool* texturePool;
            std::vector<VkDescriptorSet> textureDescriptorSets;
            std::vector<Primitive> primitives;
            
    };
}