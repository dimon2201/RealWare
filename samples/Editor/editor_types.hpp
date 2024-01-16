#pragma once

namespace realware
{
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
	sAsset(const std::string& name) : Name(name) {}

	std::string Name;
};