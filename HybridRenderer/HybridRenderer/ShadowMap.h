#pragma once

#include "Constants.h"
#include "DepthRenderPass.h"
#include "FrameBuffer.h"

class ShadowMap
{
public:

	ShadowMap() = default;
	~ShadowMap();


	void Create(Device* _devices, SwapChain* _swapChain);

	void Init();


	uint32_t width = 2048;
	uint32_t height = 2048;

	FrameBuffer frameBuffer;

	Texture depthTexture;
	DepthRenderPass renderPass;
	Device* devices = nullptr;
	SwapChain* swapChain = nullptr;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline vkPipeline;
};

