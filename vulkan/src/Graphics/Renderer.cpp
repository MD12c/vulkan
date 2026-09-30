#include "Renderer.h"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <array>

#include "Lighting/LightManager.h"
#include "Lighting/ShadowMapDimensions.h"
#include "Material/MaterialManager.h"
#include "Model/Mesh.h"
#include "Model/ModelManager.h"
#include "vkBackend/DescriptorSetsManager.h"
#include "vkBackend/Swapchain.h"
#include "vulkan/vulkan_core.h"

#include "Model\Model.h"
#include "Cameras/Camera.h"

Renderer::Renderer(Device& device, Window& window)
    : device(device),
      window(window),
      renderPassManager(device),
      descriptorSetsManager(device),
      textureManager(device, descriptorSetsManager),
      materialManager(device, textureManager, descriptorSetsManager),
      modelManager(device, materialManager),
      lightsManager(device, device.getDepthFormat(), renderPassManager.getShadowRenderPass(), descriptorSetsManager)
{
    createPipelineLayouts();
    recreateSwapchain();
    createMainDepthResources();
    createCommandBuffers();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(device.device());
    vkDestroyPipelineLayout(device.device(), pipelineLayoutDefault, nullptr);
    vkDestroyPipelineLayout(device.device(), pipelineLayoutDepth2D, nullptr);

    vkDestroyImageView(device.device(), mainDepthImage.imageView, nullptr);
    vmaDestroyImage(device.getVMA(), mainDepthImage.image, mainDepthImage.allocation);

    for (auto framebuffer : mainPassFramebuffers)
        vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
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

    if (result && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("[ERROR] failed to aquire swapchain->image");

    recordCommandBuffer(imageIndex, scene);
    result = swapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.getFramebufferResized())
    {
        window.resetWindowResized();
        recreateSwapchain();
        return;
    }
    if (result)
        throw std::runtime_error("[ERROR] failed to present swapchain->image");
}

void Renderer::recordCommandBuffer(int imageIndex, const Scene& scene)
{
    uint32_t currentFrame = static_cast<uint32_t>(swapchain->getCrntFrame());

    {  // Begin command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo))
            throw std::runtime_error("[ERROR] failed to begin recording command buffer");
    }
    {  // Begin shadow pass
        VkClearValue clearValues{};
        clearValues.depthStencil.depth   = 1.0f;
        clearValues.depthStencil.stencil = 0;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = renderPassManager.getShadowRenderPass();
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { ShadowMapDimensions::SHADOW_MAP_WIDTH, ShadowMapDimensions::SHADOW_MAP_HEIGHT };
        renderPassInfo.clearValueCount   = 1;
        renderPassInfo.pClearValues      = &clearValues;

        for (const auto& dirLight : scene.directionLights)
        {
            renderPassInfo.framebuffer = lightsManager.dir.framebuffers[dirLight.layerIndex];
            vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            shadowPipeline->Bind(commandBuffers[imageIndex]);

            for (const auto& model : scene.models)
                model.DrawShadow(commandBuffers[imageIndex], pipelineLayoutDepth2D, dirLight, Transform({ {}, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.02f) }));

            vkCmdEndRenderPass(commandBuffers[imageIndex]);
        }
    }
    {  // Begin main render pass
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color                = { { windowRGB[0], windowRGB[1], windowRGB[2], 1.0f } };
        clearValues[1].depthStencil.depth   = 1.0f;
        clearValues[1].depthStencil.stencil = 0;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = renderPassManager.getMainRenderPass();
        renderPassInfo.framebuffer       = mainPassFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();
        renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues      = clearValues.data();

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

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        mainPipeline->Bind(commandBuffers[imageIndex]);
        vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutDefault, 0, 1, &scene.camera->cameraDescriptors.descriptorSets[currentFrame], 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutDefault, 2, 1, &lightsManager.dir.descriptorSets[currentFrame], 0, nullptr);
        scene.camera->updateUniforms(scene.camera->cameraDescriptors.buffers[currentFrame].allocation);
        lightsManager.ExportUniformsTo(currentFrame, scene.directionLights);

        for (const auto& model : scene.models)
            model.Draw(commandBuffers[imageIndex], pipelineLayoutDefault, materialManager, Transform({ {}, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.02f) }));

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
    }
    if (vkEndCommandBuffer(commandBuffers[imageIndex]))
        throw std::runtime_error("[ERROR] failed to record command buffers");
}

void Renderer::createPipelineLayouts()
{
    {  // main pipeline layout
        VkPushConstantRange pushConstantRangeInfo{};
        pushConstantRangeInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRangeInfo.offset     = 0;
        pushConstantRangeInfo.size       = sizeof(Model::PushConstModel);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount         = 3;  // descriptors
        pipelineLayoutInfo.pushConstantRangeCount = 1;  // pushconstants
        pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRangeInfo;

        VkDescriptorSetLayout setLayouts[3] = { descriptorSetsManager.getGlobalSetLayout(), descriptorSetsManager.getTextureSetLayout(), descriptorSetsManager.getShadowSetLayout() };
        pipelineLayoutInfo.pSetLayouts      = setLayouts;

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayoutDefault))
            throw std::runtime_error("[ERROR] failed to create pipeline layout Default");
    }
    {  // shadow pipeline layout
        VkPushConstantRange pushConstantRangeInfo{};
        pushConstantRangeInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRangeInfo.offset     = 0;
        pushConstantRangeInfo.size       = sizeof(ShadowMapDimensions::Shadow2DPushConst);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRangeInfo;

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayoutDepth2D))
            throw std::runtime_error("[ERROR] failed to create pipeline layout Depth2D");
    }
}

void Renderer::createPipelines()
{
    {  // main pipeline creation
        assert(pipelineLayoutDefault != nullptr && "Cannot create pipeline before pipeline layout");
        renderPassManager.recreateMainRenderPassLayout();

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass            = renderPassManager.getMainRenderPass();
        pipelineConfig.pipelineLayout        = pipelineLayoutDefault;
        pipelineConfig.bindingDescriptions   = Vertex::getBindingDescriptions();
        pipelineConfig.attributeDescriptions = Vertex::getAttributeDescriptions();
        mainPipeline                         = std::make_unique<Pipeline>(device, pipelineConfig, "Assets/Shaders/default.vert.spv", "Assets/Shaders/default.frag.spv");
    }
    {  // shadow pipeline creation
        assert(pipelineLayoutDepth2D != nullptr && "Cannot create pipeline before pipeline layout");
        renderPassManager.recreateShadowRenderPassLayout();

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(ShadowMapDimensions::SHADOW_MAP_WIDTH);
        viewport.height   = static_cast<float>(ShadowMapDimensions::SHADOW_MAP_HEIGHT);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ { 0, 0 }, { ShadowMapDimensions::SHADOW_MAP_WIDTH, ShadowMapDimensions::SHADOW_MAP_HEIGHT } };

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass              = renderPassManager.getShadowRenderPass();
        pipelineConfig.pipelineLayout          = pipelineLayoutDepth2D;
        pipelineConfig.bindingDescriptions     = Vertex::getBindingDescriptions();
        pipelineConfig.attributeDescriptions   = Vertex::getAttributeDescriptions();
        pipelineConfig.dynamicStateEnables     = {};
        pipelineConfig.dynamicStateInfo        = {};
        pipelineConfig.viewportInfo.pViewports = &viewport;
        pipelineConfig.viewportInfo.pScissors  = &scissor;
        shadowPipeline                         = std::make_unique<Pipeline>(device, pipelineConfig, "Assets/Shaders/shadowMap2D.vert.spv", "Assets/Shaders/shadowMap2D.frag.spv");
    }
}

void Renderer::createCommandBuffers()
{
    commandBuffers.resize(swapchain->getMainImageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()))
        throw std::runtime_error("[ERROR] failed to allocate command buffers");
}

void Renderer::createMainDepthResources()
{
    VkFormat depthFormat = device.getDepthFormat();

    {  // Depth image creation
        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = swapchain->getSwapChainExtent().width;
        imageInfo.extent.height = swapchain->getSwapChainExtent().height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = depthFormat;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags         = 0;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage         = VMA_MEMORY_USAGE_AUTO;
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        device.createImageWithInfo(imageInfo, allocInfo, mainDepthImage);
    }
    {  // Depth image view creation
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = mainDepthImage.image;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = depthFormat;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &mainDepthImage.imageView))
            throw std::runtime_error("failed to create texture image view!");
    }
    {  // Depth image framebuffer creation
        const size_t imageCount = swapchain->getMainImageCount();
        mainPassFramebuffers.resize(imageCount);
        for (size_t i = 0; i < imageCount; i++)
        {
            std::array<VkImageView, 2> attachments = { swapchain->getMainColorImageView(i), mainDepthImage.imageView };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = renderPassManager.getMainRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments    = attachments.data();
            framebufferInfo.width           = swapchain->getSwapChainExtent().width;
            framebufferInfo.height          = swapchain->getSwapChainExtent().height;
            framebufferInfo.layers          = 1;

            if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &mainPassFramebuffers[i]))
                throw std::runtime_error("failed to create framebuffer!");
        }
    }
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
        if (swapchain->getMainImageCount() != commandBuffers.size())
        {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }

    createPipelines();
}

void Renderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}
