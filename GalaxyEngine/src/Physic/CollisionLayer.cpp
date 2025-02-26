#include "pch.h"
#include "Physic/CollisionLayer.h"

#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
    using namespace Physic;
    // In your CollisionLayerManager implementation file:

    std::string Layer::GetLayerName(CollisionLayer layer)
    {
        return CollisionLayerManager::GetLayerName(layer);
    }

    const std::vector<std::string>& Layer::GetLayerNames()
    {
        return CollisionLayerManager::GetLayerNames();
    }

    bool Layer::CanCollide(CollisionLayer layer1, CollisionLayer layer2)
    {
        return CollisionLayerManager::CanCollide(layer1, layer2);
    }

    CollisionLayerManager& CollisionLayerManager::GetInstance()
    {
        return Wrapper::PhysicsWrapper::GetInstance()->GetCollisionLayerManager();
    }

    CollisionLayer CollisionLayerManager::AddLayer(const std::string& name)
    {
        if (m_layerMap.contains(name)) {
            return m_layerMap[name];
        }
        // New layer ID is equal to the current size of the matrix.
        CollisionLayer newLayerId = static_cast<uint32_t>(m_collisionMatrix.size());
        m_layerMap[name] = newLayerId;
        // Store the layer name.
        m_layerNames.push_back(name);

        // Expand the matrix: add a new row with newLayerId + 1 columns.
        m_collisionMatrix.emplace_back(newLayerId + 1, true);
        // Expand each existing row to include the new column.
        for (size_t i = 0; i < newLayerId; ++i) {
            m_collisionMatrix[i].resize(newLayerId + 1, true);
        }
        return newLayerId;
    }

    void CollisionLayerManager::RemoveLayer(CollisionLayer layer)
    {
        if (layer >= m_layerNames.size())
            return;

        std::string removedLayerName = m_layerNames[layer];
        m_layerMap.erase(removedLayerName);

        m_layerNames.erase(m_layerNames.begin() + layer);

        m_collisionMatrix.erase(m_collisionMatrix.begin() + layer);

        for (auto& row : m_collisionMatrix) {
            if (row.size() > layer) {
                row.erase(row.begin() + layer);
            }
        }

        // Update the layer IDs in the mapping for layers that were shifted.
        for (auto& entry : m_layerMap) {
            if (entry.second > layer) {
                entry.second--;
            }
        }
    }

    void CollisionLayerManager::RemoveLayer(const std::string& name)
    {
        CollisionLayer layer = GetLayer(name);
        RemoveLayer(layer);
    }


    CollisionLayer CollisionLayerManager::GetLayer(const std::string& name) const
    {
        return m_layerMap.at(name);
    }

    // New method: Returns the name of the collision layer given its index.
    std::string CollisionLayerManager::GetLayerName(CollisionLayer layerId)
    {
        if (layerId < GetInstance().m_layerNames.size()) {
            return GetInstance().m_layerNames[layerId];
        }
        return "";
    }

    const std::vector<std::string>& CollisionLayerManager::GetLayerNames()
    {
        return GetInstance().m_layerNames;
    }

    bool CollisionLayerManager::ContainsLayer(const std::string& name) const
    {
        return m_layerMap.contains(name);
    }

    void CollisionLayerManager::SetCollision(CollisionLayer layer1, CollisionLayer layer2, bool canCollide)
    {
        m_collisionMatrix[layer1][layer2] = canCollide;
        m_collisionMatrix[layer2][layer1] = canCollide;
    }

    bool CollisionLayerManager::CanCollide(CollisionLayer layer1, CollisionLayer layer2)
    {
        return GetInstance().m_collisionMatrix[layer1][layer2];
    }

    const CollisionMatrix& CollisionLayerManager::GetCollisionMatrix() const
    {
        return m_collisionMatrix;
    }

    CollisionMatrix& CollisionLayerManager::GetCollisionMatrix()
    {
        return m_collisionMatrix;
    }

    void CollisionLayerManager::ChangeLayerName(CollisionLayer layer, const std::string& newName)
    {
        if (layer >= m_layerNames.size()) {
            return;
        }

        // If the new name already exists, return early to prevent duplicate names.
        if (m_layerMap.contains(newName)) {
            return;
        }

        std::string oldName = m_layerNames[layer];
        m_layerMap.erase(oldName);

        m_layerNames[layer] = newName;
        m_layerMap[newName] = layer;
    }
}
