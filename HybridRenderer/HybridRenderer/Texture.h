#pragma once

#include "Constants.h"

#include "Device.h"
#include <string>

class Texture
{
public:

	Texture() = default;
	~Texture();

	virtual void Create(DeviceContext* _devices, uint32_t _width, uint32_t _height, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB, 
		VkImageTiling _tiling = VK_IMAGE_TILING_OPTIMAL, 
		VkImageUsageFlags _usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VkMemoryPropertyFlags _properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	virtual void createVkImage();

	void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask, const VkImageSubresourceRange& subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	virtual void CreateImageView(VkImageAspectFlags aspectFlags);

	void CopyFromBuffer(VkBuffer buffer);

	void CopyFromTexture(Texture other);

	virtual void Destroy();

	void DestroyImageViews();

	void insertImageMemoryBarrier(VkCommandBuffer cmdbuffer, 
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, 
		VkImageLayout oldImageLayout, VkImageLayout newImageLayout, 
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, 
		VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	VkImage image;
	VkFormat format;
	VkDeviceMemory memory;
	uint32_t width, height;
	VkImageUsageFlags usage;  
	VkMemoryPropertyFlags properties;
	VkImageTiling tiling;
	VkImageView imageView;

	VkDescriptorImageInfo descriptorInfo;

	DeviceContext* devices = nullptr;

	std::string name;
	uint32_t id;

	bool hasImageView = false;

protected:
	bool initialised = false;
	bool destroyed = false;
};


class TextureSampler : public Texture
{
public:

	~TextureSampler();

	virtual void CreateSampler();

	virtual void CreateSampler(const VkSamplerCreateInfo& samplerInfo);

	void Destroy() override;


	VkSampler sampler;

};

