#ifndef RENDERER_CLASS_H
#define RENDERER_CLASS_H

#include "vulkan/vulkan.h"
#include "Device.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Window.h"
#include "Scene.h"

#include <array>

class Renderer
{
private:
    float windowRGB[3] = {
        0.7f, 0.7f, 0.7f
    };

    struct CameraBuffer
    {
        VkBuffer       buffer;
        VkDeviceMemory bufferMemory;
    };

    Device&                                                    device;
    Window&                                                    window;
    std::unique_ptr<SwapChain>                                 swapchain;
    std::unique_ptr<Pipeline>                                  pipeline;
    VkPipelineLayout                                           pipelineLayout;
    VkDescriptorSetLayout                                      globalSetLayout;
    VkDescriptorPool                                           descriptorPool;
    std::array<VkDescriptorSet, Globals::MAX_FRAMES_IN_FLIGHT> globalDescriptors;
    std::array<CameraBuffer, Globals::MAX_FRAMES_IN_FLIGHT>    cameraBuffers;
    std::vector<VkCommandBuffer>                               commandBuffers;

    void recreateSwapchain();
    void createPipelineLayout();
    void createCommandBuffers();
    void recordCommandBuffer(int imageIndex, const Scene& scene);
    void createPipeline();
    void createDescriptorLayout();
    void createDescriptorPool();
    void allocateDescriptors();
    void freeCommandBuffers();

public:
    Renderer(Device& device, Window& window);
    ~Renderer();
    Renderer(const Renderer&)              = delete;
    Renderer& operator=(const Renderer&)   = delete;
    Renderer(const Renderer&&)             = delete;
    Renderer&& operator=(const Renderer&&) = delete;

    void drawFrame(const Scene& scene);
};

#endif