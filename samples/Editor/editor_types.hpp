#pragma once

#include <vector>

namespace realware
{
	namespace core
	{
		struct sComponent;
	}

	namespace editor
	{
		class cEditorTextbox;
		class cEditorLabel;
	}
}

enum eSelectMode { NONE = 0, CREATE = 1, TRANSFORM = 2 };
struct sTextboxLabel
{
	realware::editor::cEditorTextbox* Textbox;
	realware::editor::cEditorLabel* Label;
};
enum eAssetSelectedType { ENTITY = 0, MATERIAL = 1, _COUNT = 2 };
struct sAsset
{
	sAsset(
		const std::string& name,
		const std::vector<std::string>& filenames
	) : Name(name), Filenames(filenames)
	{}

	std::string Name;
	std::vector<std::string> Filenames = { "", "" };
	std::vector<realware::core::sComponent*> Components = { nullptr, nullptr };
	glm::vec4 Color = glm::vec4(0.0f);
};