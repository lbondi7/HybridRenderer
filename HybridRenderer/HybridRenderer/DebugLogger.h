#pragma once

#include "Constants.h"
namespace DebugLogger
{
	void Log(const char* log);
	void Log(int i, const char* name = "Int");
	void Log(float f, const char* name = "Float");
	void Log(const glm::vec2& v, const char* name = "Vec2");
	void Log(const glm::vec3& v, const char* name = "Vec3");
	void Log(const glm::vec4& v, const char* name = "Vec3");

};

using namespace DebugLogger;