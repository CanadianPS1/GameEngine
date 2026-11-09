#include <algorithm>
#include <filesystem>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <stdexcept>
#include <iostream>
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
    EngineDescriptorSetLayout* EngineModel::meshSetLayout = nullptr;
    EngineDescriptorPool* EngineModel::meshPool = nullptr;
    EngineDescriptorSetLayout* EngineModel::textureSetLayout = nullptr;
    EngineDescriptorPool* EngineModel::texturePool = nullptr;
    static_assert(sizeof(EngineModel::Vertex) == 44);
    static_assert(offsetof(EngineModel::Vertex, position) == 0);
    static_assert(offsetof(EngineModel::Vertex, color) == 12);
    static_assert(offsetof(EngineModel::Vertex, normal) == 24);
    static_assert(offsetof(EngineModel::Vertex, uv) == 36);
    EngineModel::EngineModel(EngineDevice& device, const Builder& builder) : engineDevice{device}, primitives{builder.primitives}{
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
        createBatchBoundsBuffer();
        if(!meshSetLayout || !meshPool) throw std::runtime_error("Mesh descriptors not initialized");
        auto vertexInfo = vertexBuffer->descriptorInfo(vertexBuffer->getBufferSize());
        auto indexInfo = hasIndexBuffer ? indexBuffer->descriptorInfo(indexBuffer->getBufferSize()) : vertexBuffer->descriptorInfo(vertexBuffer->getBufferSize());
        auto boundsInfo = batchBoundsBuffer->descriptorInfo(batchBoundsBuffer->getBufferSize());
        EngineDescriptorWriter(*meshSetLayout, *meshPool)
            .writeBuffer(0, &vertexInfo)
            .writeBuffer(1, &indexInfo)
            .writeBuffer(2, &boundsInfo)
            .build(meshDescriptorSet);
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
        createBottomLevelAS();
    }
    EngineModel::~EngineModel(){
        if(blas != VK_NULL_HANDLE){
            auto destroyAS =reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr( engineDevice.device(), "vkDestroyAccelerationStructureKHR"));
            if(destroyAS) destroyAS(engineDevice.device(), blas, nullptr);
            blas = VK_NULL_HANDLE;
        }
    }
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
        EngineBuffer stagingBuffer{
            engineDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());
        vertexBuffer = std::make_unique<EngineBuffer>(
            engineDevice, vertexSize, vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        engineDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }
    void EngineModel::createIndexBuffers(const std::vector<uint32_t> &indices){
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if(!hasIndexBuffer) return;
        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);
        EngineBuffer stagingBuffer{
            engineDevice, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());
        indexBuffer = std::make_unique<EngineBuffer>(
            engineDevice, indexSize, indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
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
        auto extension = std::filesystem::path(filepath).extension().string();
        std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c){return std::tolower(c);});
        if(std::filesystem::path(filepath).extension() == ".png"){
            vertices.clear();
            indices.clear();
            textures.clear();
            primitives.clear();
            int width,height,channels;
            stbi_uc* pixels = stbi_load(filepath.c_str(),&width,&height,&channels,STBI_rgb_alpha);
            if(!pixels) throw std::runtime_error("Failed to load PNG");
            Primitive modelPrimitive{};
            modelPrimitive.firstVertex = 0;
            modelPrimitive.firstIndex = 0;
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    int pixelIndex = (y * width + x) * 4;
                    float alpha = pixels[pixelIndex + 3] / 255.f;
                    if(alpha == 0.f) continue;
                    glm::vec3 color{
                        pixels[pixelIndex] / 255.f,
                        pixels[pixelIndex + 1] / 255.f,
                        pixels[pixelIndex + 2] / 255.f
                    };
                    float x0 = static_cast<float>(x) / width - 0.5f;
                    float x1 = static_cast<float>(x + 1) / width - 0.5f;
                    float y0 = (static_cast<float>(height) * 0.5f - y) / width;
                    float y1 = (static_cast<float>(height) * 0.5f - y - 1.f) / width;
                    uint32_t baseVertex = static_cast<uint32_t>(vertices.size());
                    vertices.resize(vertices.size() + 4);
                    vertices[baseVertex].position = {x0,y0,0.f};
                    vertices[baseVertex + 1].position = {x1,y0,0.f};
                    vertices[baseVertex + 2].position = {x1,y1,0.f};
                    vertices[baseVertex + 3].position = {x0,y1,0.f};
                    for(uint32_t i = 0; i < 4; i++){
                        vertices[baseVertex + i].color = color;
                        vertices[baseVertex + i].normal = {0.f,0.f,1.f};
                    }
                    indices.insert(indices.end(),{
                        baseVertex,baseVertex + 1,baseVertex + 2,
                        baseVertex,baseVertex + 2,baseVertex + 3
                    });
                }
            }
            stbi_image_free(pixels);
            modelPrimitive.vertexCount = static_cast<uint32_t>(vertices.size());
            modelPrimitive.indexCount = static_cast<uint32_t>(indices.size());
            constexpr uint32_t trianglesPerBatch = 85;
            uint32_t triangleCount = modelPrimitive.indexCount / 3;
            for(uint32_t firstTriangle = 0; firstTriangle < triangleCount; firstTriangle += trianglesPerBatch){
                uint32_t count = std::min(trianglesPerBatch,triangleCount - firstTriangle);
                glm::vec3 minBounds{std::numeric_limits<float>::max()};
                glm::vec3 maxBounds{std::numeric_limits<float>::lowest()};
                for(uint32_t i = 0; i < count * 3; i++){
                    uint32_t vertexIndex = indices[firstTriangle * 3 + i];
                    const auto& position = vertices[vertexIndex].position;
                    minBounds = glm::min(minBounds,position);
                    maxBounds = glm::max(maxBounds,position);
                }
                glm::vec3 center = (minBounds + maxBounds) * 0.5f;
                float radius = glm::length(maxBounds - center);
                modelPrimitive.batchBounds.push_back({glm::vec4(center,radius)});
            }
            if(modelPrimitive.indexCount > 0) primitives.push_back(modelPrimitive);
            return;
        }
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
                constexpr uint32_t trianglesPerBatch = 85;
                uint32_t triangleCount = modelPrimitive.indexCount > 0 ? modelPrimitive.indexCount / 3 : modelPrimitive.vertexCount / 3;
                for(uint32_t firstTriangle = 0; firstTriangle < triangleCount; firstTriangle += trianglesPerBatch){
                    uint32_t count = std::min(trianglesPerBatch, triangleCount - firstTriangle);
                    glm::vec3 minBounds{std::numeric_limits<float>::max()};
                    glm::vec3 maxBounds{std::numeric_limits<float>::lowest()};
                    for(uint32_t i = 0; i < count * 3; i++){
                        uint32_t vertexIndex = modelPrimitive.indexCount > 0 ? indices[modelPrimitive.firstIndex + firstTriangle * 3 + i] : modelPrimitive.firstVertex + firstTriangle * 3 + i;
                        const auto& position = vertices[vertexIndex].position;
                        minBounds = glm::min(minBounds, position);
                        maxBounds = glm::max(maxBounds, position);
                    }
                    glm::vec3 center = (minBounds + maxBounds) * 0.5f;
                    float radius = glm::length(maxBounds - center);
                    modelPrimitive.batchBounds.push_back({glm::vec4(center, radius)});
                }
                primitives.push_back(modelPrimitive);
            }
        }
    }
    void EngineModel::createBatchBoundsBuffer(){
        std::vector<glm::vec4> bounds;
        for(auto& primitive : primitives){
            primitive.firstBatch = static_cast<uint32_t>(bounds.size());
            for(const auto& batch : primitive.batchBounds)
                bounds.push_back(batch.sphere);
        }
        if(bounds.empty()) bounds.push_back(glm::vec4(0.f));
        VkDeviceSize bufferSize = sizeof(glm::vec4) * bounds.size();
        EngineBuffer stagingBuffer{
            engineDevice, sizeof(glm::vec4), static_cast<uint32_t>(bounds.size()),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer(bounds.data());
        batchBoundsBuffer = std::make_unique<EngineBuffer>(
            engineDevice, sizeof(glm::vec4), static_cast<uint32_t>(bounds.size()),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        engineDevice.copyBuffer(stagingBuffer.getBuffer(), batchBoundsBuffer->getBuffer(), bufferSize);
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
    void EngineModel::setMeshDescriptors(EngineDescriptorSetLayout& layout, EngineDescriptorPool& pool){
        meshSetLayout = &layout;
        meshPool = &pool;
    }
    void EngineModel::createBottomLevelAS(){
        if (primitives.empty() || vertexCount == 0) return;
        VkDevice device = engineDevice.device();
        auto getBuildSizes = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
        auto createAS = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
        auto destroyAS = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
        auto buildAS = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
        if(!getBuildSizes || !createAS || !destroyAS || !buildAS) throw std::runtime_error("Failed to load acceleration structure functions");
        std::vector<VkAccelerationStructureGeometryKHR> geometries;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR> ranges;
        std::vector<uint32_t> maxPrimitiveCounts;
        for(const auto& primitive : primitives){
            bool indexed = primitive.indexCount > 0;
            uint32_t indexCountForPrimitive = indexed ? primitive.indexCount : primitive.vertexCount;
            uint32_t primitiveCount = indexCountForPrimitive / 3;
            if(primitiveCount == 0) continue;
            VkAccelerationStructureGeometryKHR geometry{};
            geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
            auto& triangles = geometry.geometry.triangles;
            triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
            triangles.vertexStride = sizeof(Vertex);
            triangles.maxVertex = indexed ? vertexCount - 1 : primitive.vertexCount - 1;
            triangles.vertexData.deviceAddress = vertexBuffer->getDeviceAddress() + (indexed ? 0 : VkDeviceAddress(primitive.firstVertex) * sizeof(Vertex));
            if(indexed){
                triangles.indexType = VK_INDEX_TYPE_UINT32;
                triangles.indexData.deviceAddress = indexBuffer->getDeviceAddress();
            }else{
                triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
                triangles.indexData.deviceAddress = 0;
            }
            VkAccelerationStructureBuildRangeInfoKHR range{};
            range.primitiveCount = primitiveCount;
            range.primitiveOffset = indexed ? primitive.firstIndex * sizeof(uint32_t) : 0;
            range.firstVertex = 0;
            range.transformOffset = 0;
            geometries.push_back(geometry);
            ranges.push_back(range);
            maxPrimitiveCounts.push_back(primitiveCount);
        }
        if(geometries.empty()) return;
        VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildInfo.geometryCount = static_cast<uint32_t>(geometries.size());
        buildInfo.pGeometries = geometries.data();
        VkAccelerationStructureBuildSizesInfoKHR sizes{};
        sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        getBuildSizes(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, maxPrimitiveCounts.data(), &sizes);
        blasBuffer = std::make_unique<EngineBuffer>(engineDevice, sizes.accelerationStructureSize, 1, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkAccelerationStructureCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.buffer = blasBuffer->getBuffer();
        createInfo.size = sizes.accelerationStructureSize;
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        if(createAS(device, &createInfo, nullptr, &blas) != VK_SUCCESS) throw std::runtime_error("Failed to create BLAS");
        VkPhysicalDeviceAccelerationStructurePropertiesKHR asProperties{};
        asProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
        VkPhysicalDeviceProperties2 properties2{};
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        properties2.pNext = &asProperties;
        vkGetPhysicalDeviceProperties2(engineDevice.getPhysicalDevice(), &properties2);
        VkDeviceSize alignment = std::max<VkDeviceSize>(1, asProperties.minAccelerationStructureScratchOffsetAlignment);
        EngineBuffer scratchBuffer{engineDevice, sizes.buildScratchSize + alignment, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
        VkDeviceAddress scratchBase = scratchBuffer.getDeviceAddress();
        VkDeviceAddress scratchAddress = (scratchBase + alignment - 1) & ~(VkDeviceAddress(alignment) - 1);
        buildInfo.dstAccelerationStructure = blas;
        buildInfo.scratchData.deviceAddress = scratchAddress;
        std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> rangePointers;
        for(const auto& range : ranges) rangePointers.push_back(&range);
        VkCommandBuffer commandBuffer = engineDevice.beginSingleTimeCommands();
        buildAS(commandBuffer, 1, &buildInfo, rangePointers.data());
        engineDevice.endSingleTimeCommands(commandBuffer);
    }
}