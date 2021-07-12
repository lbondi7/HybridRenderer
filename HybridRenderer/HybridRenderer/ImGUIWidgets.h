#pragma once
#include "Constants.h"

#include "TextureSampler.h"

class ImGUIWidget
{
public:
	bool NewWindow(const char* name);

	void EndWindow();

	void Text(const char* text);

	void TextColoured(const char* text, const glm::vec4& col);

	void TextColoured(const char* text, const glm::vec3& col);

	bool CheckBox(const char * label, bool* checked);

	bool Slider(const char * label, int* value, int min, int max);

	bool Slider(const char* label, float* value, float min, float max);

	bool Slider2(const char* label, float* values, float min, float max);

	bool Slider3(const char* label, float* values, float min, float max);

	bool Slider4(const char* label, float* values, float min, float max);

	bool Slider2(const char* label, glm::vec2& vec, float min, float max);

	bool Slider3(const char* label, glm::vec3& vec, float min, float max);

	bool Slider4(const char* label, glm::vec4& vec, float min, float max);

	bool ColourEdit(const char* label, glm::vec3& vec);

	bool ColourEdit(const char* label, glm::vec4& vec);

	bool ColourPicker(const char* label, glm::vec3& vec);

	bool ColourPicker(const char* label, glm::vec4& vec);

	bool Vec2(const char* label, const glm::vec2& vec);

	bool Vec3(const char* label, const glm::vec3& vec);

	void Image(size_t id, glm::vec2 size, glm::vec2 uv0 = glm::vec2(0.0f, 0.0f), glm::vec2 uv1 = glm::vec2(1.0f, 1.0f), 
		const glm::vec4& tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), const glm::vec4& border = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	bool NewMainMenu();

	void EndMainMenu();
	
	bool NewMenuBar();

	void EndMenuBar();

	bool NewMenu(const char* label);

	void EndMenu();

	bool MenuItem(const char* label, bool* selected, bool enabled = true);

	bool NewChild();

	void EndChild();

	void SetupImage(size_t id, const TextureSampler& texture);

	bool enabled = false;

private:
	std::vector<VkDescriptorSet> imageSets;
	bool newWindow = false;
	bool newMainMenu = false;
	bool newMenuBar = false;
	bool newMenu = false;

};

