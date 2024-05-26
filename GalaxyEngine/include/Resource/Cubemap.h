#pragma once
#include <array>

#include "GalaxyAPI.h"
#include "IResource.h"

namespace GALAXY 
{

    namespace Resource
    {
        class Cubemap : public IResource
        {
        public:
            explicit Cubemap(const Path& fullPath) : IResource(fullPath) {}
            Cubemap& operator=(const Cubemap& other);
            Cubemap(const Cubemap&) = default;
            Cubemap(Cubemap&&) noexcept = default;
            ~Cubemap() override = default;

            const char* GetResourceName() const override { return "Cubemap"; }
            static inline ResourceType GetResourceType() { return ResourceType::Cubemap; }

            void Load() override;
            void Save() const;

            static Weak<Cubemap> Create(const Path& path);

            void ShowInInspector();
        private:
            static std::string GetDirectionFromIndex(uint32_t index);
        private:
            // direction : +X, -X, +Y, -Y, +Z, -Z
            std::array<Weak<Texture>, 6> m_textures;
        };
    }
}
