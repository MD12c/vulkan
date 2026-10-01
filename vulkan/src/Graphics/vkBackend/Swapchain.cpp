#include "Swapchain.h"
#include <vulkan/vulkan_core.h>

#include "Globals.h"
#include "RenderPassManager.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

SwapChain::SwapChain(Device& device, VkExtent2D extent)
    : device(device), windowExtent(extent)
{
    init();
}

SwapChain::SwapChain(Device& device, VkExtent2D extent, std::shared_ptr<SwapChain> previous)
    : device(device), windowExtent(extent), oldSwapchain(previous)
{
    init();
    oldSwapchain = nullptr;
}

void SwapChain::init()
{
    createSwapChain();
    createImageViews();
    createSyncObjects();
}

SwapChain::~SwapChain()
{
    for (auto imageView : mainColorImageViews)
        vkDestroyImageView(device.device(), imageView, nullptr);

    if (swapChain != nullptr)
    {
        vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
        swapChain = nullptr;
    }

    // Sync objects
    for (auto semaphores : renderFinishedSemaphores)
        vkDestroySemaphore(device.device(), semaphores, nullptr);

    for (size_t i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.device(), inFlightFences[i], nullptr);
    }
}

/**
 * acquireNextImage()
 *
 *      vkWaitForFences()
 *          inFlightFences if (unsignaled) stalls else runs
 *
 *      vkAcquireNextImageKHR()
 *          gets the next image index from the present system
 *          takes an unsignaled imageAvailableSemaphores and signals it when window manager lets go of it
 *
 * submitCommandBuffers()
 *
 *      if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
 *          waits for the image at imageIndex to be freed from use by other frames
 *
 *
 *      vkResetFences()
 *          unsignals inFlightFences
 *
 *      vkQueueSubmit()
 *          waits for imageAvailableSemaphores
 *          signals ... AFTER render is finished aka command buffer:
 *              renderFinishedSemaphores
 *              inFlightFences
 *
 *      vkQueuePresentKHR()
 *          waits on renderFinishedSemaphores
 *          passes image to the window manager
 */

/// @brief Stalls CPU if more than Globals::MAX_FRAMES_IN_FLIGHT have been rendered, gets the next available image
/// @param imageIndex empty `uint32_t*`
/// @return success?
VkResult SwapChain::acquireNextImage(uint32_t* imageIndex)
{
    vkWaitForFences(device.device(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(device.device(), swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
    // imageAvailableSemaphores[currentFrame], must be a not signaled semaphore, it gets signaled when window manager lets go of the image
    return result;
}

/// @brief submits the command buffer to the queue and passes image to the window manager
/// @param buffers command buffer
/// @param imageIndex current image index obtained from `acquireNextImage()`
/// @return success?
VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
{
    if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);

    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = buffers;

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitDstStageMask      = waitStages;

    VkSemaphore waitSemaphores[]    = { imageAvailableSemaphores[currentFrame] };
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    VkSemaphore signalSemaphores[]  = { renderFinishedSemaphores[*imageIndex] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);  // sets inFlightFences to unsigned

    // waits for imageAvailableSemaphores, signals renderFinishedSemaphores + inFlightFences AFTER render is finished
    if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;  // aka renderFinishedSemaphores

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;
    presentInfo.pImageIndices   = imageIndex;

    auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);  // waits on renderFinishedSemaphores and passes image to the window manager

    currentFrame = (currentFrame + 1) % Globals::MAX_FRAMES_IN_FLIGHT;

    return result;
}

/// @brief Create a `VkSwapchainKHR` and populate `swapChainImages` vector with all the formats queried from device
///
/// 1. Query `SwapChainSupportDetails` from device and choose the prefered features from the available
///
/// 2. Establish an image count
///
/// 3. Query `QueueFamilyIndices` from device to get the queue families
///
/// 4. Call `vkCreateSwapchainKHR()`
///
/// 5. Query `vkGetSwapchainImagesKHR()` to get num of images and populate swapChainImages vector
void SwapChain::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = device.findSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = device.findSwapSurfaceFormat();
    VkPresentModeKHR   presentMode   = chooseSwapPresentMode(swapChainSupport.presentModes);
    swapChainExtent                  = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = device.surface();
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices              = device.findPhysicalQueueFamilies();
    uint32_t           queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;  // allows both families access the images
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;        // Optional
        createInfo.pQueueFamilyIndices   = nullptr;  // Optional
    }

    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;  // mobile rotation
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;               // window blending
    createInfo.presentMode    = presentMode;                                     // (MAILBOX/FIFO)
    createInfo.clipped        = VK_TRUE;                                         // clips the pixels obscured by anothe window
    createInfo.oldSwapchain   = oldSwapchain ? oldSwapchain->swapChain : VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
    mainColorImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, mainColorImages.data());
}

/// @brief Goes through all the availablePresentModes and picks the one that is `VK_PRESENT_MODE_MAILBOX_KHR`
///
/// Othewise default to `VK_PRESENT_MODE_FIFO_KHR`
/// @param availablePresentModes list of VkPresentModeKHR enums
/// @return Present mode that is available if not the one desired
VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            std::cout << "Present mode: Mailbox" << std::endl;
            return availablePresentMode;
        }
    }

    // for (const auto& availablePresentMode : availablePresentModes)
    // {
    //     if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
    //     {
    //         std::cout << "Present mode: Immediate" << std::endl;
    //         return availablePresentMode;
    //     }
    // }

    std::cout << "Present mode: V-Sync" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
}

/// @brief Picks the extent aka image size from the capabilities
/// @param capabilities `VkSurfaceCapabilitiesKHR` containing supported size
/// @return size specified in capabilities otherwise gets the current window extent
VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    VkExtent2D actualExtent = windowExtent;
    actualExtent.width      = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height     = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

/// @brief For each `VkImage` create a `VkImageView`, who will be the handle for subsequent operations
void SwapChain::createImageViews()
{
    mainColorImageViews.resize(mainColorImages.size());

    for (size_t i = 0; i < mainColorImages.size(); i++)
    {
        VkImageViewCreateInfo viewInfo{};  // VkImageView gives more precise access to the underlying data
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = mainColorImages[i];
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = device.findSwapSurfaceFormat().format;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &mainColorImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create texture image view!");
    }
}

void SwapChain::createSyncObjects()
{
    imageAvailableSemaphores.resize(Globals::MAX_FRAMES_IN_FLIGHT);  // Rendering into this image is done, safe to present
    renderFinishedSemaphores.resize(getMainImageCount());            // This image is done being displayed, safe to render into again
    inFlightFences.resize(Globals::MAX_FRAMES_IN_FLIGHT);            // don't let the CPU start recording a new command buffer into this frame slot until GPU is done with the previous use of that same slot
    imagesInFlight.resize(getMainImageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
    {
        if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}
