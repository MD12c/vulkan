#ifndef APP_CLASS_H
#define APP_CLASS_H

#include <memory>
#include <vector>

#include "Graphics/Window.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Device.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Model.h"

class App
{
private:
    int         width        = 1920;
    int         height       = 1080;
    const char* name         = "VK Tutorial";
    float       windowRGB[3] = {
        0.7f, 0.7f, 0.7f
    };

    Window                       window;
    Device                       device;
    std::unique_ptr<SwapChain>   swapchain;
    std::unique_ptr<Pipeline>    pipeline;
    VkPipelineLayout             pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffers;
    std::unique_ptr<Model>       model;

    void loadModels();
    void createPipeline();
    void createPipelineLayout();
    void createCommandBuffers();
    void freeCommandBuffers();
    void drawFrame();
    void recreateSwapchain();
    void recordCommandBuffer(int imageIndex);

public:
    App();
    ~App();
    App(const App&)            = delete;
    App& operator=(const App&) = delete;

    void run();
};

#endif