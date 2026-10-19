#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include "EngineDevice.hpp"
namespace engine{
    class EngineTexture{
        public:
            EngineTexture(EngineDevice& device, uint32_t width, uint32_t height, const std::vector<std::uint8_t>& pixels);
            ~EngineTexture();
            EngineTexture(const EngineTexture&) = delete;
            EngineTexture& operator=(const EngineTexture&) = delete;
            VkDescriptorImageInfo descriptorInfo() const{
                return {sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
            }
        private:
            void CreateImage(uint32_t width, uint32_t height);
            void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
            void CreateImageView();
            void CreateSampler();
            EngineDevice& engineDevice;
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory imageMemory = VK_NULL_HANDLE;
            VkImageView imageView = VK_NULL_HANDLE;
            VkSampler sampler = VK_NULL_HANDLE;
    };
}