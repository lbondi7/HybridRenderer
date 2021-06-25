#pragma once

#include "Buffer.h"
#include "Vertex.h"

#include <memory>
#include <vector>

class Mesh
{
public:
	Mesh() = default;
	~Mesh();


	void Init(Device* _devices);

	void Destroy();

	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::string name;

	Device* devices;
};

