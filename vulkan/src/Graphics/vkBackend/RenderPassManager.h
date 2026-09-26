#ifndef RENDER_PASS_MANAGER_H
#define RENDER_PASS_MANAGER_H

#include <vulkan/vulkan_core.h>

#include "Device.h"

class SwapChain;

class RenderPassManager
{
private:
    Device& device;
    VkRenderPass mainRenderPass;
    VkRenderPass shadowRenderPass;

public:
    RenderPassManager(Device& device);
    ~RenderPassManager();

    void createMainRenderPassLayout(SwapChain& swapchain);

    VkRenderPass getMainRenderPass() const { return mainRenderPass; }
    VkRenderPass getShadowRenderPass() const { return shadowRenderPass; }
};

#endif