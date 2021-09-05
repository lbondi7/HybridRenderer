#pragma once
#include "Texture.h"

class TextureSampler : public Texture
{
public:

	~TextureSampler();

	virtual void CreateSampler();

	virtual void CreateSampler(const VkSamplerCreateInfo& samplerInfo);

	void Destroy() override;


	VkSampler sampler;

};

