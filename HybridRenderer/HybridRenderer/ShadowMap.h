#pragma once

#include "Constants.h"
#include "DepthRenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "Texture.h"
#include "Descriptor.h"
#include "ImGUIWidgets.h"
#include "Buffer.h"

struct ShadowUBO {
	int shadowMap = 2;
	int confirmIntersection = 0;
	int terminateRay = 0;
	float alphaThreshold = 0.02;
	float bias = 0.13f;
	float blockerScale = 1.0f;
	int pcfFilterSize = 1;
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

