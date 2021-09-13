#include "GameObject.h"

GameObject::~GameObject()
{
}

void GameObject::Init(DeviceContext* deviceContext)
{
	transform.getMatrix(modelMatrix);

	min = modelMatrix * glm::vec4(model->min, 1.0f);
	max = modelMatrix * glm::vec4(model->max, 1.0f);

	auto imageCount = 3;
	uniformBuffers.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++) {
		VkDeviceSize bufferSize = sizeof(ModelUBO);
		uniformBuffers[i].Allocate(deviceContext, bufferSize, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	DescriptorSetRequest request;
	request.ids.emplace_back(DescriptorSetRequest::BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT));

	request.data.reserve(imageCount);
	for (size_t i = 0; i < imageCount; i++) {

		request.data.push_back(&uniformBuffers[i].descriptorInfo);
	}

	deviceContext->GetDescriptors(descriptor, &request);
}

void GameObject::Update()
{
	if (transform != prevTransform)
	{
		transform.getMatrix(modelMatrix);
	
		min = modelMatrix * glm::vec4(model->min, 1.0f);
		max = modelMatrix * glm::vec4(model->max, 1.0f);
		
		prevTransform = transform;
	}
}

void GameObject::Destroy()
{
	model = nullptr;
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
