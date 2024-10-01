#include "pch.h"
#include "Editor/UI/FileExplorer.h"
#include "Editor/UI/EditorUIManager.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"

#include "Editor/ThumbnailCreator.h"
#include "Resource/Cubemap.h"

#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/Script.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Resource/Prefab.h"
#include "Resource/Sound.h"

#include "Wrapper/Window.h"
#include "Wrapper/ImageLoader.h"

#include "Utils/OS.h"

/* TODO:
* Drag and Drop (folders, models, ...)
* Specification Right Click : (Ex. Recompile Shader)
* Bottom bar too see clicked file
* All this need to work with multiple selected files
* Improve left child
 */

namespace GALAXY {
#pragma region File

	static std::unordered_map<Resource::ResourceType, uint32_t> s_resourceTypeToColor
	{
		{ Resource::ResourceType::Script, 0xFF5050FF},
		{ Resource::ResourceType::Material, 0xFF50FF50 },
		{ Resource::ResourceType::Model, 0xFFFF5050 },
		{ Resource::ResourceType::Texture, 0xFF00CCFF },
		{ Resource::ResourceType::Shader, 0xFFFFCC00 },
		{ Resource::ResourceType::VertexShader, 0xFF00FFCC },
		{ Resource::ResourceType::FragmentShader, 0xFFCC00FF },
		{ Resource::ResourceType::Sound, 0xFFCCFF00 },


	};

	uint32_t Editor::UI::File::ResourceTypeToColor(Resource::ResourceType type)
	{
		if (s_resourceTypeToColor.find(type) == s_resourceTypeToColor.end())
			return 0xFFFFFFFF;
		return s_resourceTypeToColor[type];
	}

	Editor::UI::File::File(const Path& path, bool filewatch /*= false*/)
	{
		m_fileWatch = filewatch;
		if (!m_fileWatch)
			m_resource = Resource::ResourceManager::GetResource<Resource::IResource>(path);

		if (m_resource.lock())
			m_info = m_resource.lock()->GetFileInfo();
		else
			m_info = Utils::FileInfo(path);

		if (m_fileWatch)
			return;
		if (m_info.isDirectory())
		{
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(FOLDER_ICON_PATH);
			return;
		}
		using namespace Resource;
		switch (m_info.GetResourceType()) {
		case ResourceType::Texture:
		{
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(path);
			break;
		}
		case ResourceType::Shader:
		{
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(SHADER_ICON_PATH);
			break;
		}
		/*
		case ResourceType::VertexShader:
		{
			break;
		}
		case ResourceType::GeometryShader:
		{
			break;
		}
		case ResourceType::FragmentShader:
		{
			break;
		}
		*/
		case ResourceType::Model:
		{
			Shared<Resource::IResource> model = m_resource.lock();
			ASSERT(model != nullptr);
			auto thumbnailPath = Editor::ThumbnailCreator::GetThumbnailPath(model);
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(thumbnailPath);
			break;
		}
		case ResourceType::Script:
		{
			if (m_info.GetExtension() == ".cpp")
				m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(SCRIPT_CPP_ICON_PATH);
			else
				m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(SCRIPT_H_ICON_PATH);
			break;
		}
		case ResourceType::Material:
		{
			Shared<Resource::IResource> material = m_resource.lock();
			ASSERT(material != nullptr);
			auto thumbnailPath = Editor::ThumbnailCreator::GetThumbnailPath(material);
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(thumbnailPath);
			break;
		}
		case ResourceType::Sound:
		{
			// m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(SOUND_ICON_PATH);
			break;
		}
		case ResourceType::Cubemap:
			{
				//TODO Add icon to cubemap resource, draw a cube with the cubemap textures
				break;
			}
		default:
		{
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(FILE_ICON_PATH);
			break;
		}
		}
	}

	Editor::UI::File::~File()
	{
		for (auto& child : m_children)
		{
			child->m_parent.reset();
		}
		m_children.clear();
		m_children.shrink_to_fit();

		m_parent.reset();
	}

	void Editor::UI::File::FindChildren()
	{
		const auto dirIt = std::filesystem::directory_iterator(m_info.GetFullPath());
		for (const std::filesystem::directory_entry& entry : dirIt)
		{
			m_children.push_back(std::make_shared<File>(entry.path()));
			m_children.back()->m_parent = shared_from_this();
			if (entry.is_directory()) {
				m_isAnyChildFolder = true;
			}
		}
	}

	void Editor::UI::File::FindAllChildren()
	{
		if (!std::filesystem::exists(m_info.GetFullPath()))
		{
			PrintLog("%s does not exist, maybe because of no project path", m_info.GetFullPath().string().c_str());
			return;
		}
		const auto dirIt = std::filesystem::directory_iterator(m_info.GetFullPath());
		for (const std::filesystem::directory_entry& entry : dirIt)
		{
			m_children.push_back(std::make_shared<File>(entry.path(), m_fileWatch));
			m_children.back()->m_parent = weak_from_this();
			if (entry.is_directory()) {
				m_isAnyChildFolder = true;
				m_children.back()->FindAllChildren();
			}
		}
	}

	void Editor::UI::File::DisplayOnExplorer()
	{
		FileExplorer* explorer = EditorUIManager::GetInstance()->GetFileExplorer();
		if (m_info.isDirectory())
		{
			ImGui::BeginGroup();
			if (m_icon.lock()) {
				Wrapper::GUI::TextureImage(m_icon.lock().get(), Vec2f(16));
				ImGui::SameLine();
			}
			bool clicked = false;
			if (ImGui::TreeNodeEx(m_info.GetFileName().c_str(), (m_isAnyChildFolder ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf) | ImGuiTreeNodeFlags_OpenOnArrow))
			{
				if (m_wasOpen)
					clicked = ImGui::IsItemClicked();
				for (const Shared<File>& children : m_children)
				{
					children->DisplayOnExplorer();
				}
				m_wasOpen = true;
			}
			else
			{
				m_wasOpen = false;
				clicked = ImGui::IsItemClicked();
			}

			// When the tree node is clicked, set the directory of the file explorer to the folder
			if (clicked)
			{
				Path directory = GetPath();
				explorer->SetDirectory(directory);
			}
			
			
			ImGui::EndGroup();

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {

					auto draggedFiles = explorer->m_draggedFiles;
					for (int i = 0; i < draggedFiles.size(); ++i) {

						Path oldPath = draggedFiles[i]->m_info.GetFullPath();

						if (oldPath == m_info.GetFullPath())
							continue;

						Path newPath = m_info.GetFullPath() / oldPath.filename();

						// Rename Resource in resourceManager and file explorer
						Resource::ResourceManager::HandleRename(oldPath, newPath);
					}
					explorer->ReloadContent();
				}
				ImGui::EndDragDropTarget();
			}

		}
	}

	void Editor::UI::File::AddChild(Shared<File> file)
	{
		m_children.push_back(file);
	}

	Shared<Editor::UI::File> Editor::UI::File::GetWithPath(const Path& path, bool matchCase) const
	{
		using namespace Utils::FileSystem;
		for (const Shared<File>& child : m_children)
		{
			Path fullPath = child->m_info.GetFullPath();
			if (fullPath == path || !matchCase && ToLower(path) == ToLower(fullPath))
				return child;
			if (Shared<File> result = child->GetWithPath(path, matchCase))
				return result;
		}
		return nullptr;
	}

	List<Shared<Editor::UI::File>> Editor::UI::File::GetAllChildren()
	{
		List<Shared<Editor::UI::File>> childList;
		for (auto& children : m_children)
		{
			auto list = children->GetAllChildren();
			childList.push_back(children);
			childList.insert(childList.end(), list.begin(), list.end());
		}
		return childList;
	}

	List<Path> Editor::UI::File::GetAllChildrenPath()
	{
		List<Path> childList;
		for (auto& children : m_children)
		{
			auto list = children->GetAllChildrenPath();
			childList.push_back(children->m_info.GetFullPath());
			childList.insert(childList.end(), list.begin(), list.end());
		}
		return childList;
	}

#pragma endregion

	void Editor::UI::FileExplorer::Initialize()
	{
		m_workingDirectory = Resource::ResourceManager::GetInstance()->GetAssetPath();
		m_mainFile = std::make_shared<File>(m_workingDirectory);
		m_mainFile->FindAllChildren();

		m_currentFile = m_mainFile;

		m_iconSize = m_iconSize * Wrapper::GUI::GetScaleFactor();

		EditorUIManager::GetInstance()->GetInspector()->SetFileSelected(&m_selectedFiles);
	}
	
	void Editor::UI::FileExplorer::SetDirectory(const Path& directory, bool matchCase)
	{
		Shared<File> file;
		Path fullPath = m_mainFile->m_info.GetFullPath();
		if (fullPath == directory || !matchCase && Utils::FileSystem::ToLower(directory) == Utils::FileSystem::ToLower(fullPath))
			file =  m_mainFile;
		if (Shared<File> result = m_mainFile->GetWithPath(directory, matchCase))
			file = result;
		
		if (!file)
			return;
		SetCurrentFile(file);
	}

	void Editor::UI::FileExplorer::NavigateToFile(const Path& filePath)
	{
		SetDirectory(filePath.parent_path(), true);
		if (auto file = m_mainFile->GetWithPath(filePath))
			AddFileSelected(file);
	}

	void Editor::UI::FileExplorer::Draw()
	{
		if (!p_open)
			return;

		constexpr float minIconSize = 0.5f;
		constexpr float maxIconSize = 5.f;
		constexpr float iconDeltaZoom = 0.1f;
		bool openRightClick = false;

		// Begin the ImGui window for the File Explorer
		if (m_visible = ImGui::Begin("File Explorer", &p_open); m_visible) {
			const Vec2f vMin = ImGui::GetWindowContentRegionMin();
			const Vec2f vMax = ImGui::GetWindowContentRegionMax();
			const Vec2f windowPos = Vec2f(ImGui::GetWindowPos());

			m_rect.min = vMin + windowPos - Core::Application::GetInstance().GetWindow()->GetPosition();
			m_rect.max = vMax + windowPos - Core::Application::GetInstance().GetWindow()->GetPosition();

			static float size1 = 200, size2 = ImGui::GetContentRegionAvail().x;
			Wrapper::GUI::Splitter(true, 2, &size1, &size2, 10, 10);

			ImGui::BeginChild("Folder", Vec2f(size1, ImGui::GetContentRegionAvail().y));
			m_mainFile->DisplayOnExplorer();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Content", Vec2f(ImGui::GetContentRegionAvail().x, -1));

			DrawTopExplorer();

			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4f(0, 0, 100, 255));
			ImGui::BeginChild("Thumbnails");
			// Iterate through each child of the current file
			for (int i = 0, x = 0, y = 0; i < m_currentFile->m_children.size(); i++) {
				Shared<File>& child = m_currentFile->m_children[i];
				Utils::FileInfo& info = child->m_info;
				if (!child || info.GetResourceType() == Resource::ResourceType::Data) {
					continue;
				}

				ImGui::PushID(static_cast<int>(i));

				auto cursorPos = ImGui::GetCursorPos();

				bool shouldBreak = false;
				DrawThumbnail(child, i, m_iconSize, x, y, shouldBreak, openRightClick);
				if (shouldBreak)
					break;
				ImGui::PopID();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor(2);

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS")) {
					// Check if the payload data type matches
					List<uint64_t> indices;
					if (payload->DataSize % sizeof(uint64_t) == 0)
					{
						uint64_t* payloadData = static_cast<uint64_t*>(payload->Data);
						const uint64_t payloadSize = payload->DataSize / sizeof(uint64_t);
						indices.assign(payloadData, payloadData + payloadSize);
					}
					for (size_t i = 0; i < indices.size(); i++) {
						Weak<Core::GameObject> payloadGameObject;
						payloadGameObject = Core::SceneHolder::GetCurrentScene()->GetWithSceneGraphID(indices[i]);

						Resource::Prefab::CreateWith(m_currentFile->m_info.GetFullPath() / (payloadGameObject.lock()->GetName() + ".prefab"), payloadGameObject.lock());
					}
				}
				ImGui::EndDragDropTarget();
			}
			
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::GetIO().MouseWheel != 0)
			{
				m_iconSize += ImGui::GetIO().MouseWheel * iconDeltaZoom;
				m_iconSize = std::clamp(m_iconSize, minIconSize, maxIconSize);
			}
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !openRightClick || openRightClick)
			{
				if (!openRightClick) {
					ClearSelected();
					m_rightClickedFiles.clear();
				}
				ImGui::OpenPopup("RightClickPopup");
			}
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(0) && ImGui::GetHoveredID() == 0)
				ClearSelected();
			RightClickWindow();
			ImGui::EndChild();
			UpdateReloadContent();
		}

		ImGui::End();

	}

	void Editor::UI::FileExplorer::DrawTopExplorer()
	{
		static bool wasSearching = false;
		static bool isSearching = false;
		static bool shouldFocus = false;
		bool isSelected = false;
		if (ImGui::Button("Back") && m_currentFile->m_parent.lock())
		{
			Shared<File> parent = m_currentFile->m_parent.lock();
			SetCurrentFile(parent);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload"))
		{
			ReloadContent();
		}
		ImGui::SameLine();
		Vec2f lastPos = ImGui::GetCursorPos();
		std::string in = isSearching ? m_currentFile->m_info.GetRelativePath().string() : "";
		if (!isSearching) {
			ImGui::SetNextItemAllowOverlap();
			ImGui::Button("##SearchButton", Vec2f(ImGui::CalcItemWidth(), 0));
			bool isClicked = ImGui::IsItemClicked();
			ImGui::SameLine();
			ImGui::TextUnformatted("Folder path");
			if (isClicked)
			{
				isSearching = true;
				shouldFocus = true;
			}
		}
		else if (isSearching)
		{
			if (shouldFocus)
			{
				ImGui::SetKeyboardFocusHere();
				shouldFocus = false;
			}
			ImGui::SetNextItemAllowOverlap();
			if (Wrapper::GUI::InputText("Folder path", &in, ImGuiInputTextFlags_EnterReturnsTrue)) {
				in = (Resource::ResourceManager::GetProjectPath() / in).generic_string();
				if (std::filesystem::exists(in) && std::filesystem::is_directory(in))
				{
					Path absAssetPath = std::filesystem::absolute(Resource::ResourceManager::GetAssetPath());
					Path absNewPath = std::filesystem::absolute(in);
					const bool isSubdirectory = std::mismatch(absAssetPath.begin(), absAssetPath.end(), absNewPath.begin()).first == absAssetPath.end();
					if (isSubdirectory)
					{
						SetDirectory(in, false);
					}
				}
			}
			isSelected = ImGui::IsItemActive();
			if (!wasSearching && isSearching)
			{
				isSearching = true;
			}
			else
			{
				isSearching = isSelected;
			}
			wasSearching = isSearching;
		}
		Vec2f afterPos = ImGui::GetCursorPos();
		if (!isSelected)
		{
			ImGui::SetCursorPos(lastPos);
			// Separate path into components
			std::vector<std::pair<std::string /*real name*/, std::string /*display name*/>> components;
			std::string path = m_currentFile->m_info.GetRelativePath().string();
			std::stringstream ss(path);
			std::string component;
			const int maxWordLength = 15;
			while (std::getline(ss, component, '\\')) {
				components.push_back(std::make_pair(component, component.length() > maxWordLength ?
					component.substr(0, maxWordLength - 3) + "..." : component));
			}

			float currentSize = 0;
			float itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
			int cutAt = 0;
			float diff;
			for (int i = static_cast<uint32_t>(components.size() - 1); i >= 0; i--)
			{
				Vec2f textSize = ImGui::CalcTextSize(components[i].second.c_str());
				float buttonSize = textSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;
				currentSize += buttonSize;
				if (i != components.size() - 1 && currentSize > ImGui::GetContentRegionAvail().x)
				{
					cutAt = i;
					currentSize -= buttonSize;
					break;
				}
				currentSize += itemSpacingX + ImGui::GetFrameHeight() + itemSpacingX * 2.5f;

				if (currentSize > ImGui::GetContentRegionAvail().x)
				{
					diff = currentSize - ImGui::GetContentRegionAvail().x;
					cutAt = i;
					break;
				}
			}

			for (int i = cutAt; i < components.size(); i++)
			{
				if (i > 0)
				{
					const float spacingX = itemSpacingX;
					if (i != cutAt) {

						ImGui::SameLine();
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - spacingX);
					}
					ImGui::ArrowButtonEx("##" + i, ImGuiDir_Right, Vec2f(ImGui::GetFrameHeight() + spacingX * 2.5f, ImGui::GetFrameHeight()));
					ImGui::SameLine();
					if (i != cutAt) {

						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - spacingX);
					}
				}
				if (ImGui::Button(components[i].second.c_str()))
				{
					// add before components to the path
					Path finalPath = Resource::ResourceManager::GetProjectPath();
					for (int j = 0; j != i + 1; j++) {
						finalPath /= components[j].first;
					}
					SetDirectory(finalPath);
				}
			}
		}
		ImGui::SetCursorPos(afterPos);
	}

	void Editor::UI::FileExplorer::AddFileSelected(const Shared<File>& child)
	{
		if (std::ranges::count(m_selectedFiles, child) > 0)
			return;
		child->m_selected = true;
		m_selectedFiles.push_back(child);
		EditorUIManager::GetInstance()->GetInspector()->UpdateFileSelected();
	}

	void Editor::UI::FileExplorer::RemoveFileSelected(const Shared<File>& child)
	{
		for (size_t i = 0; i < m_selectedFiles.size(); i++) {
			if (m_selectedFiles[i] == child) {
				m_selectedFiles.erase(m_selectedFiles.begin() + i);
				child->m_selected = false;
				break;
			}
		}
		EditorUIManager::GetInstance()->GetInspector()->UpdateFileSelected();
	}

	void Editor::UI::FileExplorer::SelectRange(size_t startIndex, size_t endIndex)
	{
		if (startIndex > endIndex)
			std::swap(startIndex, endIndex);
		const auto children = m_currentFile->GetChildren();
		for (size_t i = startIndex; i < endIndex + 1; i++)
		{
			AddFileSelected(children[i]);
		}
		
	}

	void Editor::UI::FileExplorer::HandleDropFile(const int count, const char** paths) const
	{
		auto window = Core::Application::GetInstance().GetWindow();
		if (!p_open || !m_visible)
			return;

		const Vec2f mousePos = Input::GetMousePosition();

		if (!m_rect.IsPointInside(mousePos))
			return;

		for (size_t i = 0; i < count; i++)
		{
			PrintLog("Dropped file: %s", paths[i]);

			std::filesystem::path fromPath = Path(paths[i]);
			auto toPath = this->m_currentFile->m_info.GetFullPath() / fromPath.filename();
			std::filesystem::copy(fromPath, toPath);
		}
	}

	void Editor::UI::FileExplorer::ClearSelected()
	{
		for (const auto& selectedFile : m_selectedFiles)
		{
			selectedFile->m_selected = false;
		}
		m_selectedFiles.clear();
		EditorUIManager::GetInstance()->GetInspector()->UpdateFileSelected();
	}

	void Editor::UI::FileExplorer::SetCurrentFile(const Shared<File>& file)
	{
		m_currentFile = file;
		ClearSelected();
	}

	void Editor::UI::FileExplorer::RightClickWindow()
	{
		using namespace Resource;
		if (m_rightClickOpen = ImGui::BeginPopup("RightClickPopup"))
		{
			static auto quitPopup = [this]()
				{
					m_rightClickedFiles.clear();
					ImGui::CloseCurrentPopup();
				};

			Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
			if (!m_rightClickedFiles.empty()) {
				ImGui::TextUnformatted(m_rightClickedFiles[0]->m_info.GetFileName().c_str());
				buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
				if (m_rightClickedFiles[0]->m_resource.lock()) {
					bool allSame = true;
					bool allShader = true;
					const Resource::ResourceType commonType = m_rightClickedFiles[0]->m_resource.lock()->GetFileInfo().GetResourceType();

					for (const Shared<File>& file : m_rightClickedFiles)
					{
						auto resourceType = file->m_resource.lock()->GetFileInfo().GetResourceType();
						if (resourceType != ResourceType::FragmentShader && resourceType != ResourceType::VertexShader)
						{
							allShader = false;
						}
						if (!file->m_resource.lock() || commonType != resourceType)
						{
							allSame = false;
						}
					}
					if (allSame || allShader) {
						switch (commonType)
						{
						case ResourceType::Texture:
							//TODO : Move this inside the cubemap inspector, or other 
							if (ImGui::Button("Convert to six sided", buttonSize))
							{
								Wrapper::ImageLoader::ExtractSixSidedFromCubemap(m_rightClickedFiles[0]->m_info.GetFullPath());
								quitPopup();
							}
							//TODO : Remove this (Debug)
							if (ImGui::Button("Show In popup", buttonSize))
							{
								auto texture = m_rightClickedFiles[0]->GetResource<Resource::Texture>().get();
								Utils::OS::DisplayImageInPopup(texture);
								quitPopup();
							}
							break;
						case ResourceType::Shader:
							if (ImGui::Button("Edit", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									const auto shader = ResourceManager::GetResource<Shader>(file->m_info.GetFullPath()).lock();

									if (const auto vertex = shader->GetVertex().lock())
										Utils::OS::OpenWithVSCode(vertex->GetFileInfo().GetFullPath());
									if (const auto geom = shader->GetGeometry().lock())
										Utils::OS::OpenWithVSCode(geom->GetFileInfo().GetFullPath());
									if (const auto frag = shader->GetFragment().lock())
										Utils::OS::OpenWithVSCode(frag->GetFileInfo().GetFullPath());
								}
								quitPopup();
							}
							if (ImGui::Button("Recompile", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									dynamic_pointer_cast<Shader>(file->m_resource.lock())->Recompile();
								}
								quitPopup();
							}
							break;
						case ResourceType::FragmentShader:
						case ResourceType::GeometryShader:
						case ResourceType::VertexShader:
							if (ImGui::Button("Edit", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									Utils::OS::OpenWithVSCode(file->m_info.GetFullPath());
								}
								quitPopup();
							}
							if (ImGui::Button("Recompile", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									if (!file->m_resource.lock() || !file->m_resource.lock()->ShouldBeLoaded())
										ResourceManager::GetOrLoad<Shader>(file->m_info.GetFullPath());
									else
										dynamic_pointer_cast<BaseShader>(file->m_resource.lock())->Recompile();
								}
								quitPopup();
							}
							break;
						case ResourceType::Model:
							{
								if (m_rightClickedFiles.size() > 1)
									break;
								auto isLoaded =  m_rightClickedFiles[0]->m_resource.lock()->IsLoaded();
								if (!isLoaded && ImGui::Button("Load", buttonSize))
								{
									for (const Shared<File>& file : m_rightClickedFiles)
									{
										ResourceManager::GetOrLoad<Model>(file->m_info.GetFullPath());
									}
								}
							
								if (isLoaded && ImGui::Button("Reload Thumbnail"))
								{
									for (const Shared<File>& file : m_rightClickedFiles)
									{
										auto model = ResourceManager::GetOrLoad<Model>(file->m_info.GetFullPath());
										if (model.lock())
										{
											model.lock()->CreateThumbnail();
										}
									}
								}
								break;
							}
						case ResourceType::Mesh:
							break;
						case ResourceType::Material:
							{
								bool isLoaded = m_rightClickedFiles[0]->m_resource.lock()->IsLoaded();
								if (!isLoaded && ImGui::Button("Load", buttonSize))
								{
									for (const Shared<File>& file : m_rightClickedFiles)
									{
										ResourceManager::GetOrLoad<Material>(file->m_info.GetFullPath());
									}
								}
								if (isLoaded && ImGui::Button("Reload Thumbnail", buttonSize))
								{
									auto thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();
									thumbnailCreator->AddToQueue(m_rightClickedFiles[0]->m_resource);
									quitPopup();
								}
								break;
							}
						case ResourceType::Cubemap:
							if (ImGui::Button("Load", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									ResourceManager::GetOrLoad<Cubemap>(file->m_info.GetFullPath());
								}
							}
							break;
						case ResourceType::Sound:
							if (ImGui::Button("Load", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									ResourceManager::GetOrLoad<Resource::Sound>(file->m_info.GetFullPath());
								}
							}
							break;
						case ResourceType::Script:
							if (ImGui::Button("Edit", buttonSize))
							{
								Script::OpenScript(m_rightClickedFiles[0]->m_info.GetFullPath());
							}
							break;
						default:
							break;
						}
						ImGui::Separator();
					}
				}
				if (ImGui::Button("Rename", buttonSize))
				{
					SetRenameFile(m_rightClickedFiles[0]);

					quitPopup();
				}
				ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
				if (ImGui::Button("Delete", buttonSize))
				{
					for (const Shared<File>& file : m_rightClickedFiles)
					{
						// Remove Resource file
						Resource::ResourceManager::RemoveResource(file->m_info.GetFullPath());
						std::filesystem::remove(file->m_info.GetFullPath().string().c_str());
						std::filesystem::remove((file->m_info.GetFullPath().string() + ".gdata").c_str());
					}
					m_rightClickedFiles.clear();
					ReloadContent();
					quitPopup();
				}
				ImGui::PopStyleColor();
				ImGui::Separator();

			}
			if (ImGui::Button("Show In Explorer", buttonSize))
			{
				const Path folderPath = m_currentFile->m_info.GetFullPath();

				std::vector<std::string> filesName(m_rightClickedFiles.size());
				for (const Shared<File>& file : m_rightClickedFiles)
					filesName.push_back(file->m_info.GetFileName());
				
				Utils::OS::ShowInExplorer(folderPath, filesName);
				quitPopup();
			}
			if (ImGui::Button("New Folder", buttonSize))
			{
				const auto folderPath = m_currentFile->m_info.GetFullPath() / "New Folder";
				std::filesystem::create_directory(folderPath);
				const Shared<File> file = std::make_shared<File>(folderPath);
				m_currentFile->AddChild(file);
				SetRenameFile(file);

				quitPopup();
			}
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::Button("Script", buttonSize))
				{
					ImGui::OpenPopup("Create Script");
				}
				if (ImGui::BeginPopupModal("Create Script"))
				{
					static std::string scriptName;
					Wrapper::GUI::InputText("Script Name", &scriptName);
					if (ImGui::Button("Create") && !scriptName.empty())
					{
						Resource::Script::Create(m_currentFile->m_info.GetFullPath() / scriptName);
						ReloadContent();
						quitPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						quitPopup();
					}
					ImGui::EndPopup();
				}
				if (ImGui::Button("Material", buttonSize))
				{
					const auto materialPath = m_currentFile->m_info.GetFullPath() / "New Material.mat";
					Resource::Material::Create(materialPath);
					const Shared<File> file = std::make_shared<File>(materialPath);
					m_currentFile->AddChild(file);
					SetRenameFile(file);

					quitPopup();
				}
				if (ImGui::Button("Shader", buttonSize))
				{
					ImGui::OpenPopup("Create Shader");
				}
				if (ImGui::BeginPopupModal("Create Shader"))
				{
					static std::string shaderName;
					Wrapper::GUI::InputText("	Shader Name", &shaderName);
					if (ImGui::Button("Create") && !shaderName.empty())
					{
						Resource::Shader::Create(m_currentFile->m_info.GetFullPath() / shaderName);
						ReloadContent();
						ImGui::CloseCurrentPopup();
						quitPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						quitPopup();
					}
					ImGui::EndPopup();
				}
				if (ImGui::Button("Cubemap", buttonSize))
				{
					const auto materialPath = m_currentFile->m_info.GetFullPath() / "New Cubemap.cubemap";
					Resource::Cubemap::Create(materialPath);
					const Shared<File> file = std::make_shared<File>(materialPath);
					m_currentFile->AddChild(file);
					SetRenameFile(file);

					quitPopup();
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

	void Editor::UI::FileExplorer::SetRenameFile(const Shared<File>& file)
	{
		if (m_renameFile)
			m_renameFile->m_rename = false;
		m_renameFile = file;
		ClearSelected();
		m_renameFile->m_rename = true;
		m_openRename = true;
		m_renameFileName = file->m_info.GetFileNameNoExtension();
	}

	void Editor::UI::FileExplorer::ReloadContent()
	{
		m_shouldReloadContent = true;
	}

	void Editor::UI::FileExplorer::UpdateReloadContent()
	{
		if (!m_shouldReloadContent)
			return;
		m_shouldReloadContent = false;
		const auto currentPath = m_currentFile->m_info.GetFullPath();
		m_mainFile->m_children.clear();
		m_mainFile->FindAllChildren();
		m_currentFile = m_mainFile->GetWithPath(currentPath);
		if (!m_currentFile)
			m_currentFile = m_mainFile;
	}

	void Editor::UI::FileExplorer::DragAndDrop(Shared<File>& child)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			m_draggedFiles.clear();
			if (!child->m_selected)
				m_draggedFiles.push_back(child);
			for (int i = 0; i < m_selectedFiles.size(); ++i) {
				m_draggedFiles.push_back(m_selectedFiles[i]);
			}
			for (int i = 0; i < m_draggedFiles.size(); i++)
			{
				ImGui::TextUnformatted(m_draggedFiles[i]->m_info.GetFileName().c_str());
			}
			ImGui::SetDragDropPayload("FILE", nullptr, 0);
			ImGui::EndDragDropSource();
		}
		if (child->m_info.isDirectory())
		{
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {

					for (int i = 0; i < m_draggedFiles.size(); ++i) {
						Path oldPath = m_draggedFiles[i]->m_info.GetFullPath();
						Path newPath = child->m_info.GetFullPath() / oldPath.filename();

						// Rename Resource in resourceManager
						Resource::ResourceManager::HandleRename(oldPath, newPath);
					}
					ReloadContent();
				}
				ImGui::EndDragDropTarget();
			}
		}
	}

	void Editor::UI::FileExplorer::DrawThumbnail(Shared<File>& file, int index, float thumbnailScale, int& x, int& y, bool& shouldBreak, bool& openRightClick)
	{
		Vec2f windowSize = ImGui::GetWindowSize();
		Vec2f windowPos = ImGui::GetWindowPos();
		Vec2f regMin = ImGui::GetWindowContentRegionMin();
		Vec2f thumbnailOffset = Vec2f(150, 225) * thumbnailScale;

		// Common offsets and sizes
		constexpr Vec2f shadowOffset = Vec2f(6.0f, 5.5f);
		constexpr Vec2f imageSize = Vec2f(128.f);
		constexpr Vec2f shadowSize = Vec2f(134.0f, 210.5f);
		constexpr float bottomRectHeight = 204.5f;
		constexpr float lineHeight = 1.5f;
		constexpr int textLength = 20;

		constexpr float cornerRounding = 7.5f;
		constexpr ImU32 clickedColor = 0x80FF9933;
		constexpr ImU32 hoveredColor = 0xFF363636;
		constexpr ImU32 folderThumbColor = 0x00000000;

		const bool isFolder = file->m_info.isDirectory();

		// Common positions
		Vec2f currentPos = Vec2i(x, y) * thumbnailOffset;
		Vec2f contentMin = regMin + currentPos;
		const Vec2f rectSize = imageSize * thumbnailScale;
		if (contentMin.x >= windowSize.x - thumbnailOffset.x + 25.f) {
			y++;
			x = 0;

			currentPos = Vec2i(x, y) * thumbnailOffset;
			contentMin.x = regMin.x;
			contentMin.y += thumbnailOffset.y;
		}
		Vec2f contentRegionStart = windowPos + contentMin;

		// All the rect:
		// Thumbnail image with rounded corners
		Vec2f thumbnailMin = contentRegionStart;
		Vec2f thumbnailMax = contentRegionStart + imageSize * thumbnailScale;

		// Bottom rectangle under the thumbnail
		Vec2f bottomRectMin = contentRegionStart + Vec2f(0, (imageSize.y + lineHeight) * thumbnailScale);
		Vec2f bottomRectMax = contentRegionStart + Vec2f(imageSize.x, bottomRectHeight) * thumbnailScale;

		Vec2f topThumbnail = thumbnailMin - ImGui::GetWindowPos() + Vec2f(0, ImGui::GetScrollY());
		Vec2f buttonSize = bottomRectMax - thumbnailMin;

		ImGui::SetCursorPos(topThumbnail);
		ImGui::InvisibleButton("##", buttonSize);

		DragAndDrop(file);

		x++;
		if (!ImGui::IsRectVisible(thumbnailMin, thumbnailMax + Vec2f(0, 75) * thumbnailScale)) {
			return;
		}

		// Shadow behind the thumbnail
		Vec2f shadowMin = contentRegionStart + shadowOffset;
		Vec2f shadowMax = contentRegionStart + shadowSize * thumbnailScale;

		// Line under the thumbnail
		Vec2f lineMin = contentRegionStart + Vec2f(0, imageSize.y * thumbnailScale);
		Vec2f lineMax = contentRegionStart + Vec2f(imageSize.x, imageSize.y + lineHeight) * thumbnailScale;

		// Draw list pointer
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		std::shared_ptr<Resource::Texture> texture = file->m_icon.lock();
		const ImTextureID textureID = texture ? Wrapper::GUI::GetTextureID(texture.get()) : reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(0));
		const bool shouldDrawImage = texture && texture->IsLoaded() && texture->HasBeenSent();
		if (!isFolder || file->m_selected || file->m_hovered) {
			// Draw Shadow behind the thumbnail
			drawList->AddRectFilled(shadowMin, shadowMax, 1677721600, cornerRounding, 240);

			ImU32 color = file->m_selected ? clickedColor : file->m_hovered ? hoveredColor : 0xFF1A1A1A;
			// Draw Thumbnail background
			drawList->AddRectFilled(thumbnailMin, thumbnailMax, color, cornerRounding, 48);
			// Draw Thumbnail image with rounded corners
			if (shouldDrawImage)
				drawList->AddImageRounded(textureID, thumbnailMin, thumbnailMax, Vec2f(0, 0), Vec2f(1, 1), IM_COL32_WHITE, cornerRounding, 48);

			color = isFolder ? (file->m_selected ? clickedColor : hoveredColor) : File::ResourceTypeToColor(file->m_info.GetResourceType());
			// Line under the thumbnail
			drawList->AddRectFilled(lineMin, lineMax, color); // No rounding

			// Bottom rectangle under the thumbnail
			drawList->AddRectFilled(bottomRectMin, bottomRectMax, file->m_selected ? clickedColor : file->m_hovered ? hoveredColor : 4279834905, cornerRounding, 192);
		}
		else
		{
			if (shouldDrawImage)
				drawList->AddImageRounded(textureID, thumbnailMin, thumbnailMax, Vec2f(0, 0), Vec2f(1, 1), IM_COL32_WHITE, cornerRounding, 48);
		}

		

		ImFont* font = ImGui::GetIO().Fonts->Fonts[0];
		//Content
		//Resource Name
		if (!file->m_rename)
		{
			std::string name = file->m_info.GetFileName();
			const Vec2f textSize = font->CalcTextSizeA(13 * thumbnailScale, FLT_MAX, 0, name.c_str());
			std::string tempName;
			float maxX = contentRegionStart.x + (imageSize.x - 10.f) * thumbnailScale;
			float minX = contentRegionStart.x;
			const float maxWidth = maxX  - minX; 
			for (int i = 0; i < name.length(); i++) {
				tempName.push_back(name[i]);
				if (font->CalcTextSizeA(13 * thumbnailScale, FLT_MAX, 0, (tempName + "...").c_str()).x > maxWidth) {
					tempName.pop_back();
					name = tempName + "...";
					break;
				}
				if (i == name.length() - 1)
					name = tempName;
			}
			ImGui::PushID(779144781);
			Vec2f minTitle = Vec2f(contentRegionStart.x + 5.f * thumbnailScale, lineMax.y + 5.f * thumbnailScale);
			Vec2f maxTitle = Vec2f(contentRegionStart.x + 5.f * thumbnailScale, lineMax.y + 5.f * thumbnailScale) + font->CalcTextSizeA(13 * thumbnailScale, FLT_MAX, 0, name.c_str()) * thumbnailScale;
			drawList->AddText(font, 13 * thumbnailScale, minTitle, 0xFFFFFFFF, name.c_str());
			if (ImGui::IsMouseHoveringRect(minTitle, maxTitle) && ImGui::IsMouseClicked(0))
				SetRenameFile(file);
			ImGui::PopID();
		}

		//Resource Type
		ImGui::PushID(705220297);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.622549f, 0.622549f, 0.622549f, 1.000000));
		std::string type;
		if (isFolder)
			type = "Folder";
		else
			type = SerializeResourceTypeValue(file->m_info.GetResourceType());
		Vec2f typePos = currentPos + Vec2f(15, 205.f) * thumbnailScale;
		drawList->AddText(font, 13 * thumbnailScale, Vec2f(contentRegionStart.x + 5.f * thumbnailScale, bottomRectMax.y - 20.f * thumbnailScale), 0xFF808080, type.c_str());
		ImGui::PopStyleColor(1);
		ImGui::PopID();

		if (file->m_rename) {
			//##Input
			ImGui::PushID(579442659);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.f, 1.f, 1.f, 0.54f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.f));
			Vec2f minTitle = Vec2f(contentRegionStart.x + 5.f * thumbnailScale, lineMax.y + 5.f * thumbnailScale);
			Vec2f maxTitle = Vec2f(lineMax.x - 5.f * thumbnailScale, lineMax.y + 18.f * thumbnailScale);
			auto minCursorPos = minTitle - ImGui::GetWindowPos() + Vec2f(0, ImGui::GetScrollY());
			auto maxCursorPos = maxTitle - minTitle;
			ImGui::SetCursorPos(minCursorPos);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0, maxCursorPos.y * 0.25f));
			ImGui::SetNextItemWidth(maxCursorPos.x);
			if (m_openRename)
			{
				ImGui::SetKeyboardFocusHere();
			}
			bool enter = Wrapper::GUI::InputText("##Input", &this->m_renameFileName, ImGuiInputTextFlags_EnterReturnsTrue);
			if (m_renameFile && !m_openRename && enter)
			{
				const Path oldPath = m_renameFile->m_info.GetFullPath();
				const Path newPath = m_renameFile->m_info.GetFullPath().parent_path()
					/ (m_renameFileName + m_renameFile->m_info.GetExtension().string());

				// Rename Resource in resourceManager and file explorer
				Resource::ResourceManager::HandleRename(oldPath, newPath);

				ReloadContent();
			}
			if (m_renameFile && !m_openRename && (!ImGui::IsItemActive()))
			{
				m_renameFile = nullptr;
				m_renameFileName = "";
				file->m_rename = false;
			}
			m_openRename = false;
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(2);
			ImGui::PopID();
		}
		file->m_hovered = ImGui::IsMouseHoveringRect(contentRegionStart, bottomRectMax);

		if (file->m_hovered)
			ImGui::SetTooltip(file->m_info.GetFileName().c_str());

		if (!file->m_rename && file->m_hovered && !this->m_rightClickOpen)
		{
			if (ImGui::IsMouseReleased(0) || ImGui::IsMouseClicked(1) && !file->m_selected)
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
				{
					if (!file->m_selected)
						AddFileSelected(file);
					else
						RemoveFileSelected(file);
				}
				else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !m_selectedFiles.empty())
				{
					size_t prevSelectedChildIndex = -1;
					auto children = m_currentFile->GetChildren();
					for (size_t i = 0; i < children.size(); i++)
					{
						if (children[i] == m_selectedFiles.back())
						{
							prevSelectedChildIndex = i;
							break;
						}
					}
					ClearSelected();
					SelectRange(prevSelectedChildIndex, index);
				}
				else
				{
					ClearSelected();
					AddFileSelected(file);
				}
			}
			else if (ImGui::IsMouseDoubleClicked(0) && file->m_selected && isFolder)
			{
				SetCurrentFile(file);
				shouldBreak = true;
			}
			if (ImGui::IsMouseClicked(1))
			{
				m_rightClickedFiles = m_selectedFiles;
				openRightClick = true;
			}
		}
	}

}