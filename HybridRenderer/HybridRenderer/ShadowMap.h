#pragma once

#include "Constants.h"
#include "DepthRenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "CubemapTexture.h"
#include "Descriptor.h"
#include "ImGUIWidgets.h"

class ShadowMap
{
public:

	ShadowMap() = default;
	~ShadowMap();

	void Create(DeviceContext* _devices, SwapChain* _swapChain);

	void Initialise(const PipelineInfo& pipelineInfo);

	void Reinitialise(bool complete = true);

	void Destroy(bool complete = true);

	bool Update();

	uint32_t width;
	uint32_t height;
	uint32_t resolution = 2048;

	FrameBuffer frameBuffer;
	Pipeline pipeline;

	TextureSampler depthTexture;
	//CubemapTexture depthCubemap;
	RenderPass renderPass;

	Descriptor descriptor;

	ImGUIWidget widget;
private:

	void Initialise();

	DeviceContext* devices = nullptr;
	SwapChain* swapChain = nullptr;

};

