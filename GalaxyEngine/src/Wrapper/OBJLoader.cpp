#include "pch.h"
#include "Wrapper/OBJLoader.h"
#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Core/Application.h"

void Wrapper::OBJLoader::Load(const std::string& fullPath, Resource::Model* outputModel)
{
	//TODO
	OBJLoader model;
	model.m_path = fullPath;
	if (!model.Parse())
		return;
	for (int i = 0; i < model.m_meshes.size(); i++) {
		const std::string& meshFullPath = fullPath + ":" + model.m_meshes[i].name;
		auto mesh = std::make_shared<Resource::Mesh>(meshFullPath);
		mesh->p_name = model.m_meshes[i].name;
		mesh->p_type = Resource::ResourceType::Mesh;
		mesh->m_positions = model.m_meshes[i].positions;
		mesh->m_textureUVs = model.m_meshes[i].textureUVs;
		mesh->m_normals = model.m_meshes[i].normals;
		mesh->m_indices = model.m_meshes[i].indices;
		mesh->m_finalVertices = model.m_meshes[i].finalVertices;

		mesh->p_shouldBeLoaded = true;
		mesh->p_loaded = true;
		Resource::ResourceManager::GetInstance()->AddResource(mesh);
		outputModel->m_meshes.push_back(mesh);

		mesh->SendRequest();
	}
	PrintLog("Sucessfuly Loaded Model %s", fullPath.c_str());
}

bool Wrapper::OBJLoader::Parse()
{
	// TODO support :
	// quad
	// color
	// mtl
	// submeshes
	std::ifstream file(m_path);
	if (!file.is_open()) {
		PrintError("Failed to open OBJ file %s", m_path.c_str());
		return false;
	}

	bool quadOBJ = false;
	OBJMesh currentMesh;
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "o" || token == "g")
		{
			if (!currentMesh.name.empty()) {
				m_meshes.push_back(currentMesh);
			}
			currentMesh = OBJMesh();
			iss >> currentMesh.name;
		}
		if (token == "mtllib")
		{
			std::string mtlPath;
			iss >> mtlPath;
			mtlPath = m_path.substr(0, m_path.find_last_of('\\')) + '\\' + mtlPath;
			ReadMtl(mtlPath);
		}
		if (token == "v")
		{
			Vec3f position;
			iss >> position.x >> position.y >> position.z;
			currentMesh.positions.push_back(position);
		}
		else if (token == "vt")
		{
			Vec2f uv;
			iss >> uv.x >> uv.y;
			currentMesh.textureUVs.push_back(uv);
		}
		else if (token == "vn")
		{
			Vec3f normal;
			iss >> normal.x >> normal.y >> normal.z;
			
			currentMesh.normals.push_back(normal);
		}
		else if (token == "f")
		{
			char slash;
			std::string indexStr;
			while (iss >> indexStr) {
				Vec3i indices = ParseFaceIndex(indexStr);
				currentMesh.indices.push_back(indices);
			}

			// This will happen only one time by mesh, check if it's a quad model
			if (currentMesh.indices.size() == 4 && !quadOBJ)
			{
				quadOBJ = true;
			}
		}
	}
	if (!currentMesh.name.empty()) {
		m_meshes.push_back(currentMesh);
	}

	// Convert to Triangulate faces if it's a quad model
	if (quadOBJ)
	{
		std::vector<Vec3i> triangulateFace;

		for (size_t i = 0; i < m_meshes.size(); i++) {
			ConvertQuadToTriangles(m_meshes[i].indices, triangulateFace);
			m_meshes[i].indices = triangulateFace;
		}
	}


	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		ComputeVertices(m_meshes[i]);
	}

	return true;
}

Vec3i Wrapper::OBJLoader::ParseFaceIndex(const std::string& indexStr)
{
	Vec3i indices;
	std::istringstream indexStream(indexStr);
	std::string vertexIndexStr, uvIndexStr, normalIndexStr;

	std::getline(indexStream, vertexIndexStr, '/');
	std::getline(indexStream, uvIndexStr, '/');
	std::getline(indexStream, normalIndexStr, '/');

	indices.x = std::stoi(vertexIndexStr) - 1;   // OBJ indices start from 1
	// Case when no uvs
	try
	{
		indices.y = std::stoi(uvIndexStr) - 1;
	}
	catch (const std::exception&)
	{
		indices.y = -1;
	}
	// Case when no normals
	try
	{
		indices.z = std::stoi(normalIndexStr) - 1;
	}
	catch (const std::exception&)
	{
		indices.z = -1;
	}
	return indices;
}

