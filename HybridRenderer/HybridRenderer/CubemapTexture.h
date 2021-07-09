#pragma once
#include "TextureSampler.h"

class CubemapTexture : public TextureSampler
{
public:

	void Create(DeviceContext* _devices, uint32_t _width, uint32_t _height, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB,
		VkImageTiling _tiling = VK_IMAGE_TILING_OPTIMAL,
		VkImageUsageFlags _usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VkMemoryPropertyFlags _properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) override;

	void createVkImage() override;

	void createImageView(VkImageAspectFlags aspectMask) override;

	void createSampler() override;

};

