#include "Texture.h"

#include "Utility.h"
#include "Initilizers.h"

Texture::~Texture()
{
    if (!destroyed)
    {
        if (hasSampler)
            vkDestroySampler(devices->logicalDevice, sampler, nullptr);
        if (hasImageView)
            vkDestroyImageView(devices->logicalDevice, imageView, nullptr);

        vkDestroyImage(devices->logicalDevice, image, nullptr);
        vkFreeMemory(devices->logicalDevice, vkMemory, nullptr);
    }
	devices = nullptr;
}


void Texture::Create(DeviceContext* _devices, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties) {

    devices = _devices;
    width = _width;
    height = _height;
    format = _format;
    tiling = _tiling;
    usage = _usage;
    properties = _properties;

    createImage();
}

void Texture::createImage() {

    VkImageCreateInfo imageInfo = Initialisers::imageCreateInfo(VK_IMAGE_TYPE_2D, width, height, 1, format, usage, tiling, VK_SAMPLE_COUNT_1_BIT);

    if (vkCreateImage(devices->logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(devices->logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = Initialisers::memoryAllocateInfo(memRequirements.size, Utility::findMemoryType(memRequirements.memoryTypeBits, devices->physicalDevice, properties));

    if (vkAllocateMemory(devices->logicalDevice, &allocInfo, nullptr, &vkMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(devices->logicalDevice, image, vkMemory, 0);
}

void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = devices->generateCommandBuffer();

    VkImageMemoryBarrier barrier = Initialisers::imageMemoryBarrier(image, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    devices->EndCommandBuffer(commandBuffer);

    descriptorInfo.imageLayout = newLayout;
}

void Texture::createSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(devices->physicalDevice, &properties);

    //VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_TRUE, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR);
    VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR, 
        VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK);

    if (vkCreateSampler(devices->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    descriptorInfo.sampler = sampler;
    hasSampler = true;
}

void Texture::createSampler(const VkSamplerCreateInfo& samplerInfo) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(devices->physicalDevice, &properties);

    ////VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_TRUE, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR);
    //VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR,
    //    VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK);

    if (vkCreateSampler(devices->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    descriptorInfo.sampler = sampler;
    hasSampler = true;
}

void Texture::createImageView(VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = Initialisers::imageViewCreateInfo(image, VK_IMAGE_VIEW_TYPE_2D, format, aspectFlags);

    if (vkCreateImageView(devices->logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    descriptorInfo.imageView = imageView;
    hasImageView = true;
}

void Texture::CopyFromBuffer(VkBuffer buffer) {
    VkCommandBuffer commandBuffer = devices->generateCommandBuffer();

    VkBufferImageCopy region = Initialisers::bufferImageCopy(width, height);

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    devices->EndCommandBuffer(commandBuffer);
}


void Texture::Destroy() {
    if(hasSampler)
        vkDestroySampler(devices->logicalDevice, sampler, nullptr);
    if(hasImageView)
        vkDestroyImageView(devices->logicalDevice, imageView, nullptr);

    vkDestroyImage(devices->logicalDevice, image, nullptr);
    vkFreeMemory(devices->logicalDevice, vkMemory, nullptr);
    destroyed = true;
}

void Texture::DestroyImageViews() {
    if (hasImageView)
        vkDestroyImageView(devices->logicalDevice, imageView, nullptr);
    destroyed = true;
}
