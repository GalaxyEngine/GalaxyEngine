#include "pch.h"
#include "Component/MeshCollider.h"

#include "Component/MeshComponent.h"
#include "Resource/ResourceManager.h"
#include "Resource/Mesh.h"
#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
#ifdef WITH_EDITOR
    void Component::MeshCollider::ShowInInspector()
    {
        Collider::ShowInInspector();

        if (ImGui::Checkbox("Convex", &m_isConvex))
            SetConvex(m_isConvex);

        if (Resource::ResourceManager::ResourceField<Resource::Mesh>(m_mesh, "Mesh"))
        {
            
        }
    }

    void Component::MeshCollider::OnEditorDraw()
    {
        Collider::OnEditorDraw();

        if (!p_gameObject->IsSelected())
            return;

        auto renderer = Wrapper::Renderer::GetInstance();
        auto mesh = m_isConvex ? m_convexMesh.lock() : m_mesh.lock();
        if (!mesh)
            return;
        const Shared<Render::Camera>& currentCamera = p_gameObject->GetScene()->GetCurrentCamera();
        if (currentCamera && !mesh->GetBoundingBox().IsOnFrustum(currentCamera.get(), GetTransform()))
            return;

        Weak<Resource::Material> material;

        if (!p_debugCollide)
            material = Resource::ResourceManager::GetOrLoad<Resource::Material>(ENGINE_RESOURCE_FOLDER_NAME "/materials/ColliderMaterial.mat");
        else
            material = Resource::ResourceManager::GetOrLoad<Resource::Material>(ENGINE_RESOURCE_FOLDER_NAME "/materials/CollisionMaterial.mat");
        if (!material.lock())
            return;
	
        renderer->EnableWireframe(true);
        mesh->Render(p_gameObject->GetTransform()->GetModelMatrix(), { material }, p_gameObject->GetScene(), p_gameObject->GetSceneGraphID());
        renderer->EnableWireframe(false);
    }
#endif

    void Component::MeshCollider::OnCreate()
    {
        if (Shared<MeshComponent> meshComponent = p_gameObject->GetComponent<MeshComponent>())
        {
            SetMesh(meshComponent->GetMesh());
        }
    }

    void Component::MeshCollider::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateMeshCollider(weak_from_this());
    }

    void Component::MeshCollider::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyMeshCollider(weak_from_this());
    }

    void Component::MeshCollider::Serialize(CppSer::Serializer& serializer)
    {
        Collider::Serialize(serializer);

        if (m_mesh.lock())
            serializer << CppSer::Pair::Key << "Model" << CppSer::Pair::Value << m_mesh.lock()->GetModel()->GetUUID();
        else
            serializer << CppSer::Pair::Key << "Model" << CppSer::Pair::Value << UUID_NULL;

        serializer << CppSer::Pair::Key << "Mesh Name" << CppSer::Pair::Value << (m_mesh.lock() ? m_mesh.lock()->GetMeshName() : NONE_RESOURCE);
    }

    void Component::MeshCollider::Deserialize(CppSer::Parser& parser)
    {
        Collider::Deserialize(parser);
        
        const uint64_t modelUUID = parser["Model"].As<uint64_t>();
        const std::string meshName = parser["Mesh Name"];
        const auto model = Resource::ResourceManager::GetOrLoad<Resource::Model>(modelUUID);

        if (model.lock())
        {
            auto meshPath = Resource::Mesh::CreateMeshPath(model.lock()->GetFileInfo().GetFullPath(), meshName);
            SetMesh(Resource::ResourceManager::GetOrLoad<Resource::Mesh>(meshPath));
        }
        else
        {
            PrintError("Model with uuid %llu not found", modelUUID);
        }
    }

    Physic::AABB Component::MeshCollider::GetAABB()
    {
        auto mesh = m_isConvex ? m_convexMesh.lock() : m_mesh.lock();
        if (!mesh) 
            return {};

        Physic::AABB aabb;
    
        Vec3f position = p_gameObject->GetTransform()->GetWorldPosition();
        Quat  rotation = p_gameObject->GetTransform()->GetWorldRotation();
        Vec3f scale    = p_gameObject->GetTransform()->GetWorldScale();

        bool firstVertex = true;

        // Iterate over all the mesh's position vertices.
        for (const auto& vertex : mesh->GetPositionVertices()) {
            Vec3f transformedVertex = position + rotation * (vertex * scale);

            // On the first vertex, initialize both min and max.
            if (firstVertex)
            {
                aabb.Min = transformedVertex;
                aabb.Max = transformedVertex;
                firstVertex = false;
            }
            else
            {
                // Update the min bounds.
                aabb.Min.x = std::min(aabb.Min.x, transformedVertex.x);
                aabb.Min.y = std::min(aabb.Min.y, transformedVertex.y);
                aabb.Min.z = std::min(aabb.Min.z, transformedVertex.z);

                // Update the max bounds.
                aabb.Max.x = std::max(aabb.Max.x, transformedVertex.x);
                aabb.Max.y = std::max(aabb.Max.y, transformedVertex.y);
                aabb.Max.z = std::max(aabb.Max.z, transformedVertex.z);
            }
        }

        return aabb;
    }

    Vec3f Component::MeshCollider::Support(const Vec3f& direction)
    {
        auto mesh = m_isConvex ? m_convexMesh.lock() : m_mesh.lock();
        if (!mesh)
            return {0.f, 0.f, 0.f};

        Vec3f worldPosition = p_gameObject->GetTransform()->GetWorldPosition();
        Quat  worldRotation = p_gameObject->GetTransform()->GetWorldRotation();
        Vec3f worldScale    = p_gameObject->GetTransform()->GetWorldScale();

        // Initialize the best dot product to negative infinity.
        float bestDot = -std::numeric_limits<float>::infinity();
        Vec3f bestVertex;

        // Iterate over each vertex in the mesh.
        for (const auto& vertex : mesh->GetPositionVertices())
        {
            // Transform the vertex into world space.
            // The transformation is: scale -> rotate -> translate.
            Vec3f transformedVertex = worldPosition + worldRotation * (vertex * worldScale);

            float currentDot = transformedVertex.Dot(direction);

            // Update if this vertex is farther along the direction.
            if (currentDot > bestDot)
            {
                bestDot = currentDot;
                bestVertex = transformedVertex;
            }
        }

        return bestVertex;
    }

    void Component::MeshCollider::SetMesh(const Weak<Resource::Mesh>& mesh)
    {
        m_mesh = mesh;

        if (IsConvex())
        {
            m_convexMesh = Wrapper::PhysicsWrapper::GetInstance()->GetConvexMesh(mesh.lock());
        }
    }

    void Component::MeshCollider::SetConvex(bool convex)
    {
        m_isConvex = convex;

        if (IsConvex() && m_mesh.lock())
        {
            m_convexMesh = Wrapper::PhysicsWrapper::GetInstance()->GetConvexMesh(m_mesh.lock());
        }
    }
}
