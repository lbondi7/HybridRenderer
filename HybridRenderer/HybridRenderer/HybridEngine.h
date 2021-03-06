#pragma once

#include "VulkanCore.h"
#include "RasterRenderer.h"
#include "RayTracingRenderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Timer.h"
#include "ImGUIWidgets.h"

class HybridEngine
{
public:

	HybridEngine() = default;

	~HybridEngine();


	void run();

private:

	void initialise();
	void prepare();
	void update();
	void render();
	void RecreateSwapChain();
	void Deinitilise();

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	static void cursorCallback(GLFWwindow* window, double xOffset, double yOffset);

	bool quit = false;

	std::unique_ptr<Window> window;
	std::unique_ptr<VulkanCore> core;

	SwapChain swapChain;

	RayTracingRenderer rayTracing;

	RasterRenderer raster;

	Camera camera;

	glm::mat4 textMatrix;
	std::vector<Buffer> buffers;

	//std::vector<GameObject> gameObjects;

	//uint32_t gameObjectCount = 10;

	Resources resources;

	//DescriptorSetManager* descriptorSetManager;

	//std::vector<VkDescriptorSet> lightDescSets;
	//std::vector<Buffer> lightBuffers;

	//Descriptor lightDescriptor;

	uint32_t imageIndex;
	size_t currentFrame = 0;

	std::vector<VkSemaphore> nextImageSemaphores;
	std::vector<VkSemaphore> presentSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	//glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
	//glm::vec3 lightPos = glm::vec3(-19.0f, 20.0f, -30.0f);
	//glm::vec3 lightRot = glm::vec3(0, 0, 0);

	//float lightFOV = 45.0f;

	Scene scene;

	Timer timer;

	ImGUIWidget widget;
	bool selected = false;
	bool cameraEnabled = false;
	bool shadowMapEnabled = false;
	bool rasterEnabled = true;
	bool raytraceEnabled = false;

};

