#include "App.h"

#include <stdexcept>
#include <array>

App::App()
    : window(width, height, name),
      device(window),
      swapchain(device, window.getExtent())
{
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

App::~App()
{
    vkDeviceWaitIdle(device.device());
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void App::run()
{
    while (!window.ShouldClose())
    {
        window.StartFrame();
        window.updateFPS();

        drawFrame();

        window.EndFrame();
    }
}

void App::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;
    if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to create pipeline layout");
}
void App::createPipeline()
{
    PipelineConfigInfo pipelineConfig = Pipeline::defaultPipelineConfigInfo(swapchain.width(), swapchain.height());
    pipelineConfig.renderPass         = swapchain.getRenderPass();
    pipelineConfig.pipelineLayout     = pipelineLayout;
    pipeline                          = std::make_unique<Pipeline>(device, pipelineConfig, "Assets/Shaders/default.vert.spv", "Assets/Shaders/default.frag.spv");
}
void App::createCommandBuffers()
{
    commandBuffers.resize(swapchain.imageCount());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to allocate command buffers");

    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("[ERROR] failed to begin recording command buffer");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = swapchain.getRenderPass();
        renderPassInfo.framebuffer       = swapchain.getFrameBuffer(static_cast<uint32_t>(i));
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchain.getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color                = { windowRGB[0], windowRGB[1], windowRGB[2], 1.0f };
        clearValues[1].depthStencil.depth   = 1.0f;
        clearValues[1].depthStencil.stencil = 0;

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues    = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        pipeline->Bind(commandBuffers[i]);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("[ERROR] failed to record command buffers");
    }
}
void App::drawFrame()
{
    uint32_t imageIndex;
    auto     result = swapchain.acquireNextImage(&imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("[ERROR] failed to aquire swapchain image");

    result = swapchain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to present swapchain image");
}