#pragma once

#include "Constants.h"
#include "DepthRenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"

class ShadowMap
{
public:

	ShadowMap() = default;
	~ShadowMap();


	void Create(DeviceContext* _devices, SwapChain* _swapChain);

	void Init(DescriptorSetManager* dsManager, const PipelineInfo& pipelineInfo);

	void Destroy();

	uint32_t width = 2048;
	uint32_t height = 2048;

	FrameBuffer frameBuffer;
	Pipeline pipeline;

	Texture depthTexture;
	DepthRenderPass renderPass;
	DeviceContext* devices = nullptr;
	SwapChain* swapChain = nullptr;
};

