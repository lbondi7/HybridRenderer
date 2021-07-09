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

	virtual void Create(DeviceContext* _devices, RenderPassInfo& _info);

	//void Init();

	virtual void Init();

	void Destroy();

	VkPipelineBindPoint pipelineBindPoint;

	RenderPassInfo info;

protected:

	DeviceContext* devices = nullptr;
	SwapChain* swapChain;

	//void createRenderPass();

	//void prepareOffscreenRenderpass(OffscreenPass& offscreenPass);

};

