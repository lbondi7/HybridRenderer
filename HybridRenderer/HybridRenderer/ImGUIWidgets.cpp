#include "ImGUIWidgets.h"

#include "imgui/imgui.h"

#include "imgui/imgui_impl_vulkan.h"

bool ImGUIWidget::NewWindow(const char* name)
{
	newWindow = true;
	return ImGui::Begin(name, &enabled);
}

void ImGUIWidget::EndWindow()
{
	if(newWindow)
		ImGui::End();

	newWindow = false;
}

void ImGUIWidget::Text(const char* text)
{
	ImGui::Text(text);
}

void ImGUIWidget::TextColoured(const char* text, const glm::vec4& col)
{
	ImVec4 imCol = { col.r, col.g, col.b, col.a };
	ImGui::TextColored(imCol, text);
}

void ImGUIWidget::TextColoured(const char* text, const glm::vec3& col)
{
	ImVec4 imCol = { col.r, col.g, col.b, 1.0f };
	ImGui::TextColored(imCol, text);
}

bool ImGUIWidget::CheckBox(const char * label, bool* checked)
{
	return ImGui::Checkbox(label, checked);
}

bool ImGUIWidget::Slider(const char * label, uint32_t* value, uint32_t min, uint32_t max)
{
	return ImGui::SliderInt(label, reinterpret_cast<int*>(value), min, max);
}

bool ImGUIWidget::Slider(const char* label, float* value, float min, float max)
{
	return ImGui::SliderFloat(label, value, min, max);
}

bool ImGUIWidget::Slider2(const char* label, float* values, float min, float max)
{
	return ImGui::SliderFloat2(label, values, min, max);
}

bool ImGUIWidget::Slider3(const char* label, float* values, float min, float max)
{
	return ImGui::SliderFloat3(label, values, min, max);
}

bool ImGUIWidget::Slider4(const char* label, float* values, float min, float max)
{
	return ImGui::SliderFloat4(label, values, min, max);
}

bool ImGUIWidget::Slider2(const char* label, glm::vec2& vec, float min, float max)
{
	float values[2] = { vec.x, vec.y };
	auto result = ImGui::SliderFloat2(label, values, min, max);
	vec = { values[0], values[1] };
	return result;
}

bool ImGUIWidget::Slider3(const char* label, glm::vec3& vec, float min, float max)
{
	float values[3] = { vec.x, vec.y, vec.z };
	auto result = ImGui::SliderFloat3(label, values, min, max);
	vec = { values[0], values[1], values[2] };
	return result;

}

bool ImGUIWidget::Slider4(const char* label, glm::vec4& vec, float min, float max)
{
	float values[4] = { vec.x, vec.y, vec.z, vec.w };
	auto result = ImGui::SliderFloat4(label, values, min, max);
	vec = { values[0], values[1], values[2], values[3] };
	return result;
}

bool ImGUIWidget::ColourEdit(const char* label, glm::vec3& vec)
{
	float values[3] = { vec.x, vec.y, vec.z };
	auto result = ImGui::ColorEdit3(label, values);
	vec = { values[0], values[1], values[2] };
	return result;
}

bool ImGUIWidget::ColourEdit(const char* label, glm::vec4& vec)
{
	float values[4] = { vec.x, vec.y, vec.z, vec.w };
	auto result = ImGui::ColorEdit4(label, values);
	vec = { values[0], values[1], values[2], values[3] };
	return result;
}

bool ImGUIWidget::ColourPicker(const char* label, glm::vec3& vec)
{
	float values[3] = { vec.x, vec.y, vec.z };
	auto result = ImGui::ColorPicker3(label, values);
	vec = { values[0], values[1], values[2] };
	return result;
}

bool ImGUIWidget::ColourPicker(const char* label, glm::vec4& vec)
{
	float values[4] = { vec.x, vec.y, vec.z, vec.w };
	auto result = ImGui::ColorPicker4(label, values);
	vec = { values[0], values[1], values[2], values[3] };
	return result;
}

bool ImGUIWidget::Vec2(const char* label, const glm::vec2& vec)
{
	float values[2] = { vec.x, vec.y };
	return ImGui::InputFloat2(label, values);
}

bool ImGUIWidget::Vec3(const char* label, const glm::vec3& vec)
{
	float values[3] = { vec.x, vec.y, vec.z};
	return ImGui::InputFloat3(label, values);
}

void ImGUIWidget::Image(size_t id, glm::vec2 _size, glm::vec2 _uv0, glm::vec2 _uv1, const glm::vec4& _tint, const glm::vec4& _border)
{

	ImVec2 size = ImVec2(_size.x, _size.y);
	ImVec2 uv0 = ImVec2(_uv0.x, _uv0.y);
	ImVec2 uv1 = ImVec2(_uv1.x, _uv1.y);
	ImVec4 tint = ImVec4(_tint.x, _tint.y, _tint.z, _tint.w);
	ImVec4 border = ImVec4(_border.x, _border.y, _border.z, _border.w);
	ImGui::Image((ImTextureID)imageSets[id], size, uv0, uv1, tint, border);

}

void ImGUIWidget::Image(size_t id, glm::vec2 _size, const glm::vec4& _tint, const glm::vec4& _border, glm::vec2 _uv0, glm::vec2 _uv1)
{

	ImVec2 size = ImVec2(_size.x, _size.y);
	ImVec2 uv0 = ImVec2(_uv0.x, _uv0.y);
	ImVec2 uv1 = ImVec2(_uv1.x, _uv1.y);
	ImVec4 tint = ImVec4(_tint.x, _tint.y, _tint.z, _tint.w);
	ImVec4 border = ImVec4(_border.x, _border.y, _border.z, _border.w);
	ImGui::Image((ImTextureID)imageSets[id], size, uv0, uv1, tint, border);

}

bool ImGUIWidget::NewMainMenu()
{
	return newMainMenu = ImGui::BeginMainMenuBar();
}

void ImGUIWidget::EndMainMenu()
{
	if (newMainMenu)
		ImGui::EndMainMenuBar();
}

bool ImGUIWidget::NewMenuBar()
{
	return newMenuBar = ImGui::BeginMenuBar();
}

void ImGUIWidget::EndMenuBar()
{
	if (newMenuBar)
		ImGui::EndMenuBar();
}

bool ImGUIWidget::NewMenu(const char* label)
{
	return newMenu = ImGui::BeginMenu(label);
}

void ImGUIWidget::EndMenu()
{
	if (newMenu)
	{
		ImGui::EndMenu();
		newMenu = false;
	}
}

bool ImGUIWidget::MenuItem(const char* label, bool* selected, bool enabled)
{
	return (newMenu || newMainMenu) ? ImGui::MenuItem(label, nullptr, selected, enabled) : false;
}

bool ImGUIWidget::NewChild()
{
	return ImGui::BeginChildFrame(0, {500, 500});
}

void ImGUIWidget::EndChild()
{
	ImGui::EndChildFrame();
}

void ImGUIWidget::SetupImage(size_t id, const TextureSampler& texture)
{
	if (id >= imageSets.size()) {
		imageSets.resize(id + 1);
	}
	imageSets[id] = ImGui_ImplVulkan_AddTextureD(texture.sampler, texture.imageView, texture.descriptorInfo.imageLayout);

}
