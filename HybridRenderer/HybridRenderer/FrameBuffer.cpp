#include "FrameBuffer.h"

#include "Initilizers.h"
#include "Utility.h"

#include <array>
#include <stdexcept>

FrameBuffer::~FrameBuffer()
{
    devices = nullptr;
    swapChain = nullptr;
    vkRenderPass = VK_NULL_HANDLE;
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
	frames.reserve(devices->imageCount);
}


void FrameBuffer::createFramebuffer(const std::vector<VkImageView>& attachments, VkExtent2D extent) {

	frames.emplace_back(FrameData());

	size_t idx = frames.size() - 1;

	frames[idx].extent = extent;

	VkFramebufferCreateInfo framebufferInfo = Initialisers::framebufferCreateInfo(vkRenderPass, attachments.data(), static_cast<uint32_t>(attachments.size()), extent);

	if (vkCreateFramebuffer(devices->logicalDevice, &framebufferInfo, nullptr, &frames[idx].vkFrameBuffer) != VK_SUCCESS) {
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
	for (auto frame: frames) {
		vkDestroyFramebuffer(devices->logicalDevice, frame.vkFrameBuffer, nullptr);
	}
	frames.clear();
}