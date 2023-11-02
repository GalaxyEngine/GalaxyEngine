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
			Script(const Path& fullPath) : IResource(fullPath) {}
			Script& operator=(const Script& other) = default;
			Script(const Script&) = default;
			Script(Script&&) noexcept = default;
			virtual ~Script() {}

			void Load() override;

			void Unload() override;

			static Weak<Script> Create(const Path& path);

			static void OpenScript(const Path& path);

			static inline ResourceType GetResourceType() { return ResourceType::Script; }

		private:

		};
	}
}
