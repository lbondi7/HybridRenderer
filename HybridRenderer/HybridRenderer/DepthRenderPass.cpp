#include "DepthRenderPass.h"

#include "Initilizers.h"
#include "Utility.h"

#include <array>

DepthRenderPass::~DepthRenderPass()
{
}

void DepthRenderPass::Create(DeviceContext* _devices, SwapChain* _swapChain)
{
    devices = _devices;
    swapChain = _swapChain;

    Init();
}

void DepthRenderPass::Init()
{
    VkAttachmentDescription attachmentDescription = Initialisers::attachmentDescription(Utility::findDepthFormat(devices->physicalDevice), VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    VkAttachmentReference depthReference = Initialisers::attachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkSubpassDescription subpass = Initialisers::subpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, &depthReference);

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
