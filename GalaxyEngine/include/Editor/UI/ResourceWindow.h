#pragma once
#include "GalaxyAPI.h"

#include "Editor/UI/EditorWindow.h"

#include "Utils/Type.h"
#include "Resource/ResourceManager.h"

namespace GALAXY::Resource
{
	class Model;
}

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
				bool m_debugThumbnail = false;
				Shared<Resource::IResource> m_selectedResource = nullptr;
				Shared<Resource::IResource> m_rightClickedResource = nullptr;

				bool m_shouldOpenPopup = false;

				ResourceDir m_resourceDirDisplay = ResourceDir::Project;
			};
		}
	}
}
