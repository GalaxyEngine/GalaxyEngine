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
			Script(const std::filesystem::path& fullPath) : IResource(fullPath) {}
			Script& operator=(const Script& other) = default;
			Script(const Script&) = default;
			Script(Script&&) noexcept = default;
			virtual ~Script() {}

			void Load() override;

			void Unload() override;

			static Weak<Script> Create(const std::filesystem::path& path);

		private:

		};
	}
}
