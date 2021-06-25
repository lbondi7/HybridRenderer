#pragma once
#include "Constants.h"
#include "Transform.h"
#include "Mesh.h"
#include "Image.h"

#include <vector>



class GameObject
{
public:
	GameObject() = default;
	~GameObject();

	void Init();
	void Update();
	void Destroy();

	Transform transform;
	glm::mat4 model;

	Mesh* mesh = nullptr;
	Image* image = nullptr;

	std::vector<Buffer> uniformBuffers;

	std::vector<VkDescriptorSet> descriptorSets;

	bool shadowReceiver = true;
	bool shadowCaster = true;

private:
	Transform prevTransform;

};

