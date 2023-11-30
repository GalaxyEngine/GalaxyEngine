#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Editor
	{
		enum class ExternalTool
		{
			None = 0,
#ifdef _WIN32
			VisualStudio = 1,
#endif
			VisualStudioCode = 2
		};

		inline const char* SerializeExternalToolEnum()
		{
#ifdef _WIN32
			return "None\0Visual Studio\0Visual Studio Code";
#else
			return "None\0Visual Studio Code";
#endif
		}

		//TODO: load from file
		class EditorSettings
		{
		public:
			~EditorSettings() {}

			static EditorSettings& GetInstance();

			void Draw();

			ExternalTool GetExternalTool() const { return m_externalTool; }
			void SetExternalTool(ExternalTool val) { m_externalTool = val; }

			void SaveSettings();
			void LoadSettings();

		private:
			bool m_firstUpdate = false;
#if defined(_WIN32)
			ExternalTool m_externalTool = ExternalTool::VisualStudio;
#else
			ExternalTool m_externalTool = ExternalTool::VisualStudioCode;
#endif
		};
	}
}
