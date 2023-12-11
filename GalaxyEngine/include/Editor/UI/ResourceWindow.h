#pragma once
#include "GalaxyAPI.h"

#include "Editor/UI/EditorWindow.h"

#include "Utils/Type.h"
#include "Resource/IResource.h"

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

				UMap<Path, Shared<Resource::IResource>>* m_resources;

				ResourceDir m_resourceDirDisplay = ResourceDir::Project;
			};
		}
	}
}
