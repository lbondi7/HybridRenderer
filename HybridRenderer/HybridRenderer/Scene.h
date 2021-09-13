#pragma once

#include "GameObject.h"
#include "Resources.h"
#include "AccelerationStructure.h"


class Scene
{
public:
	Scene() = default;
	~Scene();

	void Initialise(DeviceContext* deviceContext, Resources* resources);
	void Update(uint32_t imageIndex, float dt);
	void Destroy();

	std::vector<GameObject> gameObjects;

	uint32_t gameObjectCount = 2;

	std::vector<VkDescriptorSet> lightDescSets;
	std::vector<Buffer> lightBuffers;

	Descriptor lightDescriptor;
	Descriptor asDescriptor;
	Descriptor rtASDescriptor;

	uint32_t imageIndex;

	glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
	glm::vec3 lightPos = glm::vec3(-19.0f, 20.0f, -30.0f);
	glm::vec3 lightRot = glm::vec3(0, 0, 0);

	float lightFOV = 45.0f;

	std::vector<AccelerationStructure> bottomLevelASs;
	AccelerationStructure topLevelAS;

private:
	struct LightUBO {
		alignas(16) glm::mat4 depthBiasMVP;
		alignas(16) glm::vec3 lightPos;
	}lightUBO;
	DeviceContext* deviceContext;
};

