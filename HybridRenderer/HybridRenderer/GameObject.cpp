#include "GameObject.h"

GameObject::~GameObject()
{
}

void GameObject::Init(DeviceContext* deviceContext)
{
	this->deviceContext = deviceContext;
	GetMatrix();

	if (mesh)
	{
		min = modelMatrix * glm::vec4(mesh->min, 1.0f);
		max = modelMatrix * glm::vec4(mesh->max, 1.0f);
		texture = mesh->texture;


		auto imageCount = deviceContext->imageCount;
		uniformBuffers.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++) {
			VkDeviceSize bufferSize = sizeof(ModelUBO);
			uniformBuffers[i].Allocate(deviceContext, bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		DescriptorSetRequest request;
		request.ids.emplace_back(DescriptorSetRequest::BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT));
		request.ids.emplace_back(DescriptorSetRequest::BindingType(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
		request.data.reserve(imageCount);
		for (size_t i = 0; i < imageCount; i++) {

			request.data.push_back(&uniformBuffers[i].descriptorInfo);
			request.data.push_back(&texture->descriptorInfo);
		}

		deviceContext->GetDescriptors(descriptor, &request);
	}
}

void GameObject::Update()
{
	GetMatrix();
	
	if (mesh) {
		min = modelMatrix * glm::vec4(mesh->min, 1.0f);
		max = modelMatrix * glm::vec4(mesh->max, 1.0f);
	}
	//prevTransform = transform;
}

void GameObject::Destroy()
{
	model = nullptr;
	texture = nullptr;
	for (auto& buffer : uniformBuffers)
	{
		buffer.Destroy();
	}
}

const glm::mat4& GameObject::GetMatrix()
{
	if (parent)
	{
		transform.getMatrix(modelMatrix, parent->GetMatrix());
	}
	else {
		transform.getMatrix(modelMatrix);
	}
	//transform.getMatrix(modelMatrix);
	return modelMatrix;
}

void GameObject::SetTexture(TextureSampler* texture)
{
	this->texture = texture;

	if (mesh) {
		auto imageCount = deviceContext->imageCount;

		DescriptorSetRequest request;
		request.ids.emplace_back(DescriptorSetRequest::BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT));
		request.ids.emplace_back(DescriptorSetRequest::BindingType(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
		request.data.reserve(imageCount);
		for (size_t i = 0; i < imageCount; i++) {

			request.data.push_back(&uniformBuffers[i].descriptorInfo);
			request.data.push_back(&texture->descriptorInfo);
		}

		deviceContext->GetDescriptors(descriptor, &request);
	}
}
