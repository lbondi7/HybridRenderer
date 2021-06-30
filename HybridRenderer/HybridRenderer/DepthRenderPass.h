#pragma once
#include "RenderPass.h"

class DepthRenderPass : public RenderPass
{
public:
	~DepthRenderPass();

	void Create(DeviceContext* _devices, SwapChain* _swapChain) override;


	void Init() override;

};

