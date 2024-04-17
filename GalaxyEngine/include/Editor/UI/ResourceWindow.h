#pragma once
#include "GalaxyAPI.h"

#include "Editor/UI/EditorWindow.h"

#include "Utils/Type.h"
#include "Resource/ResourceManager.h"

namespace GALAXY 
{
	namespace Editor
	{
		namespace UI
		{
			class ResourceWindow : public EditorWindow
			{
			public:
				void Initialize() override;
				void Draw() override;
			private:
				friend class MainBar;

				Resource::ResourceMap* m_resources;

				ResourceDir m_resourceDirDisplay = ResourceDir::Project;
			};
		}
	}
}
