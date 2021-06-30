#include "ColourRenderPass.h"

#include "Initilizers.h"
#include "Utility.h"

#include <array>

ColourRenderPass::~ColourRenderPass()
{
}

void ColourRenderPass::Create(DeviceContext* _devices, SwapChain* _swapChain)
{
    RenderPass::Create(_devices, _swapChain);

    Init();
}

void ColourRenderPass::Init()
{
    VkAttachmentDescription colorAttachment = Initialisers::attachmentDescription(swapChain->imageFormat, VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VkAttachmentDescription depthAttachment = Initialisers::attachmentDescription(Utility::findDepthFormat(devices->physicalDevice), VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkAttachmentReference colorAttachmentRef = Initialisers::attachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkAttachmentReference depthAttachmentRef = Initialisers::attachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkSubpassDescription subpass = Initialisers::subpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &colorAttachmentRef, &depthAttachmentRef);

    VkSubpassDependency dependency = Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = Initialisers::renderPassCreateInfo(static_cast<uint32_t>(attachments.size()), attachments.data(),
        1, &subpass, 1, &dependency);

    if (vkCreateRenderPass(devices->logicalDevice, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

}
