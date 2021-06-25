#pragma once

#include "Mesh.h"
#include "Image.h"
#include "Shader.h"
#include "Device.h"

#include <string>
#include <map>
#include <memory>

class Resources
{
public:
	Resources() = default;
	~Resources();

	void Init(Device* _devices);

	void Destroy();


	void LoadMesh(const std::string& name);

	void LoadImage(const std::string& name);

	void LoadShader(const std::string& name, VkShaderStageFlagBits stage);

	std::map<std::string, std::unique_ptr<Mesh>> meshes;
	std::map<std::string, std::unique_ptr<Image>> images;
	std::map<std::string, std::unique_ptr<Shader>> vertexShaders;
	std::map<std::string, std::unique_ptr<Shader>> fragmentShaders;

	Device* devices;
private:
	void InitShader(const std::string& name, Shader* shader, VkShaderStageFlagBits stage);

	const std::string trimName(const std::string& name);
};

