#pragma once

#include "GameObject.h"
#include "Resources.h"
#include "AccelerationStructure.h"
#include "ImGUIWidgets.h"

struct LightUBO {
	glm::mat4 proj;
	glm::mat4 view;
	alignas(16) glm::vec4 size_clippingPlanes{1.0f, 3.0f, 0.25f, 100.0f};
	alignas(16) glm::vec3 position{ 0.0f, 4.0f, -5.0f };
	alignas(16) glm::vec3 direction{ 0.0f, 4.0f, -5.0f };
	alignas(16) glm::vec4 colour{0.5f};
	alignas(16) glm::ivec4 extra;
};

class Scene
{
public:
	Scene() = default;
	~Scene();

	void Initialise(DeviceContext* deviceContext, Resources* resources);
	void Update(uint32_t imageIndex, float dt);
	void Destroy();

	void KeyCallback(int key, int scancode, int action, int mods);

	std::vector<GameObject> gameObjects;

	uint32_t gameObjectCount = 2;

	std::vector<VkDescriptorSet> lightDescSets;
	std::vector<Buffer> lightBuffers;

	Descriptor lightDescriptor;
	Descriptor asDescriptor;
	Descriptor rtASDescriptor;

	uint32_t imageIndex;

	glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
	glm::vec3 lightPos;
	glm::vec3 lightRot = glm::vec3(0, 0, 0);

	float lightFOV = 90.0f;

	std::vector<AccelerationStructure> bottomLevelASs;
	AccelerationStructure topLevelAS;

	LightUBO lightUBO;

	ImGUIWidget lightWidget;

	bool ortho = false;

private:

	DeviceContext* deviceContext;
	GameObject* CreateGameObject(Model* model);

	void LoadScene(const std::string& scene);

	DescriptorPool descriptorPool;

	bool lookAtCentre = true;
	Buffer objectBuffer;

	Resources* resources;

};

