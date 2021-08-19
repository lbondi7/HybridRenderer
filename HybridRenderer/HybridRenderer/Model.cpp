#include "Model.h"

Model::~Model()
{
}

void Model::Destroy()
{
	for (auto& mesh : meshes)
	{
		mesh->Destroy();
	}
}

void Model::Draw(VkCommandBuffer cmdBuffer)
{
	for(auto& mesh : meshes)
	{

		mesh->Bind(cmdBuffer);

		vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
	}
}
