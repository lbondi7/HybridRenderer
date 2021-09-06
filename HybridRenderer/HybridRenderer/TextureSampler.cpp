#include "TextureSampler.h"

#include "Initilizers.h"

TextureSampler::~TextureSampler()
{
    if (initialised && !destroyed)
        vkDestroySampler(devices->logicalDevice, sampler, nullptr);
}

void TextureSampler::CreateSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(devices->physicalDevice, &properties);

    //VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_TRUE, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR);
    VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK);

    if (vkCreateSampler(devices->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    descriptorInfo.sampler = sampler;
}

void TextureSampler::CreateSampler(const VkSamplerCreateInfo& samplerInfo) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(devices->physicalDevice, &properties);

    if (vkCreateSampler(devices->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    descriptorInfo.sampler = sampler;
}

void TextureSampler::Destroy()
{
    if(!destroyed)
        vkDestroySampler(devices->logicalDevice, sampler, nullptr);

    Texture::Destroy();

}
