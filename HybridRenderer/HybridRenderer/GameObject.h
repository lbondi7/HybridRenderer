#pragma once
#include "Constants.h"
#include "Transform.h"
#include "Model.h"
#include "TextureSampler.h"
#include "Descriptor.h"

#include <vector>


struct ModelUBO {
	alignas(16) glm::mat4 model;
};


class GameObject
{
public:
	GameObject() = default;
	~GameObject();

	void Init(DeviceContext* deviceContext);
	void Update();
	void Destroy();

	Transform transform;
	glm::mat4 modelMatrix;

	Model* model = nullptr;
	Mesh* mesh = nullptr;
	TextureSampler* texture = nullptr;

	std::vector<Buffer> uniformBuffers;

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> offModelDescSets;

	bool shadowReceiver = true;
	bool shadowCaster = true;

	Descriptor descriptor;
	Descriptor offscreenDescriptor;

	glm::vec3 min;
	glm::vec3 max;

private:
	Transform prevTransform;

};

