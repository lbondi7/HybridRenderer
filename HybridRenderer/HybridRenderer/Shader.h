#pragma once
#include "Constants.h"
#include "Device.h"

struct DescriptorData {

	bool operator == (const DescriptorData& other) {
		return set == other.set && binding == other.binding && type == other.type &&  stage & other.stage;
	}

	static bool contains(const DescriptorData& descriptor, const std::vector<DescriptorData>& descriptors) {
		for (auto d : descriptors)
		{
			if (descriptor == d)
				return true;
		}

		return false;
	}

	uint32_t set;
	uint32_t binding;
	VkDescriptorType type;
	VkShaderStageFlagBits stage;
};

class Shader
{
public:

	Shader() = default;
	~Shader();

	void Init(DeviceContext* _devices, const std::string& shaderName, VkShaderStageFlagBits _stage, const std::string& entryPoint = "main");

	void createModule(const std::vector<char>& code);

	void getDescriptors(const std::string& shaderName);

	void Destroy();

	VkPipelineShaderStageCreateInfo createInfo();

	VkShaderStageFlagBits stage;

	VkShaderModule shaderModule = VK_NULL_HANDLE;

	std::string entryPoint = "main";

	std::vector<DescriptorData> descriptors;


	static const char* getShaderTypeExtention(VkShaderStageFlagBits _stage);

private:

	DeviceContext* devices = nullptr;
};

