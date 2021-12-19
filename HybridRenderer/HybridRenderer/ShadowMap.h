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
	alignas(16) glm::ivec4 shadow;
	alignas(16) glm::vec4 blocker;
};

class ShadowMap
{
public:

	ShadowMap() = default;
	~ShadowMap();

	void Create(DeviceContext* _devices, int descriptorSet, const std::string& windowName = "Shadow Map");

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
	int descriptorSet = 0;
	std::string windowName = "Shadow Map";

	DeviceContext* devices = nullptr;
	SwapChain* swapChain = nullptr;

};

