#include <algorithm>
#include <filesystem>
#include <iostream>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <cstring>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "EngineModel.hpp"
#include "EngineUtils.hpp"
namespace std{
    template<>
    struct hash<engine::EngineModel::Vertex>{
        size_t operator()(engine::EngineModel::Vertex const &vertex) const {
            size_t seed = 0;
            engine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}
namespace engine{
    void EngineModel::setTextureDescriptors(EngineDescriptorSetLayout& layout, EngineDescriptorPool& pool){
        textureSetLayout = &layout;
        texturePool = &pool;
    }
    EngineDescriptorSetLayout* EngineModel::textureSetLayout = nullptr;
    EngineDescriptorPool* EngineModel::texturePool = nullptr;
    EngineModel::EngineModel(EngineDevice& device, const Builder& builder) : engineDevice{device}, primitives{builder.primitives}{
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
        auto createTexture = [&](uint32_t width, uint32_t height, const std::vector<std::uint8_t>& pixels){
            auto texture = std::make_unique<EngineTexture>(engineDevice, width, height, pixels);
            auto imageInfo = texture->descriptorInfo();
            VkDescriptorSet descriptorSet{};
            if(!texturePool->allocateDescriptor(textureSetLayout->getDescriptorSetLayout(), descriptorSet))
                throw std::runtime_error("Failed to allocate texture descriptor");
            EngineDescriptorWriter(*textureSetLayout, *texturePool)
                .writeImage(0, &imageInfo)
                .overwrite(descriptorSet);
            textureDescriptorSets.push_back(descriptorSet);
            textures.push_back(std::move(texture));
        };
        createTexture(1, 1, {255, 255, 255, 255});
        for(const auto& texture : builder.textures) createTexture(texture.width, texture.height, texture.pixels);
    }
    EngineModel::~EngineModel(){}
    std::shared_ptr<EngineModel> EngineModel::createModelFromFile(EngineDevice &device, const std::string &filepath){
        if(!textureSetLayout || !texturePool)
            throw std::runtime_error("Texture descriptors not initialized");
        Builder builder{};
        builder.loadModel(filepath);
        return std::make_shared<EngineModel>(device, builder);
    }
    void EngineModel::createVertexBuffers(const std::vector<Vertex> &vertices){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);
        EngineBuffer stagingBuffer{engineDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());
        vertexBuffer = std::make_unique<EngineBuffer>(engineDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        engineDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }
    void EngineModel::createIndexBuffers(const std::vector<uint32_t> &indices){
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if(!hasIndexBuffer) return;
        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);
        EngineBuffer stagingBuffer{engineDevice, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*) indices.data());
        indexBuffer = std::make_unique<EngineBuffer>(engineDevice, indexSize, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        engineDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

    }
    void EngineModel::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout){
        for(const auto& primitive : primitives){
            uint32_t textureIndex = primitive.textureIndex < 0 ? 0 : static_cast<uint32_t>(primitive.textureIndex + 1);
            bindTexture(commandBuffer, pipelineLayout, textureDescriptorSets[textureIndex]);
            if(hasIndexBuffer) vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
            else vkCmdDraw(commandBuffer, primitive.vertexCount, 1, primitive.firstVertex, 0);
        }
    }
    void EngineModel::bind(VkCommandBuffer commandBuffer){
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if(hasIndexBuffer) vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
    std::vector<VkVertexInputBindingDescription> EngineModel::Vertex::getBindingDescriptions(){
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> EngineModel::Vertex::getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({0,0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1,0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2,0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3,0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
        return attributeDescriptions;
    }
    void EngineModel::Builder::loadModel(const std::string &filepath){
        fastgltf::Parser parser;
        auto file = fastgltf::MappedGltfFile::FromPath(filepath);
        if(!file) throw std::runtime_error("Failed to open GLB");
        fastgltf::Expected<fastgltf::Asset> result = parser.loadGltf(file.get(),std::filesystem::path(filepath).parent_path(),fastgltf::Options::LoadExternalBuffers | fastgltf::Options::GenerateMeshIndices);        
        if(result.error() != fastgltf::Error::None) throw std::runtime_error(std::string(fastgltf::getErrorMessage(result.error())));
        auto& asset = result.get();
        vertices.clear();
        indices.clear();
        textures.clear();
        primitives.clear();
        for(auto& mesh : asset.meshes){
            for(auto& primitive : mesh.primitives){
                Primitive modelPrimitive{};
                modelPrimitive.firstIndex = static_cast<uint32_t>(indices.size());
                modelPrimitive.firstVertex = static_cast<uint32_t>(vertices.size());
                if(primitive.materialIndex.has_value()){
                    auto& material = asset.materials[*primitive.materialIndex];
                    if(material.pbrData.baseColorTexture.has_value()){
                        auto textureIndex = material.pbrData.baseColorTexture->textureIndex;
                        auto& gltfTexture = asset.textures[textureIndex];
                        if(gltfTexture.imageIndex.has_value()){
                            modelPrimitive.textureIndex = static_cast<int>(textures.size());
                            textures.push_back(DecodeTexture(asset, asset.images[*gltfTexture.imageIndex]));
                        }
                    }
                }
                auto positionIt = std::find_if(primitive.attributes.begin(),primitive.attributes.end(),[](const auto& attr){return attr.name == "POSITION";});
                if(positionIt == primitive.attributes.end()) continue;
                auto baseVertex = static_cast<uint32_t>(vertices.size());
                auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
                fastgltf::math::nvec4 color ={1.f, 1.f, 1.f};
                if(primitive.materialIndex.has_value()) color = asset.materials[*primitive.materialIndex].pbrData.baseColorFactor;
                for(std::size_t i = 0; i < positionAccessor.count; i++){
                    vertices.emplace_back();
                    vertices.back().color = {color.x(), color.y(), color.z()};
                }
                auto colorIt = primitive.findAttribute("COLOR_0");
                if(colorIt != primitive.attributes.end()){
                    auto& accessor = asset.accessors[colorIt->accessorIndex];
                    if(accessor.type == fastgltf::AccessorType::Vec3)
                        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, accessor, [&](auto c, std::size_t i){
                            vertices[baseVertex + i].color = {c.x() * color.x(), c.y() * color.y(), c.z() * color.z()};
                        });
                    else if(accessor.type == fastgltf::AccessorType::Vec4)
                        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, accessor, [&](auto c, std::size_t i){
                            vertices[baseVertex + i].color = {c.x() * color.x(), c.y() * color.y(), c.z() * color.z()};
                        });
                    
                }
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset,positionAccessor,[&](auto pos,std::size_t i){
                    vertices[baseVertex + i].position = {pos.x(),pos.y(),pos.z()};
                });
                auto normalIt = std::find_if(primitive.attributes.begin(),primitive.attributes.end(),[](const auto& attr){return attr.name == "NORMAL";});
                if(normalIt != primitive.attributes.end())
                    fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset,asset.accessors[normalIt->accessorIndex],[&](auto normal,std::size_t i){
                        vertices[baseVertex + i].normal = {
                            normal.x(),
                            normal.y(),
                            normal.z()
                        };
                    });
                auto uvIt = std::find_if(primitive.attributes.begin(),primitive.attributes.end(),[](const auto& attr){return attr.name == "TEXCOORD_0";});
                if(uvIt != primitive.attributes.end())
                    fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset,asset.accessors[uvIt->accessorIndex],[&](auto uv,std::size_t i){
                        vertices[baseVertex + i].uv = {
                            uv.x(),
                            uv.y()};
                    });
                
                if(primitive.indicesAccessor.has_value())
                    fastgltf::iterateAccessor<std::uint32_t>(asset,asset.accessors[*primitive.indicesAccessor],[&](uint32_t index){
                        indices.push_back(baseVertex + index);
                    });
                modelPrimitive.vertexCount = static_cast<uint32_t>(vertices.size()) - modelPrimitive.firstVertex;
                modelPrimitive.indexCount = static_cast<uint32_t>(indices.size()) - modelPrimitive.firstIndex;
                primitives.push_back(modelPrimitive);
            }
        }
    }
    EngineModel::DecodedTexture EngineModel::DecodeTexture(const fastgltf::Asset& asset, const fastgltf::Image& image) {
        const std::byte* bytes = nullptr;
        std::size_t size = 0;
        std::visit(fastgltf::visitor{
            [](const auto&){},[&](const fastgltf::sources::Array& src){
                bytes = src.bytes.data();
                size = src.bytes.size();
            },[&](const fastgltf::sources::Vector& src){
                bytes = src.bytes.data();
                size = src.bytes.size();
            },[&](const fastgltf::sources::BufferView& src){
                const auto& view = asset.bufferViews[src.bufferViewIndex];
                const auto& buffer = asset.buffers[view.bufferIndex];
                std::visit(fastgltf::visitor{
                    [](const auto&){},
                    [&](const fastgltf::sources::Array& data){
                        bytes = data.bytes.data() + view.byteOffset;
                        size = view.byteLength;
                    },[&](const fastgltf::sources::Vector& data){
                        bytes = data.bytes.data() + view.byteOffset;
                        size = view.byteLength;
                    }
                },buffer.data);
            }
        }, image.data);
        if(!bytes || size == 0) throw std::runtime_error("Unsupported or empty texture source");
        DecodedTexture texture{};
        int channels;
        auto* decoded = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(bytes),
            static_cast<int>(size),
            &texture.width, &texture.height, &channels, STBI_rgb_alpha
        );
        if(!decoded) throw std::runtime_error("Failed to decode texture");
        texture.pixels.assign(decoded, decoded + texture.width * texture.height * 4);
        stbi_image_free(decoded);
        return texture;
    }
    void EngineModel::bindTexture(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet){
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
             1, 1, &descriptorSet, 0, nullptr);
    }
}