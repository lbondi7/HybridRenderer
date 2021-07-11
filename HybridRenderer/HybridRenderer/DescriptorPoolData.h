#pragma once

#include "Constants.h"

#include <map>

struct DescriptorPoolData {

	VkDescriptorPool pool;
	using PoolSetCount = std::pair<uint32_t, uint32_t>;
	std::map<VkDescriptorType, PoolSetCount> count;
};