#pragma once

#include "device.h"
#include "Globals.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

class SwapChain
{
private:
    VkFormat   swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass               renderPass;

    std::vector<VkImage>        swapChainImages;
    std::vector<VkImageView>    swapChainImageViews;
    std::vector<VkImage>        depthImages;
    std::vector<VkDeviceMemory> depthImageMemorys;
    std::vector<VkImageView>    depthImageViews;

    Device&    device;
    VkExtent2D windowExtent;

    VkSwapchainKHR             swapChain;
    std::shared_ptr<SwapChain> oldSwapchain;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence>     inFlightFences;
    std::vector<VkFence>     imagesInFlight;
    size_t                   currentFrame = 0;

    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR   chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D         chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

public:
    SwapChain(Device& deviceRef, VkExtent2D windowExtent);
    SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();

    SwapChain(const SwapChain&)      = delete;
    void operator=(const SwapChain&) = delete;

    // clang-format off
    VkFramebuffer getFrameBuffer(int index) const { return swapChainFramebuffers[index]; }
    VkRenderPass  getRenderPass()           const { return renderPass; }
    VkImageView   getImageView(int index)   const { return swapChainImageViews[index]; }
    size_t        imageCount()              const { return swapChainImages.size(); }
    VkFormat      getSwapChainImageFormat() const { return swapChainImageFormat; }
    VkExtent2D    getSwapChainExtent()      const { return swapChainExtent; }
    uint32_t      width()                   const { return swapChainExtent.width; }
    uint32_t      height()                  const { return swapChainExtent.height; }
    float         extentAspectRatio()       const { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
    VkFormat      findDepthFormat()         const { return device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                                                                        VK_IMAGE_TILING_OPTIMAL,
                                                                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); }
    // clang-format on
    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
};
