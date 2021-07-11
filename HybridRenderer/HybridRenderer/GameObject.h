#pragma once
#include "Constants.h"
#include "Transform.h"
#include "Mesh.h"
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
	glm::mat4 model;

	Mesh* mesh = nullptr;
	TextureSampler* texture = nullptr;

	std::vector<Buffer> uniformBuffers;

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> offModelDescSets;

	bool shadowReceiver = true;
	bool shadowCaster = true;

	Descriptor descriptor;
	Descriptor offscreenDescriptor;


private:
	Transform prevTransform;

};

