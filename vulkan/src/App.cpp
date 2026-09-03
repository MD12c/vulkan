#include "App.h"

#include <iostream>
#include <stdexcept>
#include <array>

App::App()
    : window(width, height, name),
      device(window),
      swapchain(device, window.getExtent())
{
    loadModels();
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

std::vector<Model::Vertex> subdivide(std::vector<Model::Vertex>& vertices, int it)
{
    std::vector<Model::Vertex> newVertices;
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        glm::vec2 leftBot  = vertices[i].pos;
        glm::vec2 topCen   = vertices[i + 1].pos;
        glm::vec2 rightBot = vertices[i + 2].pos;

        glm::vec2 newVertex11 = leftBot;
        glm::vec2 newVertex12 = { (topCen.x + leftBot.x) / 2, (topCen.y + leftBot.y) / 2 };
        glm::vec2 newVertex13 = { (rightBot.x + leftBot.x) / 2, (rightBot.y + leftBot.y) / 2 };
        newVertices.push_back({ newVertex11 });
        newVertices.push_back({ newVertex12 });
        newVertices.push_back({ newVertex13 });

        glm::vec2 newVertex21 = newVertex12;
        glm::vec2 newVertex22 = topCen;
        glm::vec2 newVertex23 = { (rightBot.x + topCen.x) / 2, (rightBot.y + topCen.y) / 2 };
        newVertices.push_back({ newVertex21 });
        newVertices.push_back({ newVertex22 });
        newVertices.push_back({ newVertex23 });

        newVertices.push_back({ newVertex13 });
        newVertices.push_back({ newVertex23 });
        newVertices.push_back({ rightBot });
    }
    std::cout << it << "\n";

    if (it < 6)
        return subdivide(newVertices, it + 1);
    else
        return newVertices;
}

void App::loadModels()
{
    std ::vector<Model ::Vertex> vertices{
        { { -0.5f, 0.5f } },
        { { 0.0f, -0.5f } },
        { { 0.5f, 0.5f } }
    };

    // model = std::make_unique<Model>(device, vertices);
    model = std::make_unique<Model>(device, subdivide(vertices, 0));
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
        model->Bind(commandBuffers[i]);
        model->Draw(commandBuffers[i]);

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