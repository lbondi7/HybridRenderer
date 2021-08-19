#pragma once

#include "Buffer.h"
#include "Vertex.h"
#include "Material.h"
#include "TextureSampler.h"
#include "Descriptor.h"

#include <memory>
#include <vector>

class Mesh
{
public:
	Mesh() = default;
	~Mesh();


	void Init(DeviceContext* _devices);

	void Destroy();

	void Bind(VkCommandBuffer cmdBuffer);


	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::string name;

	TextureSampler* texture;
	Material* material;

	Descriptor descriptor;
	DeviceContext* devices;

	bool bound = false;

};

