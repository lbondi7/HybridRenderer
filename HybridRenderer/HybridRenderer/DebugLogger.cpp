#include "DebugLogger.h"


void DebugLogger::Log(const char* log)
{
	std::cout << log << std::endl;
}

void DebugLogger::Log(int i, const char* name)
{
	std::cout << name << ": " << i << std::endl;
}

void DebugLogger::Log(float f, const char* name)
{
	std::cout << name << ": " << f << std::endl;
}

void DebugLogger::Log(const glm::vec2& v, const char* name)
{
	std::cout << name << ": x(" << v.x << "), y(" << v.y << ")" << std::endl;
}

void DebugLogger::Log(const glm::vec3& v, const char* name)
{
	std::cout << name << ": x(" << v.x << "), y(" << v.y << "), z(" << v.z << ")" << std::endl;
}

void DebugLogger::Log(const glm::vec4& v, const char* name)
{
	std::cout << name << ": x(" << v.x << "), y(" << v.y << "), z(" << v.z << "), w(" << v.w << ")" <<std::endl;
}
