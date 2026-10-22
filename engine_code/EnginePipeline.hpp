#pragma once
#include "EngineDevice.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace engine{
    struct PipelineConfigInfo{
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };
    class EnginePipeline{
        public:
            EnginePipeline(EngineDevice &device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
            EnginePipeline(EngineDevice& device, const std::string& taskFilepath, const std::string& meshFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
            EnginePipeline(EngineDevice& device, const std::string& vertFilepath,
            const std::string& tescFilepath, const std::string& teseFilepath,
            const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
            ~EnginePipeline();
            EnginePipeline(const EnginePipeline&) = delete;
            EnginePipeline& operator=(const EnginePipeline&) = delete;
            void bind(VkCommandBuffer commandBuffer);
            static void defultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        private:
            static std::vector<char> readFile(const std::string& filepath);
            void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
            void createMeshGraphicsPipeline(const std::string& taskFilepath, const std::string& meshFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
            void createTessellationGraphicsPipeline(const std::string& vertFilepath, const std::string& tescFilepath, const std::string& teseFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
            VkShaderModule tescShaderModule = VK_NULL_HANDLE;
            VkShaderModule teseShaderModule = VK_NULL_HANDLE;
            EngineDevice& engineDevice;
            VkPipeline graphicsPipeline;
            VkShaderModule vertShaderModule = VK_NULL_HANDLE;
            VkShaderModule fragShaderModule = VK_NULL_HANDLE;
            VkShaderModule taskShaderModule = VK_NULL_HANDLE;
            VkShaderModule meshShaderModule = VK_NULL_HANDLE;
    };
}