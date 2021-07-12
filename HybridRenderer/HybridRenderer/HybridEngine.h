#pragma once

#include "VulkanCore.h"
#include "RasterRenderer.h"
#include "Camera.h"
#include "GameObject.h"
#include "Timer.h"
#include "ImGUIWidgets.h"

class HybridEngine
{
public:

	HybridEngine() = default;

	~HybridEngine();


	void run();

private:

	struct LightUBO {
		alignas(16) glm::mat4 depthBiasMVP;
		alignas(16) glm::vec3 lightPos;
	}lightUBO;

	void initialise();
	void prepare();
	void update();
	void render();
	void deinitilise();

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	static void cursorCallback(GLFWwindow* window, double xOffset, double yOffset);

	bool quit = false;

	std::unique_ptr<Window> window;
	std::unique_ptr<VulkanCore> core;


	std::unique_ptr<RasterRenderer> renderer;

	Camera camera;

	std::vector<GameObject> gameObjects;

	uint32_t gameObjectCount = 10;

	Resources resources;

	//DescriptorSetManager* descriptorSetManager;

	std::vector<VkDescriptorSet> lightDescSets;
	std::vector<Buffer> lightBuffers;

	Descriptor lightDescriptor;

	uint32_t imageIndex;

	glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
	glm::vec3 lightPos = glm::vec3(-19.0f, 20.0f, -30.0f);

	float lightFOV = 90.0f;

	Timer timer;

	ImGUIWidget widget;
	bool selected = false;
	bool cameraEnabled = false;
	bool shadowMapEnabled = false;

};

