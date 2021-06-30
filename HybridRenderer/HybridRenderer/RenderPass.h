#pragma once

#include "Constants.h"

#include "SwapChain.h"

class RenderPass
{
public:

	RenderPass() = default;
	~RenderPass();

	VkRenderPass vkRenderPass;

	//void Create(Device* _devices, SwapChain* _swapChain,);

	void Begin(VkCommandBuffer cmdBuffer, VkFramebuffer frameBuffer, VkExtent2D extent, const VkClearValue* pClearValues, uint32_t clearValueCount = 1);

	void End(VkCommandBuffer cmdBuffer);

	virtual void Create(DeviceContext* _devices, SwapChain* _swapChain);

	//void Init();

	virtual void Init() = 0;

	void Destroy();

	VkPipelineBindPoint pipelineBindPoint;

protected:

	SwapChain* swapChain = nullptr;
	DeviceContext* devices = nullptr;

	//void createRenderPass();

	//void prepareOffscreenRenderpass(OffscreenPass& offscreenPass);

};

