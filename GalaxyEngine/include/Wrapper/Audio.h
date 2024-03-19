#pragma once
#include "GalaxyAPI.h"

#include <memory>
#include <filesystem>

struct ma_device;
struct ma_context;
namespace GALAXY 
{
	namespace Resource
	{
		class Sound;
	}
	namespace Wrapper 
	{
		class Audio
		{
		public:
			Audio() {}
			Audio& operator=(const Audio& other) = default;
			Audio(const Audio&) = default;
			Audio(Audio&&) noexcept = default;
			virtual ~Audio() {}

			bool Initialize();

			void Release();

			static Audio* GetInstance() {
				if (s_instance == nullptr) {
					s_instance = std::make_unique<Audio>();
				}
				return s_instance.get();
			}

			bool LoadSound(const std::filesystem::path& fullPath, Resource::Sound* outputSound);
		private:
			static std::unique_ptr<Audio> s_instance;

			ma_device* m_device = nullptr;
			ma_context* m_context = nullptr;
		};
	}
}
