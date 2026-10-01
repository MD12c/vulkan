#include "Device.h"

#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>
#include <unordered_set>

#include "../Window.h"

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

// class member functions
Device::Device(Window& window) : window(window)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createVMA();
    createCommandPool();
}

Device::~Device()
{
    vkDestroyCommandPool(device_, commandPool, nullptr);
    vmaDestroyAllocator(vmallocator);
    vkDestroyDevice(device_, nullptr);

    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface_, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void Device::createVMA()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device         = device_;
    allocatorInfo.instance       = instance;

    if (vmaCreateAllocator(&allocatorInfo, &vmallocator))
        throw std::runtime_error("failed to create VMA");
}

/// @brief Create a VkInstance
///
/// 1. Call `checkValidationLayerSupport()`
///
/// 2. Specify the app info
///
/// 3. Call `getRequiredExtensions()` and put in app info
///
/// 4. Call `populateDebugMessengerCreateInfo()`
///
/// 5. Create VkInstance with `vkCreateInstance()`
///
/// 6. Call `hasGflwRequiredInstanceExtensions()` to check if GLFW has them
void Device::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
        throw std::runtime_error("validation layers requested, but not available!");

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions                    = getRequiredExtensions();
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext             = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance))
        throw std::runtime_error("failed to create instance!");

    hasGflwRequiredInstanceExtensions();
}

/// @brief sets physicalDevice to a usable VkPhysicalDevice
///
/// 1. Iterates through all GPUs and checks `isDeviceSuitable()`, then sets physicalDevice to a usable VkPhysicalDevice
///
/// 2. Prints `Device count:` and `physical device:`
void Device::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::cout << "Device count: " << deviceCount << std::endl;
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU!");

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << "physical device: " << properties.deviceName << std::endl;
}

/// @brief Creates a VkDevice aka handle to the GPU
///
/// 1. Call `findQueueFamilies()` to get the queue family with supported graphics && present
///
/// 2. Create a `VkDeviceQueueCreateInfo` struct containing the fetched queues in (1.)
///
/// 3. Create a `VkPhysicalDeviceFeatures` struct containing the used features
///
/// 4. Create a `VkDeviceCreateInfo` struct that has the previous 2 structs
///
/// 5. Call `vkCreateDevice()` to make device using struct in (5.)
///
/// 6. Call `vkGetDeviceQueue()` to get the `VkQueue` handles for both graphics and present queue families
void Device::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // collapses all same queues into a single one, capable of both
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();

    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_))
        throw std::runtime_error("failed to create logical device!");

    vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
}

/// @brief Allocates a block of memory that is reserved for command buffers aka `VkCommandPool commandPool`
void Device::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;                // graphics family queue
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |            // command buffers from this pool will be short-lived, re-recorded frequently
                                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // allows individual command buffers allocated from this pool to be reset/re-recorded independently

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool))
        throw std::runtime_error("failed to create command pool!");
}

void Device::createSurface() { window.createWindowSurface(instance, &surface_); }

/// @brief
/// 1. Call `findQueueFamilies()` to get the conforming graphics and present queue families
///
/// 2. Checks if required extensions in `deviceExtensions` are supported by the GPU with `vkEnumerateDeviceExtensionProperties()`
///
/// 3. Call `querySwapChainSupport()` to get pixel format + colorspace and presentMode
///
/// 4. Call `vkGetPhysicalDeviceFeatures()` to get a list of features supported
///
/// 5. && check: graphicsFamilyHasValue, presentFamilyHasValue, extensionsSupported, swapChainAdequate and used extensions from (3.)
/// @param device the VkPhysicalDevice in question
/// @return Are all the flags in (5.) valid?
bool Device::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    // get supported extensions for GPU device
    bool extensionsSupported;
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        extensionsSupported = requiredExtensions.empty();
    }

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate                        = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.graphicsFamilyHasValue &&
           indices.presentFamilyHasValue &&
           extensionsSupported &&
           swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

/// @brief Specifies flags for debugging and populates the debug logic lambda
/// @param createInfo `VkDebugUtilsMessengerCreateInfoEXT` info to create `VkDebugUtilsMessengerEXT`
void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo       = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pUserData = nullptr;  // Optional

    createInfo.pfnUserCallback = static_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(
        [](
            VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT             messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*                                       pUserData) -> VkBool32
        {
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
                std::cerr << "[VERBOSE INFO] " << pCallbackData->pMessage << std::endl;
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                std::cerr << "[INFO] " << pCallbackData->pMessage << std::endl;
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                std::cerr << "[WARNING] " << pCallbackData->pMessage << std::endl;
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                std::cerr << "[ERROR] " << pCallbackData->pMessage << std::endl;
            else
                std::cerr << "[MESSAGE] " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        });
}

/// @brief creates a `VkDebugUtilsMessengerEXT` object
void Device::setupDebugMessenger()
{
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr)
        throw std::runtime_error("failed to set up debug messenger! Extension is not present.");

    if (func(instance, &createInfo, nullptr, &debugMessenger))
        throw std::runtime_error("failed to set up debug messenger!");
}

/// @brief Checks if the error handling layers are present
/// @return Are all layers are present?
bool Device::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

/// @brief gets the extensions from GLFW (VK_KHR_surface and VK_KHR_win32_surface)
/// @return vector of C strings that are the names of extensions needed
std::vector<const char*> Device::getRequiredExtensions()
{
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

/// @brief
/// Print "Available extensions:" and "Required extensions:"
///
/// Throw if Required is not in Available
void Device::hasGflwRequiredInstanceExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "Available extensions:" << std::endl;
    std::unordered_set<std::string> available;
    for (const auto& extension : extensions)
    {
        std::cout << "\t" << extension.extensionName << std::endl;
        available.insert(extension.extensionName);
    }

    std::cout << "Required extensions:" << std::endl;
    auto requiredExtensions = getRequiredExtensions();
    for (const auto& required : requiredExtensions)
    {
        std::cout << "\t" << required << std::endl;
        if (available.find(required) == available.end())
            throw std::runtime_error("Missing required glfw extension");
    }
}

/// @brief Get a queue family with supported graphics && present
/// @param device the VkPhysicalDevice in question
/// @return A `QueueFamilyIndices` struct containing:
///
/// - uint32_t graphicsFamily;
///
/// - uint32_t presentFamily;
///
/// - bool     graphicsFamilyHasValue = false;
///
/// - bool     presentFamilyHasValue  = false;
QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily         = i;  // the graphics computation queue
            indices.graphicsFamilyHasValue = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport)
        {
            indices.presentFamily         = i;  // the display inside window queue
            indices.presentFamilyHasValue = true;
        }
        if (indices.graphicsFamilyHasValue && indices.presentFamilyHasValue)
            break;

        i++;
    }

    return indices;
}

/// @brief
/// 1. Gets pixel format + colorspace (e.g., `VK_FORMAT_B8G8R8A8_SRGB` + `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`).
///
/// 2. Gets presentMode (e.g., `VK_PRESENT_MODE_FIFO_KHR` [vsync'd], `VK_PRESENT_MODE_MAILBOX_KHR` [triple-buffering, low latency], `VK_PRESENT_MODE_IMMEDIATE_KHR` [no sync, possible tearing]).
/// @param device the VkPhysicalDevice in question
/// @return A `SwapChainSupportDetails` struct containing:
///
/// - `capabilities` (VkSurfaceCapabilitiesKHR)
///
/// - `formats` (std::vector<VkSurfaceFormatKHR>)
///
/// - `presentModes` (std::vector<VkPresentModeKHR>)
SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    // supported pixel format + colorspace e.g. VK_FORMAT_B8G8R8A8_SRGB + VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }

    // e.g. VK_PRESENT_MODE_FIFO_KHR (vsync'd), VK_PRESENT_MODE_MAILBOX_KHR (triple-buffering, low latency), VK_PRESENT_MODE_IMMEDIATE_KHR (no sync, possible tearing)
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
    }
    return details;
}

/// @brief Goes throught all the `availableFormats` and picks the one that is `VK_FORMAT_B8G8R8A8_SRGB` && `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`
///
/// Otherwise default to the first format available
/// @param availableFormats list of { VkFormat and VkColorSpaceKHR }
/// @return Format that is available if not the one desired
VkSurfaceFormatKHR Device::findSwapSurfaceFormat()
{
    SwapChainSupportDetails supportDetails = findSwapChainSupport();

    for (const auto& availableFormat : supportDetails.formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    return supportDetails.formats[0];
}


/// @brief Gets the supported image format from the required image features.
/// @param candidates list of desired image formats in order of preferance e.g. `{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}`
/// @param tiling `VK_IMAGE_TILING_OPTIMAL` for optimal GPU packing, `VK_IMAGE_TILING_LINEAR` for CPU access
/// @param features image format features that will be used e.g. `VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT`, `VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT` . . .
/// @return image format enum e.g. `VK_FORMAT_R8G8B8_UINT`, `VK_FORMAT_R32G32B32_SFLOAT` . . .
VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }
    throw std::runtime_error("failed to find supported format!");
}

/// @brief Find a GPU memory manager type that supports the properties you provide
/// @param typeFilter vulkan queried memory type indices this specific resource is allowed to use, varies with different GPUs
/// @param properties how to use the memory:
///
/// - `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`, fast GPU-only memory
///
/// - `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`, CPU-mappable
///
/// - `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`, CPU writes are auto flushed to GPU, no manual flush needed
/// @return index into `VkMemoryType` containing:
///
/// - `VkMemoryPropertyFlags`    propertyFlags;
///
/// - `uint32_t`                 heapIndex;
uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

/// @brief Allocates a buffer on the GPU and connects your `VkBuffer` to that memory
/// @param size the size in bytes of the buffer to be created
/// @param bufferUsage is a bitmask of `VkBufferUsageFlagBits` specifying allowed usages of the buffer e.g. `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`
/// @param allocUsage how
/// @param properties how to use the memory:
///
/// - `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`, fast GPU-only memory
///
/// - `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`, CPU-mappable
///
/// - `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`, CPU writes are auto flushed to GPU, no manual flush needed
/// @param buffer empty `VkBuffer` handle
/// @param bufferMemory empty `VkDeviceMemory` handle to the GPU memory
void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateInfo allocUsage, AllocatedBuffer& buffer)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = size;
    bufferInfo.usage       = bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // only one queue family can access this buffer at a time

    if (vmaCreateBuffer(vmallocator, &bufferInfo, &allocUsage, &buffer.buffer, &buffer.allocation, nullptr))
        throw std::runtime_error("failed to create buffer!");
}

/// @brief Allocates the command buffer using commandPool and starts recording using `vkBeginCommandBuffer()`
/// @return Handle to the command buffer recording
VkCommandBuffer Device::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer))
        throw std::runtime_error("failed to allocate command buffer");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

/// @brief Stops the recording of command buffer and submits via `vkQueueSubmit()` and waits till it finishes executing
/// @param commandBuffer handle to the command buffer recording, got by `beginSingleTimeCommands()`
void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    if (vkEndCommandBuffer(commandBuffer))
        throw std::runtime_error("failed to end command buffer");

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence;
    if (vkCreateFence(device_, &fenceInfo, nullptr, &fence))
        throw std::runtime_error("failed to create fence");

    if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, fence))  // signal THIS fence when THIS submission finishes
        throw std::runtime_error("failed to submit queue");

    if (vkWaitForFences(device_, 1, &fence, VK_TRUE, UINT64_MAX))  // CPU blocks only until THIS work is done
        throw std::runtime_error("failed to wait for fence");

    vkDestroyFence(device_, fence, nullptr);
    vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}

/// @brief Copies data from one buffer to another
/// @param srcBuffer source buffer
/// @param dstBuffer destination buffer
/// @param size number of bytes to copy
void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size      = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

/// @brief Copies buffer to image
/// @param buffer source buffer
/// @param image destination image
/// @param width image width in texels
/// @param height image height in texels
/// @param layerCount number of image layers
void Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;  // 0 = start from the begining
    region.bufferRowLength   = 0;  // 0 = tightly packed, no padding
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = layerCount;

    // in texels
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    // TRANSFER_DST_OPTIMAL     -> optimal for copying
    // SHADER_READ_ONLY_OPTIMAL -> optimal for sampling
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    endSingleTimeCommands(commandBuffer);
}

void Device::transitionImageLayout(VkImage image, VkImageSubresourceRange range, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier imageBarrier_toTransfer{};
    imageBarrier_toTransfer.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier_toTransfer.oldLayout        = oldLayout;
    imageBarrier_toTransfer.newLayout        = newLayout;
    imageBarrier_toTransfer.image            = image;
    imageBarrier_toTransfer.subresourceRange = range;
    imageBarrier_toTransfer.srcAccessMask    = 0;
    imageBarrier_toTransfer.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;

    // barrier the image into the transfer-receive layout
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

    endSingleTimeCommands(commandBuffer);
}

/// @brief Calls VMA to create, allocate and bind the image
/// @param imageInfo decription of the image
/// @param memoryUsage usualy VMA_MEMORY_USAGE_CPU_TO_GPU
/// @param image empty image
void Device::createImageWithInfo(const VkImageCreateInfo& imageInfo, VmaAllocationCreateInfo allocInfo, AllocatedImage& image)
{
    if (vmaCreateImage(vmallocator, &imageInfo, &allocInfo, &image.image, &image.allocation, nullptr))
        throw std::runtime_error("failed to create image!");
}
