#include "ImGUIWidgets.h"

#include "imgui/imgui.h"

void ImGUIWidgets::NewWindow(const char* name)
{
	ImGui::Begin(name);

}

void ImGUIWidgets::EndWindow()
{
	ImGui::End();

}

void ImGUIWidgets::Text(const char* text)
{
	ImGui::Text(text);
}

void ImGUIWidgets::TextColoured(const char* text, const glm::vec4& col)
{
	ImVec4 imCol = { col.r, col.g, col.b, col.a };
	ImGui::TextColored(imCol, text);
}

void ImGUIWidgets::TextColoured(const char* text, const glm::vec3& col)
{
	ImVec4 imCol = { col.r, col.g, col.b, 1.0f };
	ImGui::TextColored(imCol, text);
}

void ImGUIWidgets::CheckBox(bool* checked, const char * label)
{
	ImGui::Checkbox(label, checked);
}

void ImGUIWidgets::FloatSlider(float* value, float min, float max, const char* label)
{
	ImGui::SliderFloat(label, value, min, max);
}

void ImGUIWidgets::Float2Slider(float* values, float min, float max, const char* label)
{
	ImGui::SliderFloat2(label, values, min, max);
}

void ImGUIWidgets::Float3Slider(float* values, float min, float max, const char* label)
{
	ImGui::SliderFloat3(label, values, min, max);
}

void ImGUIWidgets::Float4Slider(float* values, float min, float max, const char* label)
{
	ImGui::SliderFloat4(label, values, min, max);
}

void ImGUIWidgets::Vec2Slider(glm::vec2& vec, float min, float max, const char* label)
{
	float values[2] = { vec.x, vec.y };
	ImGui::SliderFloat2(label, values, min, max);
	vec = { values[0], values[1] };
}

void ImGUIWidgets::Vec3Slider(glm::vec3& vec, float min, float max, const char* label)
{
	float values[3] = { vec.x, vec.y, vec.z };
	ImGui::SliderFloat3(label, values, min, max);
	vec = { values[0], values[1], values[2] };
}

void ImGUIWidgets::Vec4Slider(glm::vec4& vec, float min, float max, const char* label)
{
	float values[4] = { vec.x, vec.y, vec.z, vec.w };
	ImGui::SliderFloat4(label, values, min, max);
	vec = { values[0], values[1], values[2], values[3] };
}

void ImGUIWidgets::ColourEdit(glm::vec3& vec, const char* label)
{
	float values[3] = { vec.x, vec.y, vec.z };
	ImGui::ColorEdit3(label, values);
	vec = { values[0], values[1], values[2] };
}

void ImGUIWidgets::ColourEdit(glm::vec4& vec, const char* label)
{
	float values[4] = { vec.x, vec.y, vec.z, vec.w };
	ImGui::ColorEdit4(label, values);
	vec = { values[0], values[1], values[2], values[3] };
}

void ImGUIWidgets::ColourPicker(glm::vec3& vec, const char* label)
{
	float values[3] = { vec.x, vec.y, vec.z };
	ImGui::ColorPicker3(label, values);
	vec = { values[0], values[1], values[2] };
}

void ImGUIWidgets::ColourPicker(glm::vec4& vec, const char* label)
{
	float values[4] = { vec.x, vec.y, vec.z, vec.w };
	ImGui::ColorPicker4(label, values);
	vec = { values[0], values[1], values[2], values[3] };
}

void ImGUIWidgets::Vec2(const glm::vec2& vec, const char* label)
{
	float values[2] = { vec.x, vec.y };
	ImGui::InputFloat2(label, values);
}

void ImGUIWidgets::Vec3(const glm::vec3& vec, const char* label)
{
	float values[3] = { vec.x, vec.y, vec.z};
	ImGui::InputFloat2(label, values);
}