bool AreVerticesSimilar(const Vec3f& v1, const Vec2f& uv1, const Vec3f& n1,
	const Vec3f& v2, const Vec2f& uv2, const Vec3f& n2)
{
	const float epsilon = 0.0001f;
	return (std::abs(v1.x - v2.x) < epsilon &&
		std::abs(v1.y - v2.y) < epsilon &&
		std::abs(v1.z - v2.z) < epsilon &&
		std::abs(uv1.x - uv2.x) < epsilon &&
		std::abs(uv1.y - uv2.y) < epsilon &&
		std::abs(n1.x - n2.x) < epsilon &&
		std::abs(n1.y - n2.y) < epsilon &&
		std::abs(n1.z - n2.z) < epsilon);
}

void Wrapper::OBJLoader::ComputeVertices(OBJMesh& mesh)
{

	for (size_t i = 0; i < mesh.indices.size(); i++)
	{
		const Vec3i& idx = mesh.indices[i];

		mesh.finalVertices.push_back(mesh.positions[idx.x].x);
		mesh.finalVertices.push_back(mesh.positions[idx.x].y);
		mesh.finalVertices.push_back(mesh.positions[idx.x].z);

		mesh.finalVertices.push_back(mesh.textureUVs[idx.y].x);
		mesh.finalVertices.push_back(mesh.textureUVs[idx.y].y);

		mesh.finalVertices.push_back(mesh.normals[idx.z].x);
		mesh.finalVertices.push_back(mesh.normals[idx.z].y);
		mesh.finalVertices.push_back(mesh.normals[idx.z].z);
	}
}

void Wrapper::OBJLoader::ConvertQuadToTriangles(const std::vector<Vec3i>& quadIndices, std::vector<Vec3i>& triangleIndices)
{

	if (quadIndices.size() % 4 != 0) {
		PrintError("The number of index is not valid");
		return;
	}

	const int numQuads = quadIndices.size() / 4;

	for (int i = 0; i < numQuads; ++i) {
		const int quadOffset = i * 4;

		// Convertit chaque quad en deux triangles
		triangleIndices.push_back(quadIndices[quadOffset]);
		triangleIndices.push_back(quadIndices[quadOffset + 1]);
		triangleIndices.push_back(quadIndices[quadOffset + 2]);

		triangleIndices.push_back(quadIndices[quadOffset]);
		triangleIndices.push_back(quadIndices[quadOffset + 2]);
		triangleIndices.push_back(quadIndices[quadOffset + 3]);
	}
}

bool Wrapper::OBJLoader::ReadMtl(const std::string& mtlPath)
{
	std::ifstream file(mtlPath);
	if (!file.is_open()) {
		PrintError("Failed to open MTL file %s", mtlPath.c_str());
		return false;
	}

	std::string line;
	std::shared_ptr<Resource::Material> currentMaterial;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;
		// Ambient
		if (token == "newmtl")
		{
			std::string name;
			iss >> name;
			const std::string& matFullPath = mtlPath.substr(0, mtlPath.find_last_of('\\') + 1) + name + ".mat";
			currentMaterial = std::make_shared<Resource::Material>(matFullPath);
			Resource::ResourceManager::GetInstance()->AddResource(currentMaterial);
		}
		if (token == "Ka")
		{
			Vec3f ambient;
			iss >> ambient.x >> ambient.y >> ambient.z;
			currentMaterial->m_ambient = ambient;
		}
		// Diffuse
		else if (token == "Kd")
		{
			Vec3f diffuse;
			iss >> diffuse.x >> diffuse.y >> diffuse.z;
			currentMaterial->m_diffuse = diffuse;
		}
		// Specular
		else if (token == "Ks")
		{
			Vec3f specular;
			iss >> specular.x >> specular.y >> specular.z;
			currentMaterial->m_specular = specular;
		}
		// Emissive
		else if (token == "d")
		{
			float transparency;
			iss >> transparency;
			currentMaterial->m_ambient.w = transparency;
			currentMaterial->m_diffuse.w = transparency;
			currentMaterial->m_specular.w = transparency;
		}
		else if (token == "map_Kd")
		{
			std::string texPath;
			iss >> texPath;
			texPath = mtlPath.substr(0, mtlPath.find_last_of('\\') + 1) + texPath;
			currentMaterial->m_albedo = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(texPath);
		}
	}
	return true;
}
