#pragma once

#include "Constants.h"
#include "DepthRenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "CubemapTexture.h"

class ShadowMap
{
public:

	ShadowMap() = default;
	~ShadowMap();

	void update(bool& resize);
	void Create(DeviceContext* _devices, SwapChain* _swapChain);

	void Init(DescriptorSetManager* dsManager, const PipelineInfo& pipelineInfo);

	void reinit(bool complete = true);

	void Destroy(bool complete = true);

	uint32_t width;
	uint32_t height;
	int resolution = 2048;

	FrameBuffer frameBuffer;
	Pipeline pipeline;

	TextureSampler depthTexture;
	//CubemapTexture depthCubemap;
	RenderPass renderPass;
	DeviceContext* devices = nullptr;
	SwapChain* swapChain = nullptr;

	std::vector<VkDescriptorSet> descriptorSets;

	DescriptorSetManager* dsm;

};

