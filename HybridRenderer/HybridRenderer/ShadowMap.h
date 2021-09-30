#pragma once

#include "Constants.h"
#include "DepthRenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "CubemapTexture.h"
#include "Descriptor.h"
#include "ImGUIWidgets.h"
#include "Buffer.h"

struct ShadowUBO {
	int shadowMap = 2;
	int vertexRotate = 0;
	int texRotate = 0;
	int baryRotate = 0;
};

class ShadowMap
{
public:

	ShadowMap() = default;
	~ShadowMap();

	void Create(DeviceContext* _devices);

	void Initialise(const PipelineInfo& pipelineInfo);

	void Reinitialise(bool complete = true);

	void Destroy(bool complete = true);

	bool Update(uint32_t imageIndex);

	uint32_t width;
	uint32_t height;
	uint32_t resolution = 2048;

	FrameBuffer frameBuffer;

	Pipeline pipeline;

	TextureSampler depthTexture;

	RenderPass renderPass;

	Descriptor descriptor;

	ImGUIWidget widget;

	ShadowUBO shadowUBO;

	std::vector<Buffer> buffers;
private:

	void Initialise();

	DeviceContext* devices = nullptr;
	SwapChain* swapChain = nullptr;

};

