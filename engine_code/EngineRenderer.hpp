#pragma once
#include "EngineWindow.hpp"
#include "EngineDevice.hpp"
#include "EngineSwapChain.hpp"
#include <memory>
#include <vector>
#include <cassert>
namespace engine{
    class EngineRenderer{
        public:
            EngineRenderer(EngineWindow &window, EngineDevice &device);
            ~EngineRenderer();
            EngineRenderer(const EngineRenderer &) = delete;
            EngineRenderer &operator=(const EngineRenderer &) = delete;
            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
            VkRenderPass getSwapChainRenderPass() const {return etSwapChain->getRenderPass();}
            float getAspectRatio() const {return etSwapChain->extentAspectRatio();}
            bool isFrameInProgress() const {return isFrameStarted;}
            VkCommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
                return commandBuffers[currentFrameIndex];
            }
            int getFrameIndex() const{
                assert(isFrameStarted && "Cannot get frame index when frame not in progress");
                return currentFrameIndex;
            }
        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();
            EngineWindow& etWindow;
            EngineDevice& etDevice;
            std::unique_ptr<EngineSwapChain> etSwapChain;
            std::vector<VkCommandBuffer> commandBuffers;
            uint32_t currentImageIndex;
            int currentFrameIndex{0};
            bool isFrameStarted = false;
    };
}