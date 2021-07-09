#pragma once
#include "Constants.h"

namespace ImGUIWidgets
{
	void NewWindow(const char* name = "Window");

	void EndWindow();

	void Text(const char* text);

	void TextColoured(const char* text, const glm::vec4& col);

	void TextColoured(const char* text, const glm::vec3& col);

	void CheckBox(bool* checked, const char * label = "Check Box");

	void FloatSlider(float* value, float min, float max, const char* label = "Float Slider");

	void Float2Slider(float* values, float min, float max, const char* label = "Float2 Slider");

	void Float3Slider(float* values, float min, float max, const char* label = "Float 3 Slider");

	void Float4Slider(float* values, float min, float max, const char* label = "Float 4 Slider");

	void Vec2Slider(glm::vec2& vec, float min, float max, const char* label = "Vec 2 Slider");

	void Vec3Slider(glm::vec3& vec, float min, float max, const char* label = "Vec 3 Slider");

	void Vec4Slider(glm::vec4& vec, float min, float max, const char* label = "Vec 4 Slider");

	void ColourEdit(glm::vec3& vec, const char* label = "Colour Editor");

	void ColourEdit(glm::vec4& vec, const char* label = "Colour Editor");

	void ColourPicker(glm::vec3& vec, const char* label = "Colour Picker");

	void ColourPicker(glm::vec4& vec, const char* label = "Colour Picker");

	void Vec2(const glm::vec2& vec, const char* label = "Vec 2");

	void Vec3(const glm::vec3& vec, const char* label = "Vec 2");

};

