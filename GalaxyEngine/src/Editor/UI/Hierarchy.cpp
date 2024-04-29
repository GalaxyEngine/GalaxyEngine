#include "pch.h"
#include "Editor/UI/EditorUIManager.h"

#include "Core/SceneHolder.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

#include "Resource/Scene.h"

#include "Editor/ActionManager.h"
#include "Resource/Prefab.h"
#include "Resource/ResourceManager.h"

using namespace Core;

void Editor::UI::Hierarchy::Draw()
{
    if (!m_inspector)
        m_inspector = EditorUIManager::GetInstance()->GetInspector();
    if (!p_open)
        return;
    if (ImGui::Begin("Hierarchy"))
    {
        const Weak<GameObject> root = SceneHolder::GetCurrentScene()->GetRootGameObject();
        uint64_t index = 0;
        DisplayGameObject(root, index);

        if (m_openRightClick)
        {
            ImGui::OpenPopup("RightClick");
            m_openRightClick = false;
        }
        RightClickPopup();
    }
    ImGui::End();
}

void Editor::UI::Hierarchy::DisplayGameObject(const Weak<GameObject>& weakGO, uint64_t& index, bool display /*= true*/)
{
    GameObject* gameobject = weakGO.lock().get();
    if (!gameobject)
        return;

    gameobject->m_sceneGraphID = index;

    if (!display)
    {
        // Do the same for all children
        for (auto&& child : gameobject->m_children)
        {
            if (!child)
                continue;
            ImGui::TreePush(child->m_name.c_str());
            index++;
            DisplayGameObject(child, index, display);
            ImGui::TreePop();
        }
        return;
    }
    ImGui::PushID(static_cast<int>(index));
    // Display arrow button
    if (!gameobject->m_children.empty())
    {
        if (!gameobject->m_open)
        {
            if (ImGui::ArrowButton("##right", ImGuiDir_Right))
            {
                gameobject->m_open = true;
            }
        }
        else if (gameobject->m_open)
        {
            if (ImGui::ArrowButton("##down", ImGuiDir_Down))
            {
                gameobject->m_open = false;
            }
        }
        ImGui::SameLine(0, 10);
    }
    else
    {
        ImGui::Button("O", Vec2f(ImGui::GetFrameHeight()));
        ImGui::SameLine();
    }

    // Rename Field
    if (m_renameObject == gameobject)
    {
        static std::string name;
        name = m_renameObject->m_name;
        if (m_openRename)
        {
            ImGui::SetKeyboardFocusHere();
        }
        Wrapper::GUI::InputText("##InputText", &name);
        if (m_renameObject && !m_openRename && !ImGui::IsItemActive())
        {
            m_renameObject->m_name = name;
            m_renameObject = nullptr;
        }
        m_openRename = false;
    }
    // Selectable Field
    else if (ImGui::Selectable(gameobject->m_name.c_str(), gameobject->m_selected, ImGuiSelectableFlags_SelectOnNav))
    {
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !m_inspector->GetSelectedGameObjects().empty())
        {
            const Shared<GameObject> lastSelected = m_inspector->GetSelectedGameObjects()[0].lock();
            const size_t lastSelectedId = lastSelected->GetSceneGraphID();
            const size_t objectId = gameobject->GetSceneGraphID();
            const bool inferior = lastSelectedId > objectId;
            for (const Shared<GameObject>& val : SceneHolder::GetCurrentScene()->GetObjectList() | std::views::values)
            {
                const size_t currentObjectID = val->GetSceneGraphID();
                const Shared<GameObject> parent = val->GetParent();
                if (!parent || !parent->m_open)
                    continue;

                if (inferior && (currentObjectID >= lastSelectedId || currentObjectID <= objectId))
                    continue;
                if (!inferior && (currentObjectID <= lastSelectedId || currentObjectID >= objectId))
                    continue;
                m_inspector->AddSelected(val);
            }

            m_inspector->AddSelected(weakGO);
        }
        // Multi Select
        else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
            m_inspector->AddSelected(weakGO);
            // already selected and not in multi select -> rename
        else if (gameobject->m_selected && m_inspector->GetSelectedGameObjects().size() == 1)
        {
            SetRename(gameobject);
        }
        // clear all selected and set selected
        else
            m_inspector->SetSelected(weakGO);
    }

    // Right Click
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && gameobject->GetParent())
    {
        m_openRightClick = true;
        // Do not reselect when is already selected.
        if (!gameobject->m_selected)
        {
            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
                m_inspector->AddSelected(weakGO);
            else
                m_inspector->SetSelected(weakGO);
        }
    }
    else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !gameobject->GetParent())
    {
        m_openRightClick = true;
        m_inspector->ClearSelected();
    }
    else if (!ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered())
    {
        // Clear when none is clicked
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_inspector->ClearSelected();
        // clear and open right click popup when no one is right-clicked
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            m_openRightClick = true;
            m_inspector->ClearSelected();
        }
    }
    if ((ImGui::IsWindowFocused() || EditorUIManager::GetInstance()->GetSceneWindow()->IsFocused()) &&
        ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        for (const Weak<GameObject>& object : m_inspector->GetSelectedGameObjects())
        {
            if (!object.lock())
                continue;

            object.lock()->Destroy();
        }
    }

    // === Drag And Drop === //
    if (gameobject->m_parent.lock() && ImGui::BeginDragDropSource())
    {
        List<uint64_t> indices;
        if (gameobject->m_selected)
        {
            List<Weak<GameObject>> selected = m_inspector->GetSelectedGameObjects();
            for (size_t i = 0; i < selected.size(); i++)
            {
                indices.push_back(selected[i].lock()->GetSceneGraphID());
            }
        }
        else
        {
            indices.push_back(gameobject->GetSceneGraphID());
        }

        ImGui::SetDragDropPayload("GAMEOBJECTS", indices.data(), indices.size() * sizeof(uint64_t));
        ImGui::Text("Size : %d", static_cast<int>(indices.size()));
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS"))
        {
            // Check if the payload data type matches
            List<uint64_t> indices;
            if (payload->DataSize % sizeof(uint64_t) == 0)
            {
                auto payloadData = static_cast<uint64_t*>(payload->Data);
                const uint64_t payloadSize = payload->DataSize / sizeof(uint64_t);
                indices.assign(payloadData, payloadData + payloadSize);
            }
            for (size_t i = 0; i < indices.size(); i++)
            {
                Weak<GameObject> payloadGameObject;
                payloadGameObject = SceneHolder::GetCurrentScene()->GetWithSceneGraphID(indices[i]);
                if (payloadGameObject.lock() && payloadGameObject.lock()->m_parent.lock() && !gameobject->IsAParent(
                    payloadGameObject.lock().get()))
                {
                    payloadGameObject.lock()->SetParent(weakGO);
                    gameobject->m_open = true;
                }
            }
        }

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
        {
            UNUSED(payload);
            const auto explorer = Editor::UI::EditorUIManager::GetInstance()->GetFileExplorer();
            const auto draggedFiles = explorer->m_draggedFiles;
            for (size_t i = 0; i < draggedFiles.size(); ++i)
            {
                if (draggedFiles[i]->m_info.GetResourceType() == Resource::ResourceType::Prefab)
                {
                    const auto resource = Resource::ResourceManager::GetOrLoad<Resource::Prefab>(draggedFiles[i]->m_info.GetFullPath()).lock();
                    resource->Instantiate(gameobject->weak_from_this());
                }
            }
            explorer->ReloadContent();
        }
        ImGui::EndDragDropTarget();
    }

    if (display)
        display = gameobject->m_open;

    const auto drawList = ImGui::GetWindowDrawList();
    const float centerX = std::floor(ImGui::GetFrameHeight() * 0.5f);
    const Vec2f verticalLine = ImGui::GetCursorScreenPos() + Vec2f(centerX, -4);
    Vec2f lastChildPos = Vec3f::Zero();
    constexpr ImU32 white = IM_COL32_WHITE;
    // Do the same for all children
    for (size_t i = 0; i < gameobject->m_children.size(); i++)
    {
        auto child = gameobject->m_children[i];
        if (!child)
            continue;
        Vec2f cursorPos = ImGui::GetCursorScreenPos() + Vec2f(centerX + 1, centerX);
        if (i == gameobject->m_children.size() - 1)
            lastChildPos = ImGui::GetCursorScreenPos();
        
        // Draw horizontal line
        if (display)
            drawList->AddLine(cursorPos, cursorPos + Vec2f(centerX, 0), white);
        
        Wrapper::GUI::TreePush(child->m_name.c_str(), ImGui::GetFrameHeight());
        DisplayGameObject(child, i, display);
        Wrapper::GUI::TreePop(ImGui::GetFrameHeight());
    }        

    if (!gameobject->m_children.empty() && gameobject->m_open)
    {
        auto lastChild = gameobject->m_children[gameobject->m_children.size() - 1];
        drawList->AddLine(verticalLine, lastChildPos + Vec2f(centerX, centerX + 1), white);
    }
    
    ImGui::PopID();
}

void CreateGameObject(const List<Weak<GameObject>>& selected)
{
    Resource::Scene* currentScene = SceneHolder::GetCurrentScene();
    if (selected.empty())
    {
        auto createObject = [currentScene = currentScene]()
        {
            const Weak<GameObject> gameObject = currentScene->CreateObject();
            currentScene->GetRootGameObject().lock()->AddChild(gameObject.lock());
            currentScene->GetRootGameObject().lock()->SetHierarchyOpen(true);
        };

        createObject();
        const Editor::Action action(
            createObject,
            [currentScene = currentScene]()
            {
                currentScene->RevertObject();
            });

        currentScene->GetActionManager()->AddAction(action);
    }
    else
    {
        auto createObjects = [selected = selected, currentScene = currentScene]()
        {
            for (size_t i = 0; i < selected.size(); i++)
            {
                Weak<GameObject> gameObject = currentScene->CreateObject();
                if (selected[i].lock())
                {
                    selected[i].lock()->AddChild(gameObject.lock());
                    selected[i].lock()->SetHierarchyOpen(true);
                }
            }
        };

        createObjects();
        const Editor::Action action(
            createObjects,
            [number = selected.size(), currentScene = currentScene]()
            {
                currentScene->RevertObject(number);
            });
        currentScene->GetActionManager()->AddAction(action);
    }
}

void Editor::UI::Hierarchy::RightClickPopup()
{
    static bool openModelPopup = false;
    if (ImGui::BeginPopup("RightClick", ImGuiWindowFlags_NoDecoration))
    {
        const List<Weak<GameObject>> selectedGameObjects = m_inspector->GetSelectedGameObjects();
        const auto buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
        
        if (ImGui::BeginMenu("Create"))
        {
            MainBar::DisplayCreateGameObject(openModelPopup);
            ImGui::EndMenu();
        }
        

        // === At least one selected === //
        if (!selectedGameObjects.empty())
        {
            // === if selected are sibling === //
            if (!selectedGameObjects.empty())
            {
                ImGui::BeginDisabled(!selectedGameObjects[0].lock()->IsSibling(selectedGameObjects));
            }
            {
                if (ImGui::Button("Create Parent", buttonSize))
                // "Create Parent" button, clickable if at least one item is selected
                {
                    const Weak<GameObject> parent = SceneHolder::GetCurrentScene()->CreateObject();
                    const uint32_t childIndex = selectedGameObjects[0].lock()->GetParent()->
                                                            GetChildIndex(selectedGameObjects[0].lock().get());
                    selectedGameObjects[0].lock()->GetParent()->AddChild(parent.lock(), childIndex);

                    for (const Weak<GameObject>& i : selectedGameObjects)
                    {
                        i.lock()->SetParent(parent);
                    }
                    parent.lock()->m_open = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!selectedGameObjects.empty())
            {
                ImGui::EndDisabled();
            }
            // === if only one selected === //
            ImGui::BeginDisabled(selectedGameObjects.size() > 1);
            {
                if (ImGui::Button("Rename", buttonSize))
                {
                    SetRename(selectedGameObjects[0].lock().get());
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndDisabled();

            if (ImGui::Button("Delete", buttonSize))
            {
                for (const Weak<GameObject>& i : selectedGameObjects)
                {
                    i.lock()->Destroy();
                }
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("Copy", buttonSize))
            {
                Application::GetInstance().CopyObject();
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::Button("Paste", buttonSize))
        {
            Application::GetInstance().PasteObject();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (openModelPopup)
    {
        
        const List<Weak<GameObject>> selectedGameObjects = m_inspector->GetSelectedGameObjects();

        bool terminate = false;
        if (!selectedGameObjects.empty())
        {
            terminate = MainBar::UpdateModelPopup(openModelPopup, selectedGameObjects[0].lock().get());
        }
        else
        {
            terminate = MainBar::UpdateModelPopup(openModelPopup);
        }
        if (terminate)
        {
            openModelPopup = false;
        }
    }
    
}

void Editor::UI::Hierarchy::SetRename(GameObject* gameObject)
{
    m_renameObject = gameObject;
    m_openRename = true;
}
