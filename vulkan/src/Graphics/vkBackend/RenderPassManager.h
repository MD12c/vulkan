#ifndef RENDER_PASS_MANAGER_H
#define RENDER_PASS_MANAGER_H

#include <vulkan/vulkan_core.h>

#include "Device.h"

class SwapChain;

class RenderPassManager
{
private:
    Device&      device;
    VkRenderPass mainRenderPass   = VK_NULL_HANDLE;
    VkRenderPass shadowRenderPass = VK_NULL_HANDLE;

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
            for (auto& attachment : attachments)
                attachmentDescription.push_back(attachment.attachmentDescription);
            
            return attachmentDescription;
        }
    };

    RenderPassManager(Device& device);
    ~RenderPassManager();

    void createRenderPass(VkRenderPass& renderPass, RenderPassConfigInfo& renderPassConfigInfo);

    void recreateMainRenderPassLayout();
    void recreateShadowRenderPassLayout();

    VkRenderPass getMainRenderPass() const { return mainRenderPass; }
    VkRenderPass getShadowRenderPass() const { return shadowRenderPass; }
};

#endif