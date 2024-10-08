#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"

namespace GALAXY 
{
	namespace Resource
	{
		class Script : public IResource
		{
		public:
			explicit Script(const Path& fullPath) : IResource(fullPath) {}
			Script& operator=(const Script& other) = default;
			Script(const Script&) = default;
			Script(Script&&) noexcept = default;
			~Script() override {}

			void Load() override;
			void Unload() override;

			void ShowInInspector() override;

			const char* GetResourceName() const override { return "Script"; }

			static inline ResourceType GetResourceType() { return ResourceType::Script; }

#ifdef WITH_EDITOR
			static Weak<Script> Create(const Path& path);
			static void OpenScript(const Path& path);
#endif
		private:
#ifdef WITH_EDITOR
			static void OpenWithVSCode(const Path& path);
#ifdef _WIN32
			static void OpenWithVS(const Path& path);
#endif
#endif

		private:
#ifdef WITH_EDITOR
			std::string m_scriptContent;
#endif
		};
	}
}
