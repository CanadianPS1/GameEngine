#include <stdexcept>
#include "EngineTexture.hpp"
namespace engine{
    EngineTexture::EngineTexture(EngineDevice& device, uint32_t width, uint32_t height, const std::vector<std::uint8_t>& pixels) : engineDevice{device}{
        if(width == 0 || height == 0 || pixels.size() != static_cast<std::size_t>(width) * height * 4) throw std::runtime_error("Invalid texture dimensions or pixel data");
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * 4;
        engineDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingMemory);
        void* data;
        vkMapMemory(engineDevice.device(), stagingMemory, 0, imageSize, 0, &data);
        std::memcpy(data, pixels.data(), static_cast<std::size_t>(imageSize));
        vkUnmapMemory(engineDevice.device(), stagingMemory);
        try{
            CreateImage(width, height);
            TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            engineDevice.copyBufferToImage(stagingBuffer, image, width, height, 1);
            TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            CreateImageView();
            CreateSampler();
        }catch(...){
            vkDestroyBuffer(engineDevice.device(), stagingBuffer, nullptr);
            vkFreeMemory(engineDevice.device(), stagingMemory, nullptr);
            throw;
        }
        vkDestroyBuffer(engineDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(engineDevice.device(), stagingMemory, nullptr);
    }
    EngineTexture::~EngineTexture(){
        vkDestroySampler(engineDevice.device(), sampler, nullptr);
        vkDestroyImageView(engineDevice.device(), imageView, nullptr);
        vkDestroyImage(engineDevice.device(), image, nullptr);
        vkFreeMemory(engineDevice.device(), imageMemory, nullptr);
    }
    void EngineTexture::CreateImage(uint32_t width, uint32_t height){
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent = {width, height, 1};
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.format = VK_FORMAT_R8G8B8A8_SRGB;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        engineDevice.createImageWithInfo(info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
    }
    void EngineTexture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout){
        VkCommandBuffer commandBuffer = engineDevice.beginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
        if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }else{
            vkEndCommandBuffer(commandBuffer);
            vkFreeCommandBuffers(engineDevice.device(), engineDevice.getCommandPool(), 1, &commandBuffer);
            throw std::runtime_error("Unsupported texture image layout transition");
        }
        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        engineDevice.endSingleTimeCommands(commandBuffer);
    }
    void EngineTexture::CreateImageView(){
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_SRGB;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        if(vkCreateImageView(engineDevice.device(), &info, nullptr, &imageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture image view");
    }
    void EngineTexture::CreateSampler(){
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.anisotropyEnable = VK_TRUE;
        info.maxAnisotropy = engineDevice.properties.limits.maxSamplerAnisotropy;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        if(vkCreateSampler(engineDevice.device(), &info, nullptr, &sampler) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture sampler");
    }
}