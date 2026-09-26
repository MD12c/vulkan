#ifndef PIPELINE_H
#define PIPELINE_H

#include <string>
#include <vector>
#include "Device.h"

struct PipelineConfigInfo
{
    VkPipelineViewportStateCreateInfo              viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo         inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo         rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo           multisampleInfo;
    VkPipelineDepthStencilStateCreateInfo          depthStencilInfo;
    VkPipelineColorBlendStateCreateInfo            colorBlendInfo;
    VkPipelineColorBlendAttachmentState            colorBlendAttachment;
    std::vector<VkDynamicState>                    dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo               dynamicStateInfo;
    std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkPipelineLayout                               pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass                                   renderPass     = VK_NULL_HANDLE;
    uint32_t                                       subpass        = 0;
    PipelineConfigInfo()                                          = default;
    PipelineConfigInfo(const PipelineConfigInfo&)                 = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&)      = delete;
};

class Pipeline
{
private:
    Device&        device;
    VkPipeline     graphicsPipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& pipelineConfigInfo);
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

public:
    Pipeline(Device&                   device,
             const PipelineConfigInfo& pipelineConfigInfo,
             const std::string&        vertFilepath,
             const std::string&        fragFilepath);
    ~Pipeline();
    Pipeline(const Pipeline&)            = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void        Bind(VkCommandBuffer commandBuffer);
    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
};
#endif