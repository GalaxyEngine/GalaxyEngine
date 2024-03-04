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

			const char* GetResourceName() const override { return "Script"; }

			static Weak<Script> Create(const Path& path);


			static inline ResourceType GetResourceType() { return ResourceType::Script; }
#ifdef WITH_EDITOR
			static void OpenScript(const Path& path);
		private:
			static void OpenWithVSCode(const Path& path);
			static void OpenWithVS(const Path& path);
#endif
		};
	}
}
