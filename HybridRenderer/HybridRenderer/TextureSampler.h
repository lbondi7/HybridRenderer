#pragma once
#include "Texture.h"

class TextureSampler : public Texture
{
public:

	~TextureSampler();

	virtual void createSampler();

	virtual void createSampler(const VkSamplerCreateInfo& samplerInfo);

	void Destroy() override;


	VkSampler sampler;

};

