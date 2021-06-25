#include "DepthRenderPass.h"

#include "Initilizers.h"
#include "Utility.h"

#include <array>

DepthRenderPass::~DepthRenderPass()
{
}

void DepthRenderPass::Create(Device* _devices, SwapChain* _swapChain)
{
    RenderPass::Create(_devices, _swapChain);

    Init();
}

void DepthRenderPass::Init()
{
    VkAttachmentDescription attachmentDescription = Initialisers::attachmentDescription(Utility::findDepthFormat(devices->physicalDevice), VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    //attachmentDescription.format = Utility::findDepthFormat(devices->physicalDevice);
    //attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    //attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							  // Clear depth at beginning of the render pass
    //attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						  // We will read from depth, so it's important to store the depth attachment results
    //attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					  // We don't care about initial layout of the attachment
    //attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  // Attachment will be transitioned to shader read at render pass end

    VkAttachmentReference depthReference = Initialisers::attachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    //depthReference.attachment = 0;
    //depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

    VkSubpassDescription subpass = Initialisers::subpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, &depthReference);
    //subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //subpass.colorAttachmentCount = 0;													// No color attachments
    //subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies{
    Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),

        Initialisers::subpassDependency(0, VK_SUBPASS_EXTERNAL,
    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)

    };

    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = Initialisers::renderPassCreateInfo(1, &attachmentDescription, 1, &subpass, static_cast<uint32_t>(dependencies.size()), dependencies.data());

    if (vkCreateRenderPass(devices->logicalDevice, &renderPassCreateInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}
