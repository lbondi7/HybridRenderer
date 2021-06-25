#pragma once

#include "Constants.h"
#include "SwapChain.h"

class FrameBuffer
{
public:
	FrameBuffer() = default;
	~FrameBuffer();

	std::vector<VkFramebuffer> vkFrameBuffers;


	void Create(Device* _devices, SwapChain* _swapChain, VkRenderPass _vkRenderPass);

	void Create(Device* _devices, VkRenderPass _vkRenderPass);

	void Init();

	void createFramebuffers();

	void createFramebuffer(const std::vector<VkImageView>& attachments, VkExtent2D extent);

	void prepareOffscreenFramebuffer();

	void Destroy();

private:
	Device* devices;
	SwapChain* swapChain;
	VkRenderPass vkRenderPass;
};

