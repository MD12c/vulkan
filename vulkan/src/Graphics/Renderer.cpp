#include "Renderer.h"

#include <stdexcept>
#include <cassert>
#include <array>

#include "Material/MaterialManager.h"
#include "Model/ModelManager.h"
#include "vkBackend/DescriptorSetsManager.h"
#include "vulkan/vulkan_core.h"

#include "Model\Model.h"
#include "Cameras/Camera.h"

Renderer::Renderer(Device& device, Window& window)
    : device(device),
      window(window),
      descriptorSetsManager(device),
      textureManager(device, descriptorSetsManager),
      materialManager(device, textureManager),
      modelManager(device, materialManager)
{
    createPipelineLayout();
    recreateSwapchain();
    createCommandBuffers();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(device.device());
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void Renderer::drawFrame(const Scene& scene)
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

    recordCommandBuffer(imageIndex, scene);
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

void Renderer::recordCommandBuffer(int imageIndex, const Scene& scene)
{
    uint32_t currentFrame = 0;

    static int frame = 0;
    frame            = (frame + 1) % 10000;

    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("[ERROR] failed to begin recording command buffer");
    }

    {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color                = { { windowRGB[0], windowRGB[1], windowRGB[2], 1.0f } };
        clearValues[1].depthStencil.depth   = 1.0f;
        clearValues[1].depthStencil.stencil = 0;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = swapchain->getRenderPass();
        renderPassInfo.framebuffer       = swapchain->getFrameBuffer(static_cast<uint32_t>(imageIndex));
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();
        renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues      = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    {
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
    }

    pipeline->Bind(commandBuffers[imageIndex]);
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetsManager.bufferDescriptors[0].descriptorSets[currentFrame], 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textureManager.descriptorSet, 0, nullptr);
    scene.camera->updateUniforms(descriptorSetsManager.bufferDescriptors[0].Buffers[currentFrame].bufferMemory);

    for (const auto& model : scene.models)
        model.Draw(commandBuffers[imageIndex], pipelineLayout, Transform({ {}, glm::quat(0.0f, 1.0f, 0.0f, 0.0f), glm::vec3(0.2f) }));

    vkCmdEndRenderPass(commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(commandBuffers[imageIndex]))
        throw std::runtime_error("[ERROR] failed to record command buffers");

    currentFrame = (currentFrame + 1) % Globals::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::createPipelineLayout()
{
    VkPushConstantRange pushConstantRangeInfo{};
    pushConstantRangeInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRangeInfo.offset     = 0;
    pushConstantRangeInfo.size       = sizeof(Model::PushConst);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 2;  // descriptors
    pipelineLayoutInfo.pushConstantRangeCount = 1;  // pushconstants
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRangeInfo;

    VkDescriptorSetLayout setLayouts[2] = { descriptorSetsManager.getGlobalSetLayouts(), descriptorSetsManager.getTextureSetLayout() };
    pipelineLayoutInfo.pSetLayouts      = setLayouts;

    if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to create pipeline layout");
}

void Renderer::createPipeline()
{
    assert(swapchain != nullptr && "Cannot create pipeline before swap chain");
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass     = swapchain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipeline                      = std::make_unique<Pipeline>(device, pipelineConfig, "Assets/Shaders/default.vert.spv", "Assets/Shaders/default.frag.spv");
}

void Renderer::createCommandBuffers()
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

void Renderer::recreateSwapchain()
{
    auto extent = window.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.device());
    if (swapchain == nullptr)
        swapchain = std::make_unique<SwapChain>(device, extent);
    else
    {
        swapchain = std::make_unique<SwapChain>(device, extent, std::move(swapchain));
        if (swapchain->imageCount() != commandBuffers.size())
        {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }

    createPipeline();
}

void Renderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}
