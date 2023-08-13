#include "pch.h"
#include "Wrapper/GUI.h"
#include "Wrapper/Window.h"
#include "Component/ComponentHolder.h"
#include "Component/IComponent.h"

#include "Resource/Texture.h"

namespace GALAXY {

	void Wrapper::GUI::Initalize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version)
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		GLFWwindow* windowPtr = static_cast<GLFWwindow*>(window->GetWindow());
		ImGui_ImplGlfw_InitForOpenGL(windowPtr, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		PrintLog("Initalized ImGui %s", IMGUI_VERSION);
	}

	void Wrapper::GUI::UnInitalize()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Wrapper::GUI::NewFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Wrapper::GUI::EndFrame(const std::unique_ptr<Wrapper::Window>& window)
	{
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = static_cast<GLFWwindow*>(window->GetCurrentContext());
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			Wrapper::Window::MakeContextCurrent(backup_current_context);
		}
	}

	float Wrapper::GUI::DeltaTime()
	{
		return ImGui::GetIO().DeltaTime;
	}

	bool Wrapper::GUI::DrawVec3Control(const std::string& label, float* values, float resetValue /*= 0.0f*/, bool lockButton /*= false*/, float columnWidth /*= 100.0f*/)
	{
		static bool _lock = false;
		bool stillEditing = false;
		if (lockButton && _lock)
		{
			float value = values[0];
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::TextUnformatted(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 3 - 15.f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			// X
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
				values[0] = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			// y
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
				values[0] = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();

			// Z
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
				values[0] = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;
			ImGui::PopItemWidth();
			ImGui::PopStyleVar();

			if (value != values[0]) {
				values[1] = values[0];
				values[2] = values[0];
			}
			ImGui::Columns(1);
		}
		else
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::TextUnformatted(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
				values[0] = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
				values[1] = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
				values[2] = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;;
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::Columns(1);
		}
		if (lockButton) {
			ImGui::SameLine();
			ImGui::Checkbox("Lock", &_lock);
		}

		ImGui::PopID();
		return stillEditing;
	}

	std::shared_ptr<Component::BaseComponent> Wrapper::GUI::ComponentPopup()
	{
		if (ImGui::BeginPopup("ComponentPopup"))
		{
			static ImGuiTextFilter filter;
			filter.Draw();
			for (auto& component : Component::ComponentHolder::GetList())
			{
				if (filter.PassFilter(component->GetComponentName().c_str()))
				{
					Vec2f ButtonSize = Vec2f(ImGui::GetWindowContentRegionWidth(), 0);
					if (ImGui::Button(component->GetComponentName().c_str(), ButtonSize)) {
						ImGui::CloseCurrentPopup();
						return component->Clone();
					}

				}
			}
			ImGui::EndPopup();
		}
		return nullptr;
	}

	bool Wrapper::GUI::TextureButton(Resource::Texture* texture, Vec2f size)
	{
		return ImGui::ImageButton(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID())), size);
	}

	bool Wrapper::GUI::TextureButtonWithText(Resource::Texture* texture, const char* label, const Vec2f& imageSize, const Vec2f& uv0 /*= {0, 0}*/, const Vec2f& uv1 /*= { 1, 1 }*/, int frame_padding /*= 0*/, const Vec4f& bg_col /*= Vec4f(0, 0, 0, 1)*/, const Vec4f& tint_col /*= Vec4f(1, 1, 1, 1)*/)
	{
		bool pressed = false;
		Vec2f cursorPos = ImGui::GetCursorPos();
		int space = ImGui::CalcTextSize(" ").x;
		int spaceNumber = (imageSize.x / space) + 1;
		std::string resultString = label;

		for (int i = 0; i < spaceNumber; i++)
			resultString.insert(resultString.begin(), ' ');

		pressed = ImGui::Button(resultString.c_str(), Vec2f(0, imageSize.y));

		float size = ImGui::GetItemRectSize().x;

		ImGui::SetCursorPos(cursorPos);

		TextureImage(texture, imageSize, uv0, uv1);

		ImGui::SameLine();

		ImGui::InvisibleButton("invisible_Button", Vec2f(ImGui::CalcTextSize(resultString.c_str()).x - imageSize.x, imageSize.y));

		return pressed;
	}

	void Wrapper::GUI::TextureToggleButtonWithText(Resource::Texture* texture, const char* label, bool* toggle, const Vec2f& imageSize, const Vec2f& uv0 /*= { 0, 0 }*/, const Vec2f& uv1 /*= { 1, 1 }*/, int frame_padding /*= 0*/, const Vec4f& bg_col /*= Vec4f(0, 0, 0, 1)*/, const Vec4f& tint_col /*= Vec4f(1, 1, 1, 1)*/)
	{
		Vec2f cursorPos = ImGui::GetCursorPos();
		int space = static_cast<int>(ImGui::CalcTextSize(" ").x);
		int spaceNumber = (imageSize.x / space) + 1;
		std::string resultString = label;

		for (int i = 0; i < spaceNumber; i++)
			resultString.insert(resultString.begin(), ' ');

		ToggleButton(resultString.c_str(), toggle, Vec2f(0, imageSize.y));

		float size = ImGui::GetItemRectSize().x;

		ImGui::SetCursorPos(cursorPos);

		TextureImage(texture, imageSize, uv0, uv1);

		ImGui::SameLine();

		ImGui::InvisibleButton("invisible_Button", Vec2f(ImGui::CalcTextSize(resultString.c_str()).x - imageSize.x, imageSize.y));
	}

	void Wrapper::GUI::TextureImage(Resource::Texture* texture, Vec2f size, const Vec2i& uv0 /*= Vec2i(0, 0)*/, const Vec2i& uv1 /*= Vec2i(1, 1)*/)
	{
		return ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID())), size, (Vec2f)uv0, (Vec2f)uv1);
	}

	void Wrapper::GUI::ToggleButton(const char* name, bool* toggle, const Vec2f& size /*= Vec2f(0, 0)*/)
	{
		if (*toggle == true)
		{
			ImVec4 active = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			ImVec4 hovered = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			ImVec4 base = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			ImGui::PushStyleColor(ImGuiCol_Button, active);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, base);
			if (ImGui::Button(name, size))
			{
				*toggle = !*toggle;
			}
			ImGui::PopStyleColor(3);
		}
		else
		{
			if (ImGui::Button(name, size))
				*toggle = !*toggle;
		}
	}

	void Wrapper::GUI::TextSelectable(const std::string& label, const Vec4f& color /*= Vec4f(1)*/)
	{
		ImGui::PushID(label.c_str());
		ImVec2 text_size = ImGui::CalcTextSize(label.c_str(), label.c_str() + label.size());
		text_size.x = -FLT_MIN; // fill width (suppresses label)
		text_size.y += ImGui::GetStyle().FramePadding.y; // single pad

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 }); // make align with text height
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.f, 0.f, 0.f, 0.f }); // remove text input box
		ImGui::PushStyleColor(ImGuiCol_Text, color);

		ImGui::InputTextMultiline(
			"",
			const_cast<char*>(label.c_str()), // ugly const cast
			label.size() + 1, // needs to include '\0'
			text_size,
			ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoHorizontalScroll
		);

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		ImGui::PopID();
	}

	bool Wrapper::GUI::Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size /*= -1.0f*/)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + ImGui::CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return ImGui::SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

}