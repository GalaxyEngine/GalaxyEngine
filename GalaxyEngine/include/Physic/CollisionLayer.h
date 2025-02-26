#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Physic
    {
        using CollisionLayer = uint32_t;
        using CollisionMatrix = std::vector<std::vector<bool>>;

        // Collision layers
        namespace Layer
        {
            std::string GetLayerName(CollisionLayer layer);
            const std::vector<std::string>& GetLayerNames();
            bool CanCollide(CollisionLayer layer1, CollisionLayer layer2);
        }

        class CollisionLayerManager
        {
        public:
            static CollisionLayerManager& GetInstance();
            
            CollisionLayer AddLayer(const std::string& name);
            void RemoveLayer(CollisionLayer layer);
            void RemoveLayer(const std::string& name);

            CollisionLayer GetLayer(const std::string& name) const;

            static std::string GetLayerName(CollisionLayer layer);
            static const std::vector<std::string>& GetLayerNames();
            bool ContainsLayer(const std::string& name) const;

            void SetCollision(CollisionLayer layer1, CollisionLayer layer2, bool canCollide);

            static bool CanCollide(CollisionLayer layer1, CollisionLayer layer2);

            const CollisionMatrix& GetCollisionMatrix() const;
            CollisionMatrix& GetCollisionMatrix();

            void ChangeLayerName(CollisionLayer layer, const std::string& newName);

        private:
            std::unordered_map<std::string, CollisionLayer> m_layerMap;
            std::vector<std::string> m_layerNames; // Stores layer names by their ID.
            CollisionMatrix m_collisionMatrix;
        };
    }
}
