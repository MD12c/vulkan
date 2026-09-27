#ifndef RENDERER_CLASS_H
#define RENDERER_CLASS_H

#include "vkBackend/Device.h"
#include "vkBackend/Pipeline.h"
#include "vkBackend/Swapchain.h"
#include "vkBackend/DescriptorSetsManager.h"
#include "vkBackend/RenderPassManager.h"
#include "Window.h"
#include "Scene.h"

#include "Texture\TextureManager.h"
#include "Material/MaterialManager.h"
#include "Model/ModelManager.h"

class Renderer
{
private:
    float windowRGB[3] = {
        0.7f, 0.7f, 0.7f
    };

    Device&                      device;
    Window&                      window;
    RenderPassManager            renderPassManager;
    std::unique_ptr<SwapChain>   swapchain;
    std::unique_ptr<Pipeline>    mainPipeline;
    std::unique_ptr<Pipeline>    shadowPipeline;
    VkPipelineLayout             pipelineLayoutDefault;
    VkPipelineLayout             pipelineLayoutDepth2D;
    std::vector<VkCommandBuffer> commandBuffers;

    // Main Pass
    std::vector<VkFramebuffer> mainPassFramebuffers;
    AllocatedImage             mainDepthImage;

    // Depth Pass
    struct ShadowMap
    {
        VkFramebuffer  framebuffer;
        AllocatedImage image;
    };
    std::vector<ShadowMap> shadowMaps;

    void recreateSwapchain();

    void createPipelines();
    void createPipelineLayouts();

    void createMainDepthResources();
    void createMainFramebuffers();
    void createShadowFramebuffers();

    void createCommandBuffers();
    void recordCommandBuffer(int imageIndex, const Scene& scene);
    void freeCommandBuffers();

public:
    DescriptorSetsManager descriptorSetsManager;
    TextureManager        textureManager;
    MaterialManager       materialManager;
    ModelManager          modelManager;

    Renderer(Device& device, Window& window);
    ~Renderer();
    Renderer(const Renderer&)              = delete;
    Renderer& operator=(const Renderer&)   = delete;
    Renderer(const Renderer&&)             = delete;
    Renderer&& operator=(const Renderer&&) = delete;

    void drawFrame(const Scene& scene);
};

#endif