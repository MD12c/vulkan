#include "App.h"

#include <iostream>
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct PushConstantData
{
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

App::App()
    : window(width, height, name),
      device(window)
{
    loadModels();
    createPipelineLayout();
    recreateSwapchain();
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

    if (it < 2)
        return subdivide(newVertices, it + 1);
    else
        return newVertices;
}

void App::loadModels()
{
    std ::vector<Model ::Vertex> vertices{
        { { -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
    };

    model = std::make_unique<Model>(device, vertices);
    // model = std::make_unique<Model>(device, subdivide(vertices, 0));
}

void App::createPipelineLayout()
{
    VkPushConstantRange pushConstantRangeInfo{};
    pushConstantRangeInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRangeInfo.offset     = 0;
    pushConstantRangeInfo.size       = sizeof(PushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRangeInfo;
    if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to create pipeline layout");
}

void App::createPipeline()
{
    assert(swapchain != nullptr && "Cannot create pipeline before swap chain");
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass     = swapchain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipeline                      = std::make_unique<Pipeline>(device, pipelineConfig, "Assets/Shaders/default.vert.spv", "Assets/Shaders/default.frag.spv");
}

void App::createCommandBuffers()
{
    commandBuffers.resize(swapchain->imageCount());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to allocate command buffers");
}

void App::freeCommandBuffers()
{
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

void App::recordCommandBuffer(int imageIndex)
{
    static int frame = 0;
    frame = (frame + 1) % 1000;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to begin recording command buffer");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = swapchain->getRenderPass();
    renderPassInfo.framebuffer       = swapchain->getFrameBuffer(static_cast<uint32_t>(imageIndex));
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color                = { windowRGB[0], windowRGB[1], windowRGB[2], 1.0f };
    clearValues[1].depthStencil.depth   = 1.0f;
    clearValues[1].depthStencil.stencil = 0;

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(swapchain->getSwapChainExtent().width);
    viewport.height   = static_cast<float>(swapchain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ { 0, 0 }, swapchain->getSwapChainExtent() };
    vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

    pipeline->Bind(commandBuffers[imageIndex]);
    model->Bind(commandBuffers[imageIndex]);

    for (int j = 0; j < 4; j++)
    {
        PushConstantData push{};
        push.offset = { -0.5f + frame * 0.0002f, -0.4f + j * 0.25f };
        push.color  = { 0.0f, 0.0f, 0.2f + 0.2f * j };
        vkCmdPushConstants(commandBuffers[imageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
        model->Draw(commandBuffers[imageIndex]);
    }

    vkCmdEndRenderPass(commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to record command buffers");
}

void App::recreateSwapchain()
{
    auto extent = window.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.device());
    if (swapchain == nullptr)
        swapchain = std::make_unique<lve::MyEngineSwapChain>(device, extent);
    else
    {
        swapchain = std::make_unique<lve::MyEngineSwapChain>(device, extent, std::move(swapchain));
        if (swapchain->imageCount() != commandBuffers.size())
        {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }

    createPipeline();
}

void App::drawFrame()
{
    uint32_t imageIndex;
    auto     result = swapchain->acquireNextImage(&imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("[ERROR] failed to aquire swapchain->image");

    recordCommandBuffer(imageIndex);
    result = swapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.getFramebufferResized())
    {
        window.resetWindowResized();
        recreateSwapchain();
        return;
    }
    if (result != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to present swapchain->image");
}