#pragma once

#include "Constants.h"

#include "Device.h"
#include <string>

class Texture
{
public:

	Texture() = default;
	~Texture();

	virtual void Create(Device* _devices, uint32_t _width, uint32_t _height, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB, 
		VkImageTiling _tiling = VK_IMAGE_TILING_OPTIMAL, 
		VkImageUsageFlags _usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VkMemoryPropertyFlags _properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	void createImage();

	void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

	void createSampler();

	void createSampler(const VkSamplerCreateInfo& samplerInfo);

	void createImageView(VkImageAspectFlags aspectFlags);

	void CopyFromBuffer(VkBuffer buffer);

	void Destroy();

	VkImage image;
	VkFormat format;
	VkDeviceMemory vkMemory;
	uint32_t width, height;
	VkImageUsageFlags usage;  
	VkMemoryPropertyFlags properties;
	VkImageTiling tiling;

	VkImageView imageView;
	VkSampler sampler;

	VkDescriptorImageInfo descriptorInfo;

	Device* devices = nullptr;

	std::string name;

	bool hasSampler = false;
	bool hasImageView = false;

private:

};

