#include "Renderer.h"

#include <stdexcept>
#include <cassert>
#include <array>

#include "Model\Model.h"
#include "Cameras/Camera.h"

Renderer::Renderer(Device& device, Window& window)
    : device(device), window(window)
{
    createDescriptorLayout();
    createDescriptorPool();
    allocateDescriptors();
    createPipelineLayout();
    recreateSwapchain();
    createCommandBuffers();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(device.device());
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), globalSetLayout, nullptr);
    vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);

    for (auto& cameraBuffer : cameraBuffers)
    {
        vkDestroyBuffer(device.device(), cameraBuffer.buffer, nullptr);
        vkFreeMemory(device.device(), cameraBuffer.bufferMemory, nullptr);
    }
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
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &globalDescriptors[currentFrame], 0, nullptr);
    scene.camera->updateUniforms(cameraBuffers[currentFrame].bufferMemory);
    scene.model->Bind(commandBuffers[imageIndex]);

    for (int j = 0; j < 4; j++)
    {
        PushConstantData push{};
        push.offset = { -0.5f + frame * 0.0002f, -0.4f + j * 0.25f };
        push.color  = { 0.0f, 0.0f, 0.2f + 0.2f * j };
        vkCmdPushConstants(commandBuffers[imageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
        scene.model->Draw(commandBuffers[imageIndex]);
    }

    vkCmdEndRenderPass(commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
        throw std::runtime_error("[ERROR] failed to record command buffers");

    currentFrame = (currentFrame + 1) % Globals::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::createDescriptorLayout()
{
    VkDescriptorSetLayoutBinding bufferBinding{};
    bufferBinding.binding         = 0;
    bufferBinding.descriptorCount = 1;
    bufferBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;  // it's a uniform buffer binding
    bufferBinding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;         // we use it from the vertex shader

    VkDescriptorSetLayoutCreateInfo setinfo{};
    setinfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pNext        = nullptr;
    setinfo.flags        = 0;
    setinfo.bindingCount = 1;
    setinfo.pBindings    = &bufferBinding;

    vkCreateDescriptorSetLayout(device.device(), &setinfo, nullptr, &globalSetLayout);
}

void Renderer::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> sizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }  // create a descriptor pool that will hold 10 uniform buffers
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags         = 0;
    pool_info.maxSets       = 10;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes    = sizes.data();

    vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &descriptorPool);
}

void Renderer::allocateDescriptors()
{
    VkBufferUsageFlags    usage      = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    for (int i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
    {
        device.createBuffer(Camera::payloadSize, usage, properties, cameraBuffers[i].buffer, cameraBuffers[i].bufferMemory);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.pNext              = nullptr;
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &globalSetLayout;

        vkAllocateDescriptorSets(device.device(), &allocInfo, &globalDescriptors[i]);

        VkDescriptorBufferInfo binfo{};
        binfo.buffer = cameraBuffers[i].buffer;
        binfo.offset = 0;
        binfo.range  = Camera::payloadSize;

        VkWriteDescriptorSet setWrite{};
        setWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        setWrite.pNext           = nullptr;
        setWrite.dstBinding      = 0;
        setWrite.dstSet          = globalDescriptors[i];
        setWrite.descriptorCount = 1;
        setWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setWrite.pBufferInfo     = &binfo;

        vkUpdateDescriptorSets(device.device(), 1, &setWrite, 0, nullptr);
    }
}

void Renderer::createPipelineLayout()
{
    VkPushConstantRange pushConstantRangeInfo{};
    pushConstantRangeInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRangeInfo.offset     = 0;
    pushConstantRangeInfo.size       = sizeof(PushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 1;  // descriptors
    pipelineLayoutInfo.pushConstantRangeCount = 1;  // pushconstants
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRangeInfo;
    pipelineLayoutInfo.pSetLayouts            = &globalSetLayout;

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
