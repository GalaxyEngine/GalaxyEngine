#include "pch.h"

#include "Render/Camera.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/ResourceManager.h"

#include "Component/MeshComponent.h"

#include "Core/Application.h"

#ifdef WITH_EDITOR
#include "Editor/ThumbnailCreator.h"
#endif

#include "Physic/Plane.h"

#include "Wrapper/OBJLoader.h"
#include "Wrapper/FBXLoader.h"

namespace GALAXY {

	Vec3f Resource::BoundingBox::GetCenter() const
	{
		return (min + max) * 0.5f;
	}

	Vec3f Resource::BoundingBox::GetExtents() const
	{
		return max - GetCenter();
	}

	bool Resource::BoundingBox::IsOnFrustum(Render::Camera* camera, Component::Transform* objectTransform) const
	{
		auto center = GetCenter();
		auto extents = GetExtents();
		auto frustum = camera->GetFrustum();
		//Get global scale thanks to our transform
		const Vec3f globalCenter{ objectTransform->GetModelMatrix() * Vec4f(center, 1.f) };

		const Vec3f globalScale{ objectTransform->GetWorldScale() };

		// Scaled orientation
		const Vec3f right = objectTransform->GetRight() * extents.x;
		const Vec3f up = objectTransform->GetUp() * extents.y;
		const Vec3f forward = objectTransform->GetForward() * extents.z;

		const float newIi = 
			std::abs(Vec3f{ 1.f, 0.f, 0.f }.Dot(right)) +
			std::abs(Vec3f{ 1.f, 0.f, 0.f }.Dot(up)) +
			std::abs(Vec3f{ 1.f, 0.f, 0.f }.Dot(forward));

		const float newIj = 
			std::abs(Vec3f{ 0.f, 1.f, 0.f }.Dot(right)) +
			std::abs(Vec3f{ 0.f, 1.f, 0.f }.Dot(up)) +
			std::abs(Vec3f{ 0.f, 1.f, 0.f }.Dot(forward));

		const float newIk = 
			std::abs(Vec3f{ 0.f, 0.f, 1.f }.Dot(right)) +
			std::abs(Vec3f{ 0.f, 0.f, 1.f }.Dot(up)) +
			std::abs(Vec3f{ 0.f, 0.f, 1.f }.Dot(forward));

		//We not need to divide scale because it's based on the half extension of the AABB
		Vec3f newExtent = { globalScale.x * newIi, globalScale.y * newIj, globalScale.z * newIk };
		Vec3f newCenter = globalCenter;
		const BoundingBox globalAABB = BoundingBox(globalCenter - newExtent, globalCenter + newExtent);

		bool result = true;

		for (const auto& plane : frustum.planes)
		{
			if (!globalAABB.isOnOrForwardPlane(plane))
			{
				result = false;
				break;
			}
		}

		return result;
	}

	bool Resource::BoundingBox::isOnOrForwardPlane(const Physic::Plane& plane) const
	{
		const auto center = GetCenter();
		const auto extents = GetExtents();

		const float r = 
			  extents.x * std::abs(plane.normal.x) 
			+ extents.y * std::abs(plane.normal.y) 
			+ extents.z * std::abs(plane.normal.z);

		const float distance = plane.GetDistanceToPlane(center);

		return -r <= distance;
	}

	Resource::Model::~Model()
	{
		m_meshes.clear();
		m_meshes.shrink_to_fit();
	}

	void Resource::Model::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		StartLoading();
		
		if (p_fileInfo.GetExtension() == ".fbx")
		{
			m_modelType = Resource::ModelExtension::FBX;
			Wrapper::FBXLoader::Load(p_fileInfo.GetFullPath(), this);
		}
		else if (p_fileInfo.GetExtension() == ".obj")
		{
			m_modelType = Resource::ModelExtension::OBJ;
			Wrapper::OBJLoader::Load(p_fileInfo.GetFullPath(), this);
		}

		// Call every time because meshes can change
		CreateDataFile();
#ifdef WITH_EDITOR
		if (!Editor::ThumbnailCreator::IsThumbnailUpToDate(this))
			CreateThumbnail();
		for (Weak<Mesh>& mesh : m_meshes)
		{
			if (!Editor::ThumbnailCreator::IsThumbnailUpToDate(mesh.lock().get()))
				mesh.lock()->CreateThumbnail();
		}
#endif
	}

	void Resource::Model::Unload()
	{
		for (auto& mesh : m_meshes)
		{
			Resource::ResourceManager::GetInstance()->RemoveResource(mesh.lock()->GetFileInfo().GetFullPath());
		}
	}

#ifdef WITH_EDITOR
	Path Resource::Model::GetThumbnailPath() const
	{
		return Editor::ThumbnailCreator::GetThumbnailPath(this);
	}
#endif
	

	Shared<Core::GameObject> Resource::Model::ToGameObject()
	{
		Shared<Core::GameObject> root = std::make_shared<Core::GameObject>(GetFileInfo().GetFileNameNoExtension());
		size_t materialIndex = 0;
		for (auto& mesh : m_meshes)
		{
			Shared<Core::GameObject> meshGO = std::make_shared<Core::GameObject>(mesh.lock()->GetMeshName());
			auto meshComponent = meshGO->AddComponent<Component::MeshComponent>();
			meshComponent.lock()->SetMesh(mesh);
			for (auto& subMesh : mesh.lock()->m_subMeshes) {
				if (materialIndex < m_materials.size())
					meshComponent.lock()->AddMaterial(m_materials[materialIndex++]);
				else
					meshComponent.lock()->AddMaterial(ResourceManager::GetInstance()->GetDefaultMaterial());
			}
			if (m_meshes.size() != 1)
				root->AddChild(meshGO);
			else
				root = std::move(meshGO); // Only one mesh, return it
		}
		return root;
	}

	void Resource::Model::Serialize(CppSer::Serializer& serializer) const
	{
		IResource::Serialize(serializer);
		serializer << CppSer::Pair::BeginMap << "Model";
		serializer << CppSer::Pair::BeginTab;
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			auto mesh = m_meshes[i].lock();
			auto meshName = mesh->GetFileInfo().GetFileName();
			meshName = meshName.substr(meshName.find(':') + 1);
			serializer << CppSer::Pair::Key << mesh->GetUUID() << CppSer::Pair::Value << meshName;
		}
		serializer << CppSer::Pair::EndTab;
		serializer << CppSer::Pair::EndMap << "Model";
	}

	void Resource::Model::Deserialize(CppSer::Parser& parser)
	{
		IResource::Deserialize(parser);
		parser.PushDepth();
		const size_t meshCount = parser["Mesh Count"].As<size_t>();
		if (meshCount != -1)
		{
			PrintError("You have a out dated .gdata file");
			return;
		}
		for (auto [key, value] : parser.GetValueMap()[parser.GetCurrentDepth()])
		{
			auto meshPath = Mesh::CreateMeshPath(p_fileInfo.GetFullPath(), value.As<std::string>());
			auto mesh = Resource::ResourceManager::AddResource<Mesh>(meshPath).lock();
			Core::UUID uuid = std::stoull(key);
			mesh->SetUUID(uuid);
		}
	}

	void Resource::Model::ShowInInspector()
	{
#ifdef WITH_EDITOR
		if (ImGui::Button("Reload Thumbnail"))
		{
			CreateThumbnail();
		}
#endif
	}

	void Resource::Model::OnMeshLoaded()
	{
		// The has been sent is set true when all meshes have been added to the list
		if (!p_hasBeenSent)
			return;
		for (auto& mesh : m_meshes)
		{
			if (!mesh.lock()->HasBeenSent())
				return;
		}
		p_loaded.store(true);
		EOnLoad.Invoke();
		
		FinishLoading();
	}

#ifdef WITH_EDITOR
	void Resource::Model::CreateThumbnail()
	{
		Editor::ThumbnailCreator* thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();

		const Weak modelWeak = std::dynamic_pointer_cast<Model>(shared_from_this());

		thumbnailCreator->AddToQueue(modelWeak);
	}
#endif

	void Resource::Model::DrawBoundingBox(const Component::Transform* transform) const
	{
		const BoundingBox box = GetBoundingBox();
		const auto instance = Wrapper::Renderer::GetInstance();

		instance->DrawWireCube(transform->GetWorldPosition() + box.GetCenter(), Vec3f(
			(box.max.x - box.min.x) / 2.0f,
			(box.max.y - box.min.y) / 2.0f,
			(box.max.z - box.min.z) / 2.0f
		) * transform->GetWorldScale(), Vec4f(1, 0, 0, 1), 5.f);
	}

	List<Weak<Resource::Material>> Resource::Model::GetMaterialsOfMesh(const Mesh* mesh) const
	{
		List<Weak<Resource::Material>> materials;
		size_t index = 0;
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			if (m_meshes[i].lock().get() == mesh)
			{
				for (size_t j = 0; j < mesh->m_subMeshes.size(); j++)
				{
					if (m_materials.size() > index + j)
						materials.push_back(m_materials[index + j]);
					else
						materials.push_back(ResourceManager::GetDefaultMaterial());
				}
			}
			index += m_meshes[i].lock()->m_subMeshes.size();
		}
		return materials;
	}

	void Resource::Model::ComputeBoundingBox(const std::vector<std::vector<Vec3f>>& positionVertices)
	{
		for (size_t i = 0; auto & weakMesh : m_meshes)
		{
			const Shared<Mesh> mesh = weakMesh.lock();
			mesh->ComputeBoundingBox(positionVertices[i]);

			m_boundingBox.min.x = std::min(m_boundingBox.min.x, mesh->m_boundingBox.min.x);
			m_boundingBox.min.y = std::min(m_boundingBox.min.y, mesh->m_boundingBox.min.y);
			m_boundingBox.min.z = std::min(m_boundingBox.min.z, mesh->m_boundingBox.min.z);

			m_boundingBox.max.x = std::max(m_boundingBox.max.x, mesh->m_boundingBox.max.x);
			m_boundingBox.max.y = std::max(m_boundingBox.max.y, mesh->m_boundingBox.max.y);
			m_boundingBox.max.z = std::max(m_boundingBox.max.z, mesh->m_boundingBox.max.z);
			i++;
		}
	}
}