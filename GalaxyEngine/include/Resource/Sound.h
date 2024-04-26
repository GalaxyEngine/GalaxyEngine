#pragma once
#include "GalaxyAPI.h"
#include "Resource/IResource.h"

namespace GALAXY 
{
	namespace Wrapper
	{
		class Audio;
	}
	namespace Resource
	{
		class Sound : public IResource
		{
		public:
			explicit Sound(const Path& fullPath);
			Sound& operator=(const Sound& other) = default;
			Sound(const Sound&) = default;
			Sound(Sound&&) noexcept = default;
			~Sound() override {}

			void Load() override;
			void Unload() override;

			void Play();
			void Stop();

			const char* GetResourceName() const override { return "Sound"; }

			static inline ResourceType GetResourceType() { return ResourceType::Sound; }
			
			void ShowInInspector() override;

		private:
			friend Wrapper::Audio;

		};
	}
}
