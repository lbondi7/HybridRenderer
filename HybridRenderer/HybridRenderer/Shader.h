#pragma once
#include "Constants.h"
#include "Device.h"

class Shader
{
public:

	Shader() = default;
	~Shader();

	void Init(Device* _devices, const std::string& shaderName, VkShaderStageFlagBits _stage, const std::string& entryPoint = "main");

	void createModule(const std::vector<char>& code);


	VkShaderStageFlagBits stage;

	VkShaderModule module;

	std::string entryPoint = "main";

	VkPipelineShaderStageCreateInfo shaderInfo;

	Device* devices = nullptr;

	static const char* getShaderTypeExtention(VkShaderStageFlagBits _stage);


};

