#include "pch.h"
#include "Wrapper/FBXLoader.h"
#include "Wrapper/ImageLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"

#include <openFBX/ofbx.h>
#include <stb/stb_image.h>

namespace GALAXY
{
	const char* ReadFile(const char* filename, uint32_t& size, bool& success)
	{
		std::ifstream is(filename, std::ifstream::binary);
		if (is.is_open()) {
			success = true;
			// get length of file:
			is.seekg(0, is.end);
			uint32_t length = (uint32_t)is.tellg();
			is.seekg(0, is.beg);

			char* buffer = new char[length];

			// read data as a block:
			is.read(buffer, length);
			is.close();
			size = length;
			return buffer;
		}
		else
		{
			success = false;
			PrintWarning("File %s cannot be found", filename);
			return 0;
		}
	}

	Vec4f ToVec4f(const ofbx::Color& c)
	{
		return { c.r, c.g, c.b, 1 };
	}

	Vec3f ToVec3f(const ofbx::Vec3& v)
	{
		return { v.x, v.y, v.z };
	}

	Vec2f ToVec2f(const ofbx::Vec2& v)
	{
		return { v.x, v.y };
	}

	Quat ToQuat(const ofbx::Quat& q)
	{
		return { q.x, q.y, q.z, q.w };
	}

	void Wrapper::FBXLoader::Load(const std::filesystem::path& fullPath, Resource::Model* outputModel)
	{
		PROFILE_SCOPE_LOG("FBXLoader::Load(%s)", fullPath.generic_string().c_str());
		uint32_t size;
		bool sucess;
		auto data = (ofbx::u8*)ReadFile(fullPath.generic_string().c_str(), size, sucess);
		if (!sucess) {
			delete[] data;
			data = nullptr;
			return;
		}
		ofbx::IScene* Scene = ofbx::load(data, size, (ofbx::u16)ofbx::LoadFlags::NONE);
		if (Scene)
		{
			LoadTextures(Scene, fullPath);
			LoadModel(Scene, fullPath, outputModel);
			Scene->destroy();
		}
		delete[] data;
		data = nullptr;
	}

	void Wrapper::FBXLoader::LoadTextures(ofbx::IScene* fbxScene, const std::filesystem::path& fullPath)
	{
		for (int i = 0; i < fbxScene->getEmbeddedDataCount(); i++) {
			constexpr int size = 4096;
			char tmp[size];
			fbxScene->getEmbeddedFilename(i).toString(tmp);
			std::filesystem::path texPath = tmp;
			std::filesystem::path texPath2 = fullPath.parent_path().stem() / texPath.filename();
			if (!std::filesystem::exists(texPath) || !std::filesystem::exists(texPath2)) {
				const ofbx::DataView& embeddedData = fbxScene->getEmbeddedData(i); // Assuming this function exists.

				const ofbx::u8* textureData = embeddedData.begin + 4;
				std::size_t textureSize = static_cast<std::size_t>(embeddedData.end - embeddedData.begin - 4);

				/*
				* TODO: Export settings : export texture, if not exporting it,
				* the model need to be load every time to get the texture, the texture also need a uuid save into the.gdata of the fbx
				* or only get the uuid, but the texture could not be load so no display
				*/
				auto image = Wrapper::ImageLoader::LoadFromMemory((unsigned char*)textureData, static_cast<int>(textureSize));

				Resource::Texture::CreateWithData(texPath2, image);

				const bool exportImage = true;
				if (exportImage)
				{
					std::filesystem::create_directories(texPath2);
					Wrapper::ImageLoader::SaveImage(texPath.string().c_str(), image);
				}
			}
			else
			{
				if (!std::filesystem::exists(texPath))
					Resource::ResourceManager::GetOrLoad<Resource::Texture>(texPath2);
				else
					Resource::ResourceManager::GetOrLoad<Resource::Texture>(texPath);
			}
		}
	}

	std::vector<float> ComputeVertices(std::vector<Vec3f> positions, std::vector<Vec2f> textureUVs, std::vector<Vec3f> normals, std::vector<Vec3i> indices)
	{
		std::vector<float> finalVertices;
		std::vector<Vec3f> tangents;
		tangents.resize(indices.size());

		for (size_t k = 0; k < indices.size(); k += 3)
		{
			const Vec3i& idx0 = indices[k];
			const Vec3i& idx1 = indices[k + 1];
			const Vec3i& idx2 = indices[k + 2];

			const Vec3f& Edge1 = positions[idx1.x] - positions[idx0.x];
			const Vec3f& Edge2 = positions[idx2.x] - positions[idx0.x];

			const float DeltaU1 = textureUVs[idx1.y].x - textureUVs[idx0.y].x;
			const float DeltaV1 = textureUVs[idx1.y].y - textureUVs[idx0.y].y;
			const float DeltaU2 = textureUVs[idx2.y].x - textureUVs[idx0.y].x;
			const float DeltaV2 = textureUVs[idx2.y].y - textureUVs[idx0.y].y;

			float f = DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1;
			if (fabs(f) < 1e-6) {
				f = 1.0f; // Prevent division by zero and provide a default value for 'f'
			}
			else {
				f = 1.0f / f;
			}

			Vec3f Tangent;

			Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
			Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
			Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

			Tangent.Normalize();

			tangents[k] = Tangent;
			tangents[k + 1] = Tangent;
			tangents[k + 2] = Tangent;
		}

		for (size_t i = 0; i < indices.size(); i++)
		{
			const Vec3i& idx = indices[i];

			finalVertices.push_back(positions[idx.x].x);
			finalVertices.push_back(positions[idx.x].y);
			finalVertices.push_back(positions[idx.x].z);

			finalVertices.push_back(textureUVs[idx.y].x);
			finalVertices.push_back(textureUVs[idx.y].y);

			finalVertices.push_back(normals[idx.z].x);
			finalVertices.push_back(normals[idx.z].y);
			finalVertices.push_back(normals[idx.z].z);

			finalVertices.push_back(tangents[i].x);
			finalVertices.push_back(tangents[i].y);
			finalVertices.push_back(tangents[i].z);
		}
		return finalVertices;
	}

	void SetMaterialTexture(Weak<Resource::Texture>* texture, const ofbx::Material* fbxMaterial, const std::filesystem::path& fullPath, ofbx::Texture::TextureType texType)
	{
		if (auto fbxTexture = fbxMaterial->getTexture(texType))
		{
			constexpr int size = 4096;
			char texName[size];
			fbxTexture->getRelativeFileName().toString(texName);
			Path texPath = fullPath.parent_path() / texName;
			if (!std::filesystem::exists(texPath))
				*texture = Resource::ResourceManager::GetResource<Resource::Texture>(texPath).lock();
			else
				*texture = Resource::ResourceManager::GetOrLoad<Resource::Texture>(texPath).lock();
		}
	}

	void Wrapper::FBXLoader::LoadModel(ofbx::IScene* fbxScene, const std::filesystem::path& fullPath, Resource::Model* outputModel)
	{
		std::vector<std::vector<Vec3f>> allPositions;
		for (int i = 0; i < fbxScene->getMeshCount(); i++) {
			const ofbx::Mesh* fbxMesh = fbxScene->getMesh(i);

			// Load Materials
			size_t materialCount = fbxMesh->getMaterialCount();
			for (int j = 0; j < materialCount; j++) {
				const ofbx::Material* fbxMaterial = fbxMesh->getMaterial(j);
				const std::filesystem::path& materialFullPath = fullPath.parent_path() / (std::string(fbxMaterial->name) + ".mat");
				Shared<Resource::Material> material = nullptr;
				if (std::filesystem::exists(materialFullPath))
					material = Resource::ResourceManager::GetOrLoad<Resource::Material>(materialFullPath).lock();
				else
					material = Resource::ResourceManager::GetResource<Resource::Material>(materialFullPath).lock();

				//TODO : Add Export settings, so this is only call when exporting
				if (!material /* && exporting*/)
				{
					material = Resource::ResourceManager::AddResource<Resource::Material>(materialFullPath).lock();
					material->m_shader = Resource::ResourceManager::GetInstance()->GetDefaultShader();
					material->SetAmbient(ToVec4f(fbxMaterial->getAmbientColor()));
					material->SetDiffuse(ToVec4f(fbxMaterial->getDiffuseColor()));
					material->SetSpecular(ToVec4f(fbxMaterial->getSpecularColor()));
					material->SetHeightScale(static_cast<float>(fbxMaterial->getBumpFactor()));

					auto albedo = material->GetAlbedo();
					SetMaterialTexture(&albedo, fbxMaterial, fullPath, ofbx::Texture::TextureType::DIFFUSE);
					material->SetAlbedo(albedo);
					auto normalMap = material->GetNormalMap();
					SetMaterialTexture(&normalMap, fbxMaterial, fullPath, ofbx::Texture::TextureType::NORMAL);
					material->SetNormalMap(normalMap);

					material->Save();
					material->Load();

					material->p_hasBeenSent = true;
				}
				outputModel->m_materials.push_back(material);
			}

			const char* name = fbxMesh->name;
			const std::filesystem::path& meshFullPath = Resource::Mesh::CreateMeshPath(fullPath, name);
			Shared<Resource::Mesh> mesh = Resource::ResourceManager::GetResource<Resource::Mesh>(meshFullPath).lock();

			if (!mesh) {
				// If mesh not in resource manager
				mesh = Resource::ResourceManager::AddResource<Resource::Mesh>(meshFullPath).lock();
			}
			

			std::vector<float> finalVertices;
			std::vector<Vec3f> positions;

			auto fbxPositions = fbxMesh->getGeometryData().getPositions();
			auto fbxTextureUVs = fbxMesh->getGeometryData().getUVs();
			auto fbxNormals = fbxMesh->getGeometryData().getNormals();

			const auto pushToVector = [&](int index) {
				positions.push_back(ToVec3f(fbxPositions.get(index)));

				finalVertices.push_back(fbxPositions.get(index).x);
				finalVertices.push_back(fbxPositions.get(index).y);
				finalVertices.push_back(fbxPositions.get(index).z);

				finalVertices.push_back(fbxTextureUVs.get(index).x);
				finalVertices.push_back(fbxTextureUVs.get(index).y);

				finalVertices.push_back(fbxNormals.get(index).x);
				finalVertices.push_back(fbxNormals.get(index).y);
				finalVertices.push_back(fbxNormals.get(index).z);

				finalVertices.push_back(0);
				finalVertices.push_back(0);
				finalVertices.push_back(0);
				};

			//TODO : Handle tangents
			//TODO : Fix loading with mix of triangles and quads
			int totalVertexCount = 0;
			size_t totalVertexCountSub = 0;
			auto partitionCount = fbxMesh->getGeometryData().getPartitionCount();
			for (int j = 0; j < partitionCount; j++) {
				Resource::SubMesh subMesh;
				subMesh.startIndex = totalVertexCount;

				auto currentPartition = fbxMesh->getGeometryData().getPartition(j);
				int tris = currentPartition.triangles_count;

				for (size_t k = 0; k < currentPartition.polygon_count; k++) {
					auto currentPolygon = currentPartition.polygons[k];
					if (currentPolygon.vertex_count == 3) {
						for (int l = totalVertexCount; l < totalVertexCount + currentPolygon.vertex_count; l++) {
							pushToVector(l);
							totalVertexCountSub += 1;
						}
					}
					else if (currentPolygon.vertex_count == 4)
					{
						pushToVector(totalVertexCount);
						pushToVector(totalVertexCount + 1);
						pushToVector(totalVertexCount + 2);

						pushToVector(totalVertexCount + 2);
						pushToVector(totalVertexCount + 3);
						pushToVector(totalVertexCount);
						totalVertexCountSub += 6;
					}
					totalVertexCount += currentPolygon.vertex_count;
				}
				subMesh.count = totalVertexCountSub - subMesh.startIndex;
				mesh->m_subMeshes.push_back(subMesh);
			}

			allPositions.push_back(positions);

			mesh->m_finalVertices = finalVertices;

			mesh->p_shouldBeLoaded = true;
			mesh->p_loaded = true;

			outputModel->m_meshes.push_back(mesh);

			mesh->m_model = outputModel;

			auto bind = [outputModel] { outputModel->OnMeshLoaded(); };
			mesh->OnLoad.Bind(bind);

			mesh->SendRequest();
		}
		outputModel->ComputeBoundingBox(allPositions);
		outputModel->p_hasBeenSent = true;

	}
}
