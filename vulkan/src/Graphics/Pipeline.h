#ifndef PIPELINE_H
#define PIPELINE_H

#include <string>
#include <vector>
#include "Device.h"

struct PipelineConfigInfo
{
    VkViewport                             viewport;
    VkRect2D                               scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo   multisampleInfo;
    VkPipelineColorBlendAttachmentState    colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo    colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo  depthStencilInfo;
    VkPipelineLayout                       pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass                           renderPass     = VK_NULL_HANDLE;
    uint32_t                               subpass        = 0;
};

class Pipeline
{
private:
    lve::MyEngineDevice& device;
    VkPipeline           graphicsPipeline;
    VkShaderModule       vertShaderModule;
    VkShaderModule       fragShaderModule;

    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& pipelineConfigInfo);
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

public:
    Pipeline(lve::MyEngineDevice&      device,
             const PipelineConfigInfo& pipelineConfigInfo,
             const std::string&        vertFilepath,
             const std::string&        fragFilepath);
    ~Pipeline();
    Pipeline(const Pipeline&)            = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void Bind(VkCommandBuffer commandBuffer);
    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
};
#endif