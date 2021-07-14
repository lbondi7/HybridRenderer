#pragma once

#include "Mesh.h"
#include "TextureSampler.h"
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

	void Init(DeviceContext* _devices);

	void Destroy();


	void getShaders(std::vector<Shader*>& shaders, const std::vector<std::string>& shaderNames);

	void LoadMesh(const std::string& name);

	void LoadTexture(const std::string& name);

	void LoadShader(const std::string& name, VkShaderStageFlagBits stage);

	std::map<std::string, std::unique_ptr<Mesh>> meshes;
	std::map<std::string, std::unique_ptr<TextureSampler>> textures;
	std::map<std::string, std::unique_ptr<Shader>> vertexShaders;
	std::map<std::string, std::unique_ptr<Shader>> fragmentShaders;
	std::map<std::string, std::unique_ptr<Shader>> raygenShaders;
	std::map<std::string, std::unique_ptr<Shader>> rayclosesthitShaders;
	std::map<std::string, std::unique_ptr<Shader>> raymissShaders;

	DeviceContext* devices;

private:
	void InitShader(const std::string& name, Shader* shader, VkShaderStageFlagBits stage);

	const std::string trimName(const std::string& name);
};

