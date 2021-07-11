#include "GameObject.h"

GameObject::~GameObject()
{
}

void GameObject::Init(DeviceContext* deviceContext)
{
	transform.getMatrix(model);
	prevTransform = transform;

	auto imageCount = deviceContext->imageCount;
	uniformBuffers.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++) {
		VkDeviceSize bufferSize = sizeof(ModelUBO);
		uniformBuffers[i].Create2(deviceContext, bufferSize, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	using BindingType = std::pair<uint32_t, VkDescriptorType>;

	DescriptorSetRequest request;
	request.ids.reserve(2);
	request.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
	request.ids.emplace_back(BindingType(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));

	DescriptorSetRequest request2;
	request2.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));

	request.data.reserve(imageCount);
	request2.data.reserve(imageCount);
	for (size_t i = 0; i < imageCount; i++) {

		request.data.push_back(&uniformBuffers[i].descriptorInfo);
		request.data.push_back(&texture->descriptorInfo);
		request2.data.push_back(&uniformBuffers[i].descriptorInfo);
	}

	deviceContext->getDescriptors(descriptor, request);
	deviceContext->getDescriptors(offscreenDescriptor, request2);

	//descriptor.initialise(deviceContext, request);
	//offscreenDescriptor.initialise(deviceContext, request2);
}

void GameObject::Update()
{
	if (transform != prevTransform)
	{

		transform.getMatrix(model);
		prevTransform = transform;
	}
}

void GameObject::Destroy()
{
	mesh = nullptr;
	texture = nullptr;
	for (auto& buffer : uniformBuffers)
	{
		buffer.Destroy();
	}
	for (auto& descriptorSet : descriptorSets)
	{
		descriptorSet = VK_NULL_HANDLE;
	}
	descriptorSets.clear();

	for (auto& descriptorSet : offModelDescSets)
	{
		descriptorSet = VK_NULL_HANDLE;
	}
	offModelDescSets.clear();

}
