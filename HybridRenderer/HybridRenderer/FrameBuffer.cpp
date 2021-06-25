#include "FrameBuffer.h"

#include "Initilizers.h"
#include "Utility.h"

#include <array>
#include <stdexcept>

FrameBuffer::~FrameBuffer()
{
    devices = nullptr;
    swapChain = nullptr;
    vkRenderPass = nullptr;
}

void FrameBuffer::Create(Device* _devices, SwapChain* _swapChain, VkRenderPass _vkRenderPass, OffscreenPass& offscreenPass)
{
    devices = _devices;
    swapChain = _swapChain;
    vkRenderPass = _vkRenderPass;
    Init(offscreenPass);
}

void FrameBuffer::Create(Device* _devices, VkRenderPass _vkRenderPass)
{
	devices = _devices;
	vkRenderPass = _vkRenderPass;
}

void FrameBuffer::Init(OffscreenPass& offscreenPass)
{
    createFramebuffers();
	prepareOffscreenFramebuffer(offscreenPass);
}

void FrameBuffer::createFramebuffers() {
   vkFrameBuffers.resize(swapChain->imageCount);

    for (size_t i = 0; i < swapChain->imageCount; i++) {
        std::array<VkImageView, 2> attachments = {
            swapChain->images[i].imageView,
            swapChain->depthImage.imageView
        };

        VkFramebufferCreateInfo framebufferInfo = Initialisers::framebufferCreateInfo(vkRenderPass, attachments.data(), static_cast<uint32_t>(attachments.size()), swapChain->extent, 1);

        if (vkCreateFramebuffer(devices->logicalDevice, &framebufferInfo, nullptr, &vkFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void FrameBuffer::createFramebuffer(const std::vector<VkImageView>& attachments, VkExtent2D extent) {

	vkFrameBuffers.emplace_back(VkFramebuffer());

	size_t idx = vkFrameBuffers.size() - 1;

	VkFramebufferCreateInfo framebufferInfo = Initialisers::framebufferCreateInfo(vkRenderPass, attachments.data(), static_cast<uint32_t>(attachments.size()), extent);

	if (vkCreateFramebuffer(devices->logicalDevice, &framebufferInfo, nullptr, &vkFrameBuffers[idx]) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}

VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)
{
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

	if (tiling == VK_IMAGE_TILING_OPTIMAL)
		return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	if (tiling == VK_IMAGE_TILING_LINEAR)
		return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	return false;
}

void FrameBuffer::prepareOffscreenFramebuffer(OffscreenPass& offscreenPass)
{

	// For shadow mapping we only need a depth attachment
	VkImageCreateInfo image{};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.extent.width = offscreenPass.width;
	image.extent.height = offscreenPass.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.format = Utility::findDepthFormat(devices->physicalDevice);																// Depth stencil attachment
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;		// We will sample directly from the depth attachment for the shadow mapping

	if (vkCreateImage(devices->logicalDevice, &image, nullptr, &offscreenPass.depth.image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(devices->logicalDevice, offscreenPass.depth.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = Utility::findMemoryType(memReqs.memoryTypeBits, devices->physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(devices->logicalDevice, &memAlloc, nullptr, &offscreenPass.depth.mem) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(devices->logicalDevice, offscreenPass.depth.image, offscreenPass.depth.mem, 0);

	VkImageViewCreateInfo depthStencilView{};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = Utility::findDepthFormat(devices->physicalDevice);
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = offscreenPass.depth.image;

	if (vkCreateImageView(devices->logicalDevice, &depthStencilView, nullptr, &offscreenPass.depth.view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	// Create sampler to sample from to depth attachment
	// Used to sample in the fragment shader for shadowed rendering
	VkFilter shadowmap_filter = formatIsFilterable(devices->physicalDevice, Utility::findDepthFormat(devices->physicalDevice), VK_IMAGE_TILING_OPTIMAL) ?
		VK_FILTER_LINEAR :
		VK_FILTER_NEAREST;
	VkSamplerCreateInfo sampler{};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = shadowmap_filter;
	sampler.minFilter = shadowmap_filter;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	if (vkCreateSampler(devices->logicalDevice, &sampler, nullptr, &offscreenPass.depthSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	offscreenPass.frameBuffers.resize(swapChain->imageCount);
	for (size_t i = 0; i < swapChain->imageCount; i++) {
		// Create frame buffer
		VkFramebufferCreateInfo fbufCreateInfo{};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.renderPass = offscreenPass.renderPass;
		fbufCreateInfo.attachmentCount = 1;
		fbufCreateInfo.pAttachments = &offscreenPass.depth.view;
		fbufCreateInfo.width = offscreenPass.width;
		fbufCreateInfo.height = offscreenPass.height;
		fbufCreateInfo.layers = 1;

		if (vkCreateFramebuffer(devices->logicalDevice, &fbufCreateInfo, nullptr, &offscreenPass.frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void FrameBuffer::Destroy() {
	for (auto framebuffer : vkFrameBuffers) {
		vkDestroyFramebuffer(devices->logicalDevice, framebuffer, nullptr);
	}
}