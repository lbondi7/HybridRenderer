#pragma once

#include "Constants.h"
#include "SwapChain.h"

struct FrameData {
	VkFramebuffer vkFrameBuffer;
	VkExtent2D extent;
};

class FrameBuffer
{
public:
	FrameBuffer() = default;
	~FrameBuffer();

	void Create(DeviceContext* _devices, SwapChain* _swapChain, VkRenderPass _vkRenderPass);

	void Create(DeviceContext* _devices, VkRenderPass _vkRenderPass);

	void Init(VkRenderPass _vkRenderPass);

	void createFramebuffer(const std::vector<VkImageView>& attachments, VkExtent2D extent);

	void Destroy();

	std::vector<FrameData> frames;

private:
	DeviceContext* devices;
	SwapChain* swapChain;
	VkRenderPass vkRenderPass;
};

