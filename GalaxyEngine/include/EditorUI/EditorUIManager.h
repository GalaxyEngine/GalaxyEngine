#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/Hierarchy.h"
namespace GALAXY::EditorUI {
	class Hierarchy;
	class EditorUIManager
	{
	public:
		~EditorUIManager() {}

		static EditorUIManager* GetInstance();

		void Initalize();

		void DrawUI();

	private:
		static std::unique_ptr<EditorUIManager> m_instance;
		std::unique_ptr<Hierarchy> m_hierarchy;
	};
}
