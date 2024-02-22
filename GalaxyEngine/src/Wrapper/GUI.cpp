#include "pch.h"
#include "Wrapper/GUI.h"
#include "Wrapper/Window.h"
#include "Component/ComponentHolder.h"
#include "Component/IComponent.h"

#include "Resource/Texture.h"

#include "Core/Application.h"
#include "Wrapper/Window.h"

#include <set>

namespace GALAXY {
	static bool s_initialized = false;
	void Wrapper::GUI::Initialize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version)
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

		// Setup Platform/Renderer back ends
		GLFWwindow* windowPtr = static_cast<GLFWwindow*>(window->GetWindow());
		ImGui_ImplGlfw_InitForOpenGL(windowPtr, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		SetTheme();

		PrintLog("Initalized ImGui %s", IMGUI_VERSION);
		s_initialized = true;
	}

	void Wrapper::GUI::UnInitalize()
	{
		if (!s_initialized)
			return;
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	static float current_fontSize;
	void Wrapper::GUI::SetDefaultFontSize(const float pixel_size)
	{
		static ImGuiIO& io = ImGui::GetIO();

		if (pixel_size == current_fontSize)
			return;
		// Create a new ImFontConfig
		ImFontConfig config;
		config.SizePixels = pixel_size;
		config.OversampleH = config.OversampleV = 1;
		config.PixelSnapH = true;    

		// Load the new font
		io.Fonts->AddFontDefault(&config);
		current_fontSize = pixel_size;
	}

	void Wrapper::GUI::SetTheme()
	{
		// ChatGPT generated theme
		ImVec4* colors = ImGui::GetStyle().Colors;

		// Darken the entire color palette
		for (int i = 0; i < ImGuiCol_COUNT; i++) {
			ImVec4& color = colors[i];
			color.x *= 0.7f;
			color.y *= 0.7f;
			color.z *= 0.7f;
		}

		colors[ImGuiCol_Text] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.90f);
		colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.90f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.31f, 0.31f, 0.31f, 0.80f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.41f, 0.41f, 0.41f, 0.80f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.71f, 0.71f, 0.71f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.90f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.31f, 0.31f, 0.31f, 0.80f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.41f, 0.41f, 0.41f, 0.80f);
		colors[ImGuiCol_Header] = ImVec4(0.31f, 0.31f, 0.31f, 0.80f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.41f, 0.41f, 0.41f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.51f, 0.51f, 0.51f, 0.80f);
		colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.60f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.90f, 0.90f, 0.60f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.60f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.90f, 0.90f, 0.90f, 0.60f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
		colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.20f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.80f);
		colors[ImGuiCol_TabActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.91f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.07f, 0.07f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 0.97f);
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

	struct InputTextCallback_UserData
	{
		std::string* Str;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		const InputTextCallback_UserData* user_data = static_cast<InputTextCallback_UserData*>(data->UserData);
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
			std::string* str = user_data->Str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = const_cast<char*>(str->c_str());
		}
		return 0;
	}


	bool Wrapper::GUI::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = str;
		return ImGui::InputText(label, const_cast<char*>(str->c_str()), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	float Wrapper::GUI::DeltaTime()
	{
		return ImGui::GetIO().DeltaTime;
	}

	void Wrapper::GUI::SetNextItemOpen(const bool open /*= true*/)
	{
		ImGui::SetNextItemOpen(open);
	}

	bool Wrapper::GUI::TreeNode(const char* treeName)
	{
		return ImGui::TreeNode(treeName);
	}

	void Wrapper::GUI::TreePop()
	{
		ImGui::TreePop();
	}

	void Wrapper::GUI::PushID(const size_t id)
	{
		ImGui::PushID(static_cast<int>(id));
	}

	void Wrapper::GUI::PopID()
	{
		ImGui::PopID();
	}

	bool Wrapper::GUI::Button(const char* buttonName, const Vec2f buttonSize)
	{
		return ImGui::Button(buttonName, buttonSize);
	}

	void Wrapper::GUI::SameLine()
	{
		ImGui::SameLine();
	}

	float Wrapper::GUI::GetScaleFactor()
	{
		return Core::Application::GetInstance().GetWindow()->GetScreenScale();
	}

	bool Wrapper::GUI::DrawVec3Control(const std::string& label, float* values, const float resetValue /*= 0.0f*/, bool lockButton /*= false*/, float columnWidth /*= 100.0f*/)
	{
		static bool _lock = false;
		bool stillEditing = false;
		if (lockButton && _lock)
		{
			const float value = values[0];

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::TextUnformatted(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 3 - 15.f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			const ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			// X
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			if (ImGui::Button("X", buttonSize))
				values[0] = resetValue;
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			// y
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			if (ImGui::Button("Y", buttonSize))
				values[0] = resetValue;
			ImGui::PopStyleColor(3);

			ImGui::SameLine();

			// Z
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			if (ImGui::Button("Z", buttonSize))
				values[0] = resetValue;
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;
			ImGui::PopItemWidth();
			ImGui::PopItemWidth();
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
			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::TextUnformatted(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			const float lineHeight = GImGui->Font->FontSize * GetScaleFactor() + GImGui->Style.FramePadding.y * 2.0f;
			const ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			if (ImGui::Button("X", buttonSize))
				values[0] = resetValue;
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			if (ImGui::Button("Y", buttonSize))
				values[1] = resetValue;
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			if (ImGui::Button("Z", buttonSize))
				values[2] = resetValue;
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f"))
				stillEditing = true;
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

	ImTextureID Wrapper::GUI::GetTextureID(const Resource::Texture* texture)
	{
		return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID()));
	}

	std::shared_ptr<Component::BaseComponent> Wrapper::GUI::ComponentPopup()
	{
		if (ImGui::BeginPopup("ComponentPopup"))
		{
			static ImGuiTextFilter filter;
			filter.Draw();
			for (const auto& component : Component::ComponentHolder::GetList())
			{
				if (filter.PassFilter(component->GetComponentName()))
				{
					const Vec2f buttonSize = Vec2f(ImGui::GetWindowContentRegionWidth(), 0);
					if (ImGui::Button(component->GetComponentName(), buttonSize)) {
						ImGui::CloseCurrentPopup();
						auto cloned = component->Clone();
						return cloned;
					}
				}
			}
			ImGui::EndPopup();
		}
		return nullptr;
	}

	bool Wrapper::GUI::TextureButton(const Resource::Texture* texture, const Vec2f size)
	{
		if (!texture->HasBeenSent())
			return false;
		return ImGui::ImageButton(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID())), size * GetScaleFactor());
	}

	bool Wrapper::GUI::TextureButtonWithText(Resource::Texture* texture, const char* label, const Vec2f& imageSize, const Vec2f& uv0 /*= {0, 0}*/, const Vec2f& uv1 /*= { 1, 1 }*/, int frame_padding /*= 0*/, const Vec4f& bg_col /*= Vec4f(0, 0, 0, 1)*/, const Vec4f& tint_col /*= Vec4f(1, 1, 1, 1)*/)
	{
		if (!texture->HasBeenSent())
			return false;
		bool pressed = false;
		const Vec2f cursorPos = ImGui::GetCursorPos();
		const int space = static_cast<int>(ImGui::CalcTextSize(" ").x);
		const int spaceNumber = (static_cast<int>(imageSize.x) / space) + 1;
		std::string resultString = label;

		for (int i = 0; i < spaceNumber; i++)
			resultString.insert(resultString.begin(), ' ');

		pressed = ImGui::Button(resultString.c_str(), Vec2f(0, imageSize.y));

		ImGui::SetCursorPos(cursorPos);

		TextureImage(texture, imageSize, uv0, uv1);

		ImGui::SameLine();

		ImGui::InvisibleButton("invisible_Button", Vec2f(ImGui::CalcTextSize(resultString.c_str()).x - imageSize.x, imageSize.y));

		return pressed;
	}

	bool Wrapper::GUI::TextureToggleButtonWithText(Resource::Texture* texture, const char* label, bool* toggle,
	                                               const Vec2f& imageSize, const Vec2f& uv0 /*= { 0, 0 }*/,
	                                               const Vec2f& uv1 /*= { 1, 1 }*/, int frame_padding /*= 0*/,
	                                               const Vec4f& bg_col /*= Vec4f(0, 0, 0, 1)*/,
	                                               const Vec4f& tint_col /*= Vec4f(1, 1, 1, 1)*/)
	{
		const Vec2f cursorPos = ImGui::GetCursorPos();
		const int space = static_cast<int>(ImGui::CalcTextSize(" ").x);
		const int spaceNumber = (static_cast<int>(imageSize.x) / space) + 1;
		const bool before = *toggle;
		bool result = false;

		std::string resultString = label;

		for (int i = 0; i < spaceNumber; i++)
			resultString.insert(resultString.begin(), ' ');


		ToggleButton(resultString.c_str(), toggle, Vec2f(0, imageSize.y));

		result = before != *toggle;

		ImGui::SetCursorPos(cursorPos);

		TextureImage(texture, imageSize, uv0, uv1);

		ImGui::SameLine();

		ImGui::InvisibleButton("invisible_Button", Vec2f(ImGui::CalcTextSize(resultString.c_str()).x - imageSize.x, imageSize.y));
		return result;
	}

	void Wrapper::GUI::TextureImage(Resource::Texture* texture, Vec2f size, const Vec2i& uv0 /*= Vec2i(0, 0)*/, const Vec2i& uv1 /*= Vec2i(1, 1)*/)
	{
		if (!texture || !texture->HasBeenSent())
			return;
		return ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID())), size, (Vec2f)uv0, (Vec2f)uv1);
	}

	void Wrapper::GUI::ToggleButton(const char* name, bool* toggle, const Vec2f& size /*= Vec2f(0, 0)*/)
	{
		if (*toggle == true)
		{
			const Vec4f active = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			const Vec4f hovered = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			const Vec4f base = ImGui::GetStyleColorVec4(ImGuiCol_Button);
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

	bool Wrapper::GUI::Splitter(const bool split_vertically, const float thickness, float* size1, float* size2, const float min_size1, const float min_size2, const float splitter_long_axis_size /*= -1.0f*/)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + ImGui::CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return ImGui::SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}



}