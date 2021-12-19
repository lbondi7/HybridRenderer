#include "DebugLogger.h"
#include <iostream>


void DebugLogger::Log(const char* log)
{
	std::cout << log << std::endl;
}

void DebugLogger::Log(int i, const char* name)
{
	std::cout << name << ": " << i << std::endl;
}


void DebugLogger::Log(uint8_t i, const char* name)
{
    std::cout << name << ": " << i << std::endl;
}

void DebugLogger::Log(uint16_t i, const char* name)
{
    std::cout << name << ": " << i << std::endl;
}

void DebugLogger::Log(uint32_t i, const char* name)
{
    std::cout << name << ": " << i << std::endl;
}

void DebugLogger::Log(uint64_t i, const char* name)
{
    std::cout << name << ": " << i << std::endl;
}

void DebugLogger::Log(float f, const char* name)
{
	std::cout << name << ": " << f << std::endl;
}

void DebugLogger::Log(double d, const char* name)
{
    std::cout << name << ": " << d << std::endl;
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

VkResult DebugLogger::Log(VkResult result)
{
	switch (result)
    {
      case VK_SUCCESS:
        std::cout << "SUCCESS" << std::endl;
        break;
      case VK_NOT_READY:
          std::cout << "NOT READY" << std::endl;
          break;
      case VK_TIMEOUT:
          std::cout << "TIMEOUT" << std::endl;
          break;
      case VK_EVENT_SET:
          std::cout << "EVENT SET" << std::endl;
          break;
      case VK_EVENT_RESET:
          std::cout << "EVENT RESET" << std::endl;
          break;
      case VK_INCOMPLETE:
          std::cout << "INCOMPLETE" << std::endl;
          break;
      case VK_ERROR_OUT_OF_HOST_MEMORY:
          std::cout << "ERROR OUT OF HOST MEMORY" << std::endl;
          break;
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
          std::cout << "ERROR OUT OF DEVICE MEMORY" << std::endl;
          break;
      case VK_ERROR_INITIALIZATION_FAILED:
          std::cout << "ERROR INITIALIZATION FAILED" << std::endl;
          break;
      case VK_ERROR_DEVICE_LOST:
          std::cout << "ERROR DEVICE LOST" << std::endl;
          break;
      case VK_ERROR_MEMORY_MAP_FAILED:
          std::cout << "ERROR MEMORY MAP FAILED" << std::endl;
          break;
      case VK_ERROR_LAYER_NOT_PRESENT:
          std::cout << "ERROR LAYER NOT PRESENT" << std::endl;
          break;
      case VK_ERROR_EXTENSION_NOT_PRESENT:
          std::cout << "ERROR EXTENSION NOT PRESENT" << std::endl;
          break;
      case VK_ERROR_FEATURE_NOT_PRESENT:
          std::cout << "ERROR FEATURE NOT PRESENT" << std::endl;
          break;
      case VK_ERROR_INCOMPATIBLE_DRIVER:
          std::cout << "ERROR INCOMPATIBLE DRIVER" << std::endl;
          break;
      case VK_ERROR_TOO_MANY_OBJECTS:
          std::cout << "ERROR TOO MANY OBJECTS" << std::endl;
          break;
      case VK_ERROR_FORMAT_NOT_SUPPORTED:
          std::cout << "ERROR FORMAT NOT SUPPORTED" << std::endl;
          break;
      case VK_ERROR_SURFACE_LOST_KHR:
          std::cout << "ERROR SURFACE LOST KHR" << std::endl;
          break;
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
          std::cout << "ERROR NATIVE WINDOW IN USE KHR" << std::endl;
          break;
      case VK_SUBOPTIMAL_KHR:
          std::cout << "SUBOPTIMAL KHR)" << std::endl;
          break;
      case VK_ERROR_OUT_OF_DATE_KHR:
          std::cout << "ERROR OUT OF DATE KHR" << std::endl;
          break;
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
          std::cout << "ERROR INCOMPATIBLE DISPLAY KHR" << std::endl;
          break;
      case VK_ERROR_VALIDATION_FAILED_EXT:
          std::cout << "ERROR VALIDATION FAILED EXT" << std::endl;
          break;
      case VK_ERROR_INVALID_SHADER_NV:
          std::cout << "ERROR INVALID SHADER NV" << std::endl;
          break;
      default:
          std::cout << "UNKNOWN ERROR" << std::endl;
          break;
    }
	return result;
}

void DebugLogger::Log(void* data, const char* name)
{
    std::cout << name << ": " << data << std::endl;
}