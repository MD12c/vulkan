#pragma once

#include "device.h"
#include "RenderPassManager.h"

#include <vulkan/vulkan.h>

#include <cstddef>
#include <memory>
#include <vector>

class SwapChain
{
private:
    Device&    device;
    VkExtent2D windowExtent;
    VkExtent2D swapChainExtent;
    VkFormat   swapChainImageFormat;

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
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR   chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D         chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

public:
    struct RenderPassConfigInfo
    {
        struct Attachment
        {
            VkAttachmentDescription attachmentDescription{};
            VkAttachmentReference   attachmentReference{};
        };
        std::vector<Attachment>           attachments{};
        std::vector<VkSubpassDescription> subpasses{};
        std::vector<VkSubpassDependency>  dependencies{};

        RenderPassConfigInfo(size_t numAttachments, size_t numSubpasses, size_t numDependancies)
        {
            attachments.resize(numAttachments);
            subpasses.resize(numSubpasses);
            dependencies.resize(numDependancies);
        }

        std::vector<VkAttachmentDescription> groupAttachments()
        {
            std::vector<VkAttachmentDescription> attachmentDescription;
            for (auto& attachment : attachments) attachmentDescription.push_back(attachment.attachmentDescription);
            return attachmentDescription;
        }
    };

    SwapChain(Device& deviceRef, VkExtent2D windowExtent);
    SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();
    SwapChain(const SwapChain&)       = delete;
    void operator=(const SwapChain&)  = delete;
    SwapChain(const SwapChain&&)      = delete;
    void operator=(const SwapChain&&) = delete;

    void createRenderPass(VkRenderPass& renderPass, RenderPassConfigInfo& renderPassConfigInfo);

    // clang-format off
    VkImageView   getMainColorImageView(int index) const { return mainColorImageViews[index]; }
    size_t        getMainImageCount()              const { return mainColorImages.size(); }
    VkFormat      getSwapChainImageFormat()        const { return swapChainImageFormat; }
    VkExtent2D    getSwapChainExtent()             const { return swapChainExtent; }
    uint32_t      getWidth()                       const { return swapChainExtent.width; }
    uint32_t      getHeight()                      const { return swapChainExtent.height; }
    float         getExtentAspectRatio()           const { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
    VkFormat      getDepthFormat()                 const { return device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                                                                               VK_IMAGE_TILING_OPTIMAL,
                                                                                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); }
    // clang-format on
    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
};
