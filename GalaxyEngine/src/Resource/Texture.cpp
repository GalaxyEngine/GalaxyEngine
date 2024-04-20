#include "pch.h"

#include "Resource/Texture.h"
#include "Resource/ResourceManager.h"

#include "Wrapper/ImageLoader.h"

#include "Core/Application.h"

Resource::Texture::~Texture()
{
	Wrapper::Renderer::GetInstance()->DestroyTexture(this);
	if (m_bytes != nullptr)
		Wrapper::ImageLoader::ImageFree(m_bytes);
}

void Resource::Texture::Load()
{
	if (p_shouldBeLoaded.load())
		return;
	p_shouldBeLoaded.store(true);
	StartLoading();

	if (p_fileInfo.GetExtension() == ".tmb") {
		this->SetDisplayOnInspector(false);
		this->SetCreateDataFile(false);
	}

	auto image = Wrapper::ImageLoader::Load(p_fileInfo.GetFullPath().string().c_str(), 4);
	if (m_bytes = std::move(image.data)) {
		p_loaded.store(true);
		m_size = image.size;
	}
	else
	{
		PrintError("Failed to load Image %s", p_fileInfo.GetFullPath().string().c_str());
		return;
	}

	if (!std::filesystem::exists(GetDataFilePath()))
		CreateDataFile();
	SendRequest();

	FinishLoading();
}

void Resource::Texture::Send()
{
	if (p_hasBeenSent)
		return;
	p_hasBeenSent.store(true);
	Wrapper::Renderer::GetInstance()->CreateTexture(this);
	Wrapper::ImageLoader::ImageFree(m_bytes);
	m_bytes = nullptr;
}

void Resource::Texture::Bind(const uint32_t index /* = 0 */)
{
	if (!p_hasBeenSent)
		return;
	Wrapper::Renderer::GetInstance()->BindTexture(this, index);
}

void Resource::Texture::UnBind()
{
	Wrapper::Renderer::GetInstance()->UnbindTexture();
}

void Resource::Texture::Serialize(CppSer::Serializer& serializer) const
{
	IResource::Serialize(serializer);
	serializer <<CppSer::Pair::BeginMap << "Texture";
	serializer << CppSer::Pair::Key << "Filtering" << CppSer::Pair::Value << (int)m_filtering;
	serializer << CppSer::Pair::Key << "Wrapping" << CppSer::Pair::Value << (int)m_wrapping;
	serializer <<CppSer::Pair::EndMap << "Texture";
}

void Resource::Texture::Deserialize(CppSer::Parser& parser)
{
	IResource::Deserialize(parser);
	parser.PushDepth();
	m_filtering = (TextureFiltering)parser["Filtering"].As<int>();
	m_wrapping = (TextureWrapping)parser["Wrapping"].As<int>();
}

void Resource::Texture::ShowInInspector()
{
	int filteringMode = (int)m_filtering;
	if (ImGui::Combo("Filtering", &filteringMode, SerializeTextureFilteringEnum()))
	{
		Wrapper::Renderer::GetInstance()->SetTextureFiltering(this, (TextureFiltering)filteringMode);
	}

	int wrappingMode = (int)m_wrapping;
	if (ImGui::Combo("Wrapping", &wrappingMode, SerializeTextureWrappingEnum()))
	{
		Wrapper::Renderer::GetInstance()->SetTextureWrapping(this, (TextureWrapping)wrappingMode);
	}

	if (ImGui::Button("Save"))
	{
		Save();
	}
}

void Resource::Texture::Save()
{
	if (!p_loaded)
		return;

	CreateDataFile();
}

void Resource::Texture::CreateWithData(const Path& path, const Wrapper::Image& image, TextureFiltering filtering /*= TextureFiltering::LINEAR*/, TextureFormat format /*= TextureFormat::RGBA*/)
{
	if (!image.data)
		return;
	Shared<Texture> texture = std::make_shared<Texture>(path);
	texture->p_shouldBeLoaded.store(true);
	texture->p_loaded.store(true);

	texture->m_bytes = image.data;
	texture->m_size = image.size;

	Resource::ResourceManager::AddResource(texture);
	
	texture->SendRequest();
}
