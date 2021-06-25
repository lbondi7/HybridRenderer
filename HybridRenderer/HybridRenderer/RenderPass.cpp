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

void RenderPass::Create(Device* _devices, SwapChain* _swapChain, OffscreenPass& offscreenPass)
{
    devices = _devices;
    swapChain = _swapChain;

    Init(offscreenPass);
}

void RenderPass::Create(Device* _devices, SwapChain* _swapChain)
{
    devices = _devices;
    swapChain = _swapChain;
}

void RenderPass::Init(OffscreenPass& offscreenPass) {
    //createRenderPass();

    //prepareOffscreenRenderpass(offscreenPass);
}

//
//void RenderPass::createRenderPass() {
//    VkAttachmentDescription colorAttachment = Initialisers::attachmentDescription(swapChain->imageFormat, VK_SAMPLE_COUNT_1_BIT,
//        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
//
//    VkAttachmentDescription depthAttachment = Initialisers::attachmentDescription(Utility::findDepthFormat(devices->physicalDevice), VK_SAMPLE_COUNT_1_BIT, 
//        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, 
//        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//
//    VkAttachmentReference colorAttachmentRef = Initialisers::attachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//
//    VkAttachmentReference depthAttachmentRef = Initialisers::attachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//
//    VkSubpassDescription subpass = Initialisers::subpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &colorAttachmentRef, &depthAttachmentRef);
//
//    VkSubpassDependency dependency = Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0, 
//        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
//        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
//        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
//
//    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
//    VkRenderPassCreateInfo renderPassInfo = Initialisers::renderPassCreateInfo(static_cast<uint32_t>(attachments.size()), attachments.data(), 
//        1, &subpass, 1, &dependency);
//
//    if (vkCreateRenderPass(devices->logicalDevice, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create render pass!");
//    }
//}
//
//void RenderPass::prepareOffscreenRenderpass(OffscreenPass& offscreenPass)
//{
//    VkAttachmentDescription attachmentDescription = Initialisers::attachmentDescription(Utility::findDepthFormat(devices->physicalDevice), VK_SAMPLE_COUNT_1_BIT, 
//        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, 
//        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, 
//        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
//
//    //attachmentDescription.format = Utility::findDepthFormat(devices->physicalDevice);
//    //attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
//    //attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							  // Clear depth at beginning of the render pass
//    //attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						  // We will read from depth, so it's important to store the depth attachment results
//    //attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    //attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    //attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					  // We don't care about initial layout of the attachment
//    //attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  // Attachment will be transitioned to shader read at render pass end
//
//    VkAttachmentReference depthReference = Initialisers::attachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//    //depthReference.attachment = 0;
//    //depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass
//
//    VkSubpassDescription subpass = Initialisers::subpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, &depthReference);
//    //subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    //subpass.colorAttachmentCount = 0;													// No color attachments
//    //subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment
//
//    // Use subpass dependencies for layout transitions
//    std::array<VkSubpassDependency, 2> dependencies{
//    Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0, 
//    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
//    VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
//
//        Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
//    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)
//    
//    };
//
//    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//    VkRenderPassCreateInfo renderPassCreateInfo = Initialisers::renderPassCreateInfo(1, &attachmentDescription, 1, &subpass, static_cast<uint32_t>(dependencies.size()), dependencies.data());
//
//    if (vkCreateRenderPass(devices->logicalDevice, &renderPassCreateInfo, nullptr, &offscreenPass.renderPass) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create render pass!");
//    }
//
//}

void RenderPass::Destroy() {
    vkDestroyRenderPass(devices->logicalDevice, vkRenderPass, nullptr);
}
