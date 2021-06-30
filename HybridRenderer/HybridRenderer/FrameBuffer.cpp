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

void FrameBuffer::Create(DeviceContext* _devices, SwapChain* _swapChain, VkRenderPass _vkRenderPass)
{
    devices = _devices;
    swapChain = _swapChain;
    Init(_vkRenderPass);
}

void FrameBuffer::Create(DeviceContext* _devices, VkRenderPass _vkRenderPass)
{
	devices = _devices;
	Init(_vkRenderPass);
}

void FrameBuffer::Init(VkRenderPass _vkRenderPass)
{
	vkRenderPass = _vkRenderPass;
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

void FrameBuffer::Destroy() {
	for (auto framebuffer : vkFrameBuffers) {
		vkDestroyFramebuffer(devices->logicalDevice, framebuffer, nullptr);
	}
	vkFrameBuffers.clear();
}