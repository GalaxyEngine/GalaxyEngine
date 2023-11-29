#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Editor
	{
		enum class ExternalTool
		{
			Unknown = 0,
			VisualStudio = 1,
			VisualStudioCode = 2
		};

		inline const char* SerializeExternalToolEnum()
		{
			return "Unknown\0Visual Studio\0Visual Studio Code";
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

		private:
#if defined(_WIN32)
			ExternalTool m_externalTool = ExternalTool::VisualStudio;
#else
			ExternalTool m_externalTool = ExternalTool::VisualStudioCode;
#endif
		};
	}
}
