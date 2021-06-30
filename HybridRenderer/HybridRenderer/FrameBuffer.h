#pragma once

#include "Constants.h"
#include "SwapChain.h"

class FrameBuffer
{
public:
	FrameBuffer() = default;
	~FrameBuffer();

	std::vector<VkFramebuffer> vkFrameBuffers;


	void Create(DeviceContext* _devices, SwapChain* _swapChain, VkRenderPass _vkRenderPass);

	void Create(DeviceContext* _devices, VkRenderPass _vkRenderPass);

	void Init(VkRenderPass _vkRenderPass);

	void createFramebuffers();

	void createFramebuffer(const std::vector<VkImageView>& attachments, VkExtent2D extent);

	void Destroy();

private:
	DeviceContext* devices;
	SwapChain* swapChain;
	VkRenderPass vkRenderPass;
};

