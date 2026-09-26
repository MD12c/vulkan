#include "RenderPassManager.h"

#include "Swapchain.h"

RenderPassManager::RenderPassManager(Device& device)
    : device(device)
{
}

RenderPassManager::~RenderPassManager()
{
    vkDestroyRenderPass(device.device(), mainRenderPass, nullptr);
    vkDestroyRenderPass(device.device(), shadowRenderPass, nullptr);
}

void RenderPassManager::createMainRenderPassLayout(SwapChain& swapchain)
{
    const uint32_t                  NUM_ATTACHMENTS = 2;
    const uint32_t                  NUM_SUBPASSES   = 1;
    const uint32_t                  NUM_DEPENDACIES = 1;
    SwapChain::RenderPassConfigInfo renderpassConfig(NUM_ATTACHMENTS, NUM_SUBPASSES, NUM_DEPENDACIES);

    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format         = swapchain.getSwapChainImageFormat();
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;             // No MSAA
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // Clears color at start
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      // Save color at end, VK_ATTACHMENT_STORE_OP_DONT_CARE makes cool glitch effect
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // N/A (no stencil for color)
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // N/A
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        renderpassConfig.attachments[0].attachmentDescription          = colorAttachment;
        renderpassConfig.attachments[0].attachmentReference.attachment = 0;
        renderpassConfig.attachments[0].attachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format         = swapchain.getDepthFormat();
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // Clears depth at start
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // Discards depth at end
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // Discards at start stencil
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // Discards at start stencil
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        renderpassConfig.attachments[1].attachmentDescription          = depthAttachment;
        renderpassConfig.attachments[1].attachmentReference.attachment = 1;
        renderpassConfig.attachments[1].attachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    renderpassConfig.subpasses[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;  // opposite of COMPUTE
    renderpassConfig.subpasses[0].colorAttachmentCount    = 1;                                // can be N color, but only 1 depth allowed
    renderpassConfig.subpasses[0].pColorAttachments       = &renderpassConfig.attachments[0].attachmentReference;
    renderpassConfig.subpasses[0].pDepthStencilAttachment = &renderpassConfig.attachments[1].attachmentReference;

    renderpassConfig.dependencies[0].srcSubpass    = VK_SUBPASS_EXTERNAL;                                                                         // must wait until external work is done before start subPass 0
    renderpassConfig.dependencies[0].srcAccessMask = 0;                                                                                           // tells to flush the cache buffer into the VRAM, none when 0
    renderpassConfig.dependencies[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;  // don't let this subpass commands begin until any prev commands that reached this stage finish aka dependany filter
    renderpassConfig.dependencies[0].dstSubpass    = 0;                                                                                           // start subpass index
    renderpassConfig.dependencies[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;  // blocks targeted commands that reach to this point, prevents execution of stages that depend on this pass
    renderpassConfig.dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;         // invalidate data from local cache to forces GPU to get it in VRAM

    swapchain.createRenderPass(mainRenderPass, renderpassConfig);
}
