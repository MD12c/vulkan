#ifndef SWAPCHAIN_CLASS_H
#define SWAPCHAIN_CLASS_H

#include "Device.h"
#include "RenderPassManager.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <memory>
#include <vector>

class SwapChain
{
private:
    Device&    device;
    VkExtent2D windowExtent;
    VkExtent2D swapChainExtent;

    VkSwapchainKHR             swapChain;
    std::shared_ptr<SwapChain> oldSwapchain;

    std::vector<VkImage>     mainColorImages;
    std::vector<VkImageView> mainColorImageViews;

    // Sync
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence>     inFlightFences;
    std::vector<VkFence>     imagesInFlight;
    size_t                   currentFrame = 0;

    void init();
    void createSwapChain();
    void createImageViews();
    void createSyncObjects();

    // Helper functions
    VkPresentModeKHR   chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D         chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

public:
    SwapChain(Device& deviceRef, VkExtent2D windowExtent);
    SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();
    SwapChain(const SwapChain&)       = delete;
    void operator=(const SwapChain&)  = delete;
    SwapChain(const SwapChain&&)      = delete;
    void operator=(const SwapChain&&) = delete;

    // clang-format off
    VkImageView getMainColorImageView(size_t index) const { return mainColorImageViews[index]; }
    size_t      getMainImageCount()                 const { return mainColorImages.size(); }
    VkExtent2D  getSwapChainExtent()                const { return swapChainExtent; }
    uint32_t    getWidth()                          const { return swapChainExtent.width; }
    uint32_t    getHeight()                         const { return swapChainExtent.height; }
    float       getExtentAspectRatio()              const { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
    size_t      getCrntFrame()                      const { return currentFrame; }
    // clang-format on

    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
};

#endif