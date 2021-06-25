#pragma once

#include "Constants.h"

#include "SwapChain.h"

class RenderPass
{
public:

	RenderPass() = default;
	~RenderPass();

	VkRenderPass vkRenderPass;

	void Create(Device* _devices, SwapChain* _swapChain, OffscreenPass& offscreenPass);


	virtual void Create(Device* _devices, SwapChain* _swapChain);

	void Init(OffscreenPass& offscreenPass);

	virtual void Init() = 0;

	void Destroy();

	VkPipelineBindPoint pipelineBindPoint;

protected:

	SwapChain* swapChain = nullptr;
	Device* devices = nullptr;

	//void createRenderPass();

	//void prepareOffscreenRenderpass(OffscreenPass& offscreenPass);

};

