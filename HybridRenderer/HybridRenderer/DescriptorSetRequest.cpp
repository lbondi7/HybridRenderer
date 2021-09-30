#include "DescriptorSetRequest.h"

#include "Initilizers.h"

#include "Buffer.h"
#include <vector>


void DescriptorBinding::AddBufferData(void* data)
{
	auto buffers = reinterpret_cast<Buffer*>(data);
	this->data.reserve(3);
	for(size_t i = 0; i< 3; ++i)
	{
		this->data.push_back(&buffers[i].descriptorInfo);
	}
}

void DescriptorBinding::AddData(void* data)
{
	this->data.reserve(3);
	this->data.push_back(data);
	this->data.push_back(data);
	this->data.push_back(data);
}

DescriptorSetRequest::DescriptorSetRequest(size_t i)
{
	bindings.reserve(i);
}

DescriptorSetRequest::DescriptorSetRequest(const std::vector<LayoutSetOrder>& tags, size_t i)
{
	bindings.reserve(i);
	layoutTags.insert(layoutTags.cend(), tags.begin(), tags.cend());
}

void DescriptorSetRequest::AddDescriptorBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits shaderFlags, uint32_t count)
{
	auto& _binding = bindings.emplace_back();
	_binding.binding = binding;
	_binding.type = type;
	_binding.descriptorCount = count;
	_binding.shaderFlags = shaderFlags;
	totalSets += count;
}

void DescriptorSetRequest::AddDescriptorBufferData(size_t binding, void* data)
{
	for(auto& b : bindings)
	{
		if(b.binding == binding)
			b.AddBufferData(data);
	}
}

void DescriptorSetRequest::AddDescriptorImageData(size_t binding, void* data)
{
	for (auto& b : bindings)
	{
		if (b.binding == binding)
			b.AddData(data);
	}
}

void DescriptorSetRequest::AddDescriptorSetLayoutTags(const std::vector<std::string>& tags)
{
	//layoutTags.insert(layoutTags.end(), tags.begin(), tags.end());
}

void DescriptorSetRequest::AddDescriptorSetLayoutTag(const std::string& tag)
{
	//layoutTags.emplace_back(tag);
}
