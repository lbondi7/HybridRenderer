#pragma once
#include "RenderPass.h"

class DepthRenderPass : public RenderPass
{
public:
	~DepthRenderPass();

	void Create(Device* _devices, SwapChain* _swapChain) override;


	void Init() override;

};

