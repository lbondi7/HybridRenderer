#pragma once

#include "Buffer.h"
#include "Vertex.h"
#include "Material.h"
#include "Texture.h"
#include "Descriptor.h"

#include <memory>
#include <vector>

class Mesh
{
public:
	Mesh() = default;
	~Mesh();


	void Init(DeviceContext* _devices);

	//void SetTexture(TextureSampler* texture);

	void Destroy();

	void Bind(VkCommandBuffer cmdBuffer);


	Buffer vertexBuffer;
	Buffer indexBuffer;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::string name;
	glm::vec3 min;
	glm::vec3 max;

	TextureSampler* texture;
	//Material* material;

	Descriptor descriptor;
	DeviceContext* devices;

	bool bound = false;

};

