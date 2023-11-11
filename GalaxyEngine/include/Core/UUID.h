#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Core
	{
		class UUID
		{
		public:
			UUID();
			UUID(uint64_t uuid);
			UUID& operator=(const UUID& other) = default;
			UUID(const UUID&) = default;
			UUID(UUID&&) noexcept = default;
			virtual ~UUID();

			operator uint64_t() const { return m_UUID; }
		private:
			uint64_t m_UUID;
		};
	}

}

namespace std
{
	template<>
	struct hash<GALAXY::Core::UUID>
	{
		size_t operator()(const GALAXY::Core::UUID& uuid) const
		{
			return hash<uint64_t>()(uuid);
		}
	};
}
