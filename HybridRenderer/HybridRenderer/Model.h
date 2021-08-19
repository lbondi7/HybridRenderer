#pragma once

#include "Mesh.h"

class Model
{
public:
	Model() = default;
	~Model();


	void Destroy();

	void Draw(VkCommandBuffer cmdBuffer);

	glm::vec3 min;
	glm::vec3 max;
	std::string name;
	std::vector<std::unique_ptr<Mesh>> meshes;
};

