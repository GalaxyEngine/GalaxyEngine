#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Wrapper
	{
		class MTLLoader
		{
		public:
			MTLLoader() {}

			static bool Load(const std::filesystem::path& path);

		private:

		};
	}
}
