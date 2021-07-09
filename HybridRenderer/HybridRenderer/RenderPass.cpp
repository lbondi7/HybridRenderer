#include "RenderPass.h"

#include "Utility.h"

#include "Initilizers.h"

#include <thread>
#include <array>
#include <stdexcept>

RenderPass::~RenderPass()
{
    devices = nullptr;
    swapChain = nullptr;
}

//void RenderPass::Create(Device* _devices, SwapChain* _swapChain, OffscreenPass& offscreenPass)
//{
//    devices = _devices;
//    swapChain = _swapChain;
//
//    Init(offscreenPass);
//}

void RenderPass::Begin(VkCommandBuffer cmdBuffer, VkFramebuffer frameBuffer, VkExtent2D extent, const VkClearValue* pClearValues, uint32_t clearValueCount)
{

    VkRenderPassBeginInfo renderPassBeginInfo = Initialisers::renderPassBeginInfo(vkRenderPass, frameBuffer,
        extent, clearValueCount, pClearValues);

    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::End(VkCommandBuffer cmdBuffer)
{

    vkCmdEndRenderPass(cmdBuffer);
}

void RenderPass::Create(DeviceContext* _devices, RenderPassInfo& _info)
{
    devices = _devices;
    info = _info;

    Init();
}


void RenderPass::Init()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector< VkAttachmentReference> references;

    VkSubpassDescription subpass = Initialisers::subpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS);
    uint32_t idx = 0;
    for (auto& attachment : info.attachments)
    {
        attachments.emplace_back(
            Initialisers::attachmentDescription(attachment.format, 
                VK_SAMPLE_COUNT_1_BIT,  attachment.loadOp, VK_ATTACHMENT_STORE_OP_STORE, 
                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                attachment.initialLayout, attachment.finalLayout));

        if (attachment.type == AttachmentType::COLOUR) {

            VkAttachmentReference reference = Initialisers::attachmentReference(idx, attachment.referenceLayout);
            //references.emplace_back(Initialisers::attachmentReference(idx, attachment.referenceLayout));
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &reference;
        }
        else if (attachment.type == AttachmentType::DEPTH) {
            VkAttachmentReference reference = Initialisers::attachmentReference(idx, attachment.referenceLayout);
            subpass.pDepthStencilAttachment = &reference;
        }

        ++idx;
    }

    VkRenderPassCreateInfo renderPassInfo = Initialisers::renderPassCreateInfo(static_cast<uint32_t>(attachments.size()), attachments.data(),
        1, &subpass, static_cast<uint32_t>(info.dependencies.size()), info.dependencies.data());

    if (vkCreateRenderPass(devices->logicalDevice, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPass::Destroy() {
    vkDestroyRenderPass(devices->logicalDevice, vkRenderPass, nullptr);
}
