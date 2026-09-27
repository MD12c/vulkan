#ifndef DEVICE_CLASS_H
#define DEVICE_CLASS_H

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vma/vk_mem_alloc.h>

class Window;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool     graphicsFamilyHasValue = false;
    bool     presentFamilyHasValue  = false;
};

struct AllocatedBuffer
{
    VkBuffer      buffer;
    VmaAllocation allocation;
};

struct AllocatedImage
{
    VkImage       image;
    VkImageView   imageView;
    VmaAllocation allocation;
};

class Device
{
private:
    VkInstance               instance;
    VmaAllocator             vmallocator;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
    Window&                  window;
    VkCommandPool            commandPool;

    VkDevice     device_;
    VkSurfaceKHR surface_;
    VkQueue      graphicsQueue_;
    VkQueue      presentQueue_;

    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    void createInstance();
    void createVMA();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool                     isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool                     checkValidationLayerSupport();
    QueueFamilyIndices       findQueueFamilies(VkPhysicalDevice device);
    void                     populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void                     hasGflwRequiredInstanceExtensions();
    SwapChainSupportDetails  querySwapChainSupport(VkPhysicalDevice device);

public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    Device(Window& window);
    ~Device();

    Device(const Device&)         = delete;
    void operator=(const Device&) = delete;
    Device(Device&&)              = delete;
    Device& operator=(Device&&)   = delete;

    // clang-format off
    VkCommandPool getCommandPool() const { return commandPool; }
    VkDevice      device()         const { return device_; }
    VkSurfaceKHR  surface()        const { return surface_; }
    VkQueue       graphicsQueue()  const { return graphicsQueue_; }
    VkQueue       presentQueue()   const { return presentQueue_; }
    // clang-format on

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }  // return querySwapChainSupport(physicalDevice);
    uint32_t                findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices      findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }  // return findQueueFamilies(physicalDevice);
    VkFormat                findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VmaAllocator            getVMA() const { return vmallocator; }
    VkPhysicalDevice        getPhysicalDevice() const { return physicalDevice; }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateInfo allocUsage, AllocatedBuffer& buffer);
    void createImageWithInfo(const VkImageCreateInfo& imageInfo, VmaAllocationCreateInfo allocInfo, AllocatedImage& image);

    VkCommandBuffer beginSingleTimeCommands();
    void            endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void            copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void            copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
    void            transitionImageLayout(VkImage image, VkImageSubresourceRange range, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkPhysicalDeviceProperties properties;
};

#endif