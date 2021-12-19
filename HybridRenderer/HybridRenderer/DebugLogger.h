#pragma once

#include "Constants.h"


namespace DebugLogger
{
	void Log(const char* log);
	void Log(int i, const char* name = "Int");
	void Log(uint8_t i, const char* name = "Uint8_t");
	void Log(uint16_t i, const char* name = "Uint16_t");
	void Log(uint32_t i, const char* name = "Uint32_t");
	void Log(uint64_t i, const char* name = "Uint64_t");
	void Log(float f, const char* name = "Float");
	void Log(double d, const char* name);
	void Log(const glm::vec2& v, const char* name = "Vec2");
	void Log(const glm::vec3& v, const char* name = "Vec3");
	void Log(const glm::vec4& v, const char* name = "Vec3");
	VkResult Log(VkResult result);
	void Log(void* data, const char* name);
};

using namespace DebugLogger;