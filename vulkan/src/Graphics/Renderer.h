#ifndef RENDERER_CLASS_H
#define RENDERER_CLASS_H

#include "vulkan/vulkan.h"
#include "vkBackend/Device.h"
#include "vkBackend/Pipeline.h"
#include "vkBackend/Swapchain.h"
#include "vkBackend/DescriptorSetsManager.h"
#include "Window.h"
#include "Scene.h"

#include <array>

class Renderer
{
private:
    float windowRGB[3] = {
        0.7f, 0.7f, 0.7f
    };

    Device&                      device;
    Window&                      window;
    std::unique_ptr<SwapChain>   swapchain;
    std::unique_ptr<Pipeline>    pipeline;
    VkPipelineLayout             pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffers;

    void recreateSwapchain();
    void createPipelineLayout();
    void createCommandBuffers();
    void recordCommandBuffer(int imageIndex, const Scene& scene);
    void createPipeline();
    void freeCommandBuffers();

public:
    DescriptorSetsManager descriptorSetsManager;

    Renderer(Device& device, Window& window);
    ~Renderer();
    Renderer(const Renderer&)              = delete;
    Renderer& operator=(const Renderer&)   = delete;
    Renderer(const Renderer&&)             = delete;
    Renderer&& operator=(const Renderer&&) = delete;

    void drawFrame(const Scene& scene);
};

#endif