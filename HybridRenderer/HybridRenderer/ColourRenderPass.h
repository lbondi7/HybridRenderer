#pragma once

#include "RenderPass.h"

class ColourRenderPass : public RenderPass
{
public:
	~ColourRenderPass();

	void Create(Device* _devices, SwapChain* _swapChain) override;

	void Init() override;

};

