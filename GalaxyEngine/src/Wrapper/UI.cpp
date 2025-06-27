#include "pch.h"
#include "Wrapper/UI.h"

#include <ranges>

#include "Wrapper/Window.h"
#include "Component/ComponentHolder.h"
#include "Component/IComponent.h"

#include "Resource/Texture.h"

#include "Core/Application.h"
#include "Wrapper/Window.h"

#include <set>

namespace GALAXY {

	Wrapper::FontManager* Wrapper::FontManager::s_instance = {};
    
	void Wrapper::FontManager::Initialize()
	{
		s_instance = new FontManager();

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		
		CreateFontGUI(FONT_DEFAULT_NAME, FONT_DEFAULT_PATH, 16.0f);
		CreateFontGUI(FONT_DEFAULT_BOLD_NAME, FONT_DEFAULT_BOLD_PATH, 16.0f);
		CreateFontGUI(FONT_DEFAULT_ITALIC_NAME, FONT_DEFAULT_ITALIC_PATH, 16.0f);
		CreateFontGUI(FONT_DEFAULT_BOLD_ITALIC_NAME, FONT_DEFAULT_BOLD_ITALIC_PATH, 16.0f);

		CreateFontGUI(FONT_TITLE_1, FONT_DEFAULT_BOLD_PATH, 20.0f);
	}

	void Wrapper::FontManager::Destroy()
	{
		delete s_instance;
	}

	void Wrapper::FontManager::AddFonts()
	{
		if (s_instance->m_fontsNotSent.empty())
			return;
		ImGuiIO& io = ImGui::GetIO();
		for (const auto& fontName : s_instance->m_fontsNotSent)
		{
			FontData& font = s_instance->m_fonts[fontName];
			font.handle = io.Fonts->AddFontFromFileTTF(font.path.generic_string().c_str(), font.size);
		}
		s_instance->m_fontsNotSent.clear();
		
		ImGui_ImplOpenGL3_CreateFontsTexture();
	}

	void Wrapper::FontManager::RecreateFonts(float dpiScale)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();

		for (auto& font : s_instance->m_fonts | std::views::values)
		{
			font.handle = io.Fonts->AddFontFromFileTTF(font.path.generic_string().c_str(),  font.size * dpiScale);
		}
	}

	void Wrapper::FontManager::CreateFontGUI(const std::string& name, const std::filesystem::path& fontPath,
												float size)
	{
		if (s_instance->m_fonts.contains(name))
		{
			PrintError("Font %s already exists!", name.c_str());
			return;
		}
		s_instance->m_fonts[name] = {.name = name, .path = fontPath, .size = size};
		s_instance->m_fontsNotSent.push_back(name);
	}

	ImFont* Wrapper::FontManager::GetFont(const std::string& name)
	{
		if (s_instance->m_fonts.contains(name))
			return s_instance->m_fonts[name].handle;
		return nullptr;
	}
	
	static bool s_initialized = false;
	void Wrapper::UI::Initialize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version)
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

		FontManager::Initialize();

		PrintLog("Initalized ImGui %s", IMGUI_VERSION);
		s_initialized = true;
	}

	void Wrapper::UI::UnInitalize()
	{
		if (!s_initialized)
			return;
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		FontManager::Destroy();
	}

	static float current_fontSize;
	void Wrapper::UI::SetDefaultFontSize(const float pixel_size)
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

	void Wrapper::UI::SetTheme()
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
		colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
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
		colors[ImGuiCol_CheckMark] = ImVec4(0.8f, 0.8f, 0.8f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.71f, 0.71f, 0.71f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.90f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.31f, 0.31f, 0.31f, 0.80f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.41f, 0.41f, 0.41f, 0.80f);
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 0.80f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.33f, 0.33f, 0.33f, 0.80f);
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

	void Wrapper::UI::NewFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Wrapper::UI::EndFrame(const std::unique_ptr<Wrapper::Window>& window)
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

	void Wrapper::UI::TextFont(const std::string& font, const std::string& text)
	{
		ImGui::PushFont(FontManager::GetFont(font));
		ImGui::TextUnformatted(text.c_str());
		ImGui::PopFont();
	}

	void Wrapper::UI::PushFont(const std::string& fontName)
	{
		ImFont* font = FontManager::GetFont(fontName);
		ASSERT(font != nullptr);
		ImGui::PushFont(font);
	}

	void Wrapper::UI::PopFont()
	{
		ImGui::PopFont();
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


	bool Wrapper::UI::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = str;
		return ImGui::InputText(label, const_cast<char*>(str->c_str()), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	float Wrapper::UI::DeltaTime()
	{
		return ImGui::GetIO().DeltaTime;
	}

	void Wrapper::UI::SetNextItemOpen(const bool open /*= true*/)
	{
		ImGui::SetNextItemOpen(open);
	}

	bool Wrapper::UI::TreeNode(const char* treeName)
	{
		return ImGui::TreeNode(treeName);
	}

	void Wrapper::UI::TreePop()
	{
		ImGui::TreePop();
	}

	void Wrapper::UI::TreePush(const void* ptr_id, float indent)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGui::Indent(indent);
		window->DC.TreeDepth++;
		ImGui::PushID(ptr_id);
	}

	void Wrapper::UI::TreePop(float indent)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGui::Unindent(indent);

		window->DC.TreeDepth--;
		ImU32 tree_depth_mask = (1 << window->DC.TreeDepth);

		// Handle Left arrow to move to parent tree node (when ImGuiTreeNodeFlags_NavLeftJumpsBackHere is enabled)
		if (window->DC.TreeJumpToParentOnPopMask & tree_depth_mask) // Only set during request
		{
			ImGuiNavTreeNodeData* nav_tree_node_data = &g.NavTreeNodeStack.back();
			IM_ASSERT(nav_tree_node_data->ID == window->IDStack.back());
			if (g.NavIdIsAlive && g.NavMoveDir == ImGuiDir_Left && g.NavWindow == window && ImGui::NavMoveRequestButNoResultYet())
				ImGui::NavMoveRequestResolveWithPastTreeNode(&g.NavMoveResultLocal, nav_tree_node_data);
			g.NavTreeNodeStack.pop_back();
		}
		window->DC.TreeJumpToParentOnPopMask &= tree_depth_mask - 1;

		IM_ASSERT(window->IDStack.Size > 1); // There should always be 1 element in the IDStack (pushed during window creation). If this triggers you called TreePop/PopID too much.
		ImGui::PopID();
	}

	void Wrapper::UI::PushID(const size_t id)
	{
		ImGui::PushID(static_cast<int>(id));
	}

	void Wrapper::UI::PopID()
	{
		ImGui::PopID();
	}

	void Wrapper::UI::SameLine()
	{
		ImGui::SameLine();
	}

	bool Wrapper::UI::Button(const char* buttonName, const Vec2f& size)
	{
		return ImGui::Button(buttonName, size);
	}

	bool Wrapper::UI::DragFloat(const char* label, float* value, float speed, float min, float max, const char* format,
		int flags)
	{
		return ImGui::DragFloat(label, value, speed, min, max, format, flags);
	}

	bool Wrapper::UI::DragInt(const char* label, int* value, float speed, int min, int max, const char* format,
		int flags)
	{
		return ImGui::DragInt(label, value, speed, min, max, format, flags);
	}

	bool Wrapper::UI::DragDouble(const char* label, double* value, float speed, double min, double max,
		const char* format, int flags)
	{
		return ImGui::DragScalar(label, ImGuiDataType_Double, value, speed, &min, &max, format, flags);
	}

	float Wrapper::UI::GetScaleFactor()
	{
		return Core::Application::GetInstance().GetWindow()->GetScreenScale();
	}

	void Wrapper::UI::Test(Component::BaseComponent* component)
	{
		PrintError("eysy");
		
	}

	bool Wrapper::UI::Checkbox(const char* label, bool* checked)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGui::AlignTextToFramePadding();

		float fullWidth = ImGui::GetContentRegionAvail().x;

		float checkboxWidth = ImGui::GetFrameHeight();

		ImGui::TextUnformatted(label);

		ImGui::SameLine(fullWidth - checkboxWidth);

		bool result = ImGui::Checkbox(("##" + std::string(label)).c_str(), checked);

		return result;
	}

	static const char* Items_SingleStringGetter(void* data, int idx)
	{
		const char* items_separated_by_zeros = (const char*)data;
		int items_count = 0;
		const char* p = items_separated_by_zeros;
		while (*p)
		{
			if (idx == items_count)
				break;
			p += strlen(p) + 1;
			items_count++;
		}
		return *p ? p : nullptr;
	}

	bool Wrapper::UI::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGui::AlignTextToFramePadding();
		
		float fullWidth = ImGui::GetContentRegionAvail().x;

		// const char* previewText = Items_SingleStringGetter((void*)items_separated_by_zeros, *current_item);
		// float comboWidth = ImGui::CalcTextSize(previewText, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;

		ImGui::TextUnformatted(label);

		float comboWidth = 180.f;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(comboWidth);
		ImGui::SetCursorPosX(fullWidth - comboWidth);
		bool result = ImGui::Combo(("##" + std::string(label)).c_str(), current_item, items_separated_by_zeros, popup_max_height_in_items);

		return result;
	}
	
	void Wrapper::UI::Spinner(const char* label, float radius, float thickness, uint32_t color, float speed)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
    
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 center(pos.x + radius, pos.y + radius);
		float time = static_cast<float>(ImGui::GetTime()) * speed;
    
		const int num_segments = 12;
		const float two_pi = 2.0f * IM_PI;
		float start = fmodf(time, two_pi);
    
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
		ImVec4 base_color = ImGui::ColorConvertU32ToFloat4(color);
    
		for (int i = 0; i < num_segments; i++)
		{
			float angle = start + (static_cast<float>(i) / num_segments) * two_pi;
			ImVec2 p(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
        
			float fade = (static_cast<float>(i) / num_segments);
			ImVec4 seg_color = base_color;
			seg_color.w *= fade;
			ImU32 seg_color_u32 = ImGui::GetColorU32(seg_color);
        
			draw_list->AddCircleFilled(p, thickness, seg_color_u32);
		}
    
		ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
	}

	void Wrapper::UI::ArcSpinner(const char* label, float radius, float thickness, uint32_t color, float speed)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
    
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 center(pos.x + radius, pos.y + radius);
		float time = static_cast<float>(ImGui::GetTime()) * speed;
    
		const float arc_span = 270.0f * (IM_PI / 180.0f);
		float start_angle = fmodf(time, 2.0f * IM_PI); 
		float end_angle = start_angle + arc_span;
    
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->PathArcTo(center, radius, start_angle, end_angle, 32);
		draw_list->PathStroke(color, false, thickness);
    
		ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
	}

	void Wrapper::UI::LinesSpinner(const char* label, float radius, float thickness, uint32_t color, float speed)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
    
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 center(pos.x + radius, pos.y + radius);
		float time = static_cast<float>(ImGui::GetTime()) * speed;
    
		const int num_lines = 12;
		const float two_pi = 2.0f * IM_PI;
		float start = fmodf(time, two_pi);
    
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec4 base_color = ImGui::ColorConvertU32ToFloat4(color);
    
		for (int i = 0; i < num_lines; i++)
		{
			float angle = start + (static_cast<float>(i) / num_lines) * two_pi;
			float inner_radius = radius * 0.5f;
			float outer_radius = radius;
			ImVec2 start_point(center.x + cosf(angle) * inner_radius, center.y + sinf(angle) * inner_radius);
			ImVec2 end_point(center.x + cosf(angle) * outer_radius, center.y + sinf(angle) * outer_radius);
        
			float fade = (static_cast<float>(i) / num_lines);
			ImVec4 line_color = base_color;
			line_color.w *= fade;
			ImU32 line_color_u32 = ImGui::GetColorU32(line_color);
        
			draw_list->AddLine(start_point, end_point, line_color_u32, thickness);
		}
    
		ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
	}

	void Wrapper::UI::PulsatingDotsSpinner(const char* label, float radius, float dot_radius, uint32_t color,
		float speed)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
    
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 center(pos.x + radius, pos.y + radius);
    
		float time = static_cast<float>(ImGui::GetTime()) * speed;
		const int num_dots = 8;
		const float two_pi = 2.0f * IM_PI;
    
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
		for (int i = 0; i < num_dots; i++)
		{
			float angle = time + (static_cast<float>(i) / num_dots) * two_pi;
			ImVec2 dot_center(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
        
			float phase = (static_cast<float>(i) / num_dots) * two_pi;
			float scale = 0.75f + 0.25f * sinf(time + phase);
			float current_dot_radius = dot_radius * scale;
        
			draw_list->AddCircleFilled(dot_center, current_dot_radius, color);
		}
    
		ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
	}

	void Wrapper::UI::ConcentricSpinners(const char* label, float outer_radius, float thickness, uint32_t color,
		float speed)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
    
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 center(pos.x + outer_radius, pos.y + outer_radius);
    
		float time = static_cast<float>(ImGui::GetTime()) * speed;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
		const float outer_arc_span = 270.0f * (IM_PI / 180.0f);
		float outer_start_angle = fmodf(time, 2.0f * IM_PI);
		float outer_end_angle = outer_start_angle + outer_arc_span;
    
		draw_list->PathArcTo(center, outer_radius, outer_start_angle, outer_end_angle, 32);
		draw_list->PathStroke(color, false, thickness);
    
		float inner_radius = outer_radius * 0.6f;
		const float inner_arc_span = 180.0f * (IM_PI / 180.0f);
		float inner_start_angle = fmodf(-time * 1.5f, 2.0f * IM_PI);
		float inner_end_angle = inner_start_angle + inner_arc_span;
    
		draw_list->PathArcTo(center, inner_radius, inner_start_angle, inner_end_angle, 24);
		draw_list->PathStroke(color, false, thickness * 0.8f);
    
		ImGui::InvisibleButton(label, ImVec2(outer_radius * 2, outer_radius * 2));
	}

	void Wrapper::UI::LineFadeSpinner(const char* label, float radius, float thickness, uint32_t color, float speed)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
    
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 center(pos.x + radius, pos.y + radius);
		float time = static_cast<float>(ImGui::GetTime()) * speed;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		const float arcSpan = 360.f * (IM_PI / 180.0f);
		float startAngle = fmodf(time, 2.0f * IM_PI);
		const int numSegments = 32;

		ImVec4 baseColor = ImGui::ColorConvertU32ToFloat4(color);

		for (int i = 0; i < numSegments; i++)
		{
			float t0 = static_cast<float>(i) / numSegments;
			float t1 = static_cast<float>(i + 1) / numSegments;
			float angle0 = startAngle + t0 * arcSpan;
			float angle1 = startAngle + t1 * arcSpan;
        
			ImVec2 p0(center.x + cosf(angle0) * radius, center.y + sinf(angle0) * radius);
			ImVec2 p1(center.x + cosf(angle1) * radius, center.y + sinf(angle1) * radius);
			
			float fade = t0;
			ImVec4 segColor = baseColor;
			segColor.w *= fade;
			ImU32 segColorU32 = ImGui::GetColorU32(segColor);
        
			draw_list->AddLine(p0, p1, segColorU32, thickness);
		}
    
		ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
	}

	void Wrapper::UI::DisableIniFile(bool value)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (value)
			io.IniFilename = nullptr;
		else
			io.IniFilename = "imgui.ini";
	}

	bool Wrapper::UI::DrawVec3Control(const std::string& label, float* values, const float resetValue /*= 0.0f*/, bool lockButton /*= false*/, float columnWidth /*= 100.0f*/)
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

	ImTextureID Wrapper::UI::GetTextureID(const Resource::Texture* texture)
	{
		return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID()));
	}

	std::shared_ptr<Component::BaseComponent> Wrapper::UI::ComponentPopup()
	{
		if (ImGui::BeginPopup("ComponentPopup"))
		{
			static ImGuiTextFilter filter;
			filter.Draw();
			for (const auto& component : Component::ComponentHolder::GetList())
			{
				if (filter.PassFilter(component->GetComponentName()))
				{
					ImGui::Image(component->GetIcon(), Vec2f(ImGui::GetFrameHeight()));
					ImGui::SameLine();
					const Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
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

	bool Wrapper::UI::TextureButton(ImTextureID textureID, const Vec2f& size)
	{
		return ImGui::ImageButton(textureID, size * GetScaleFactor());
	}

	bool Wrapper::UI::TextureButton(const Resource::Texture* texture, const Vec2f& size)
	{
		if (!texture || !texture->HasBeenSent())
			return false;
		return ImGui::ImageButton(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID())), size * GetScaleFactor());
	}

	bool Wrapper::UI::TextureButtonWithText(Resource::Texture* texture, const char* label, const Vec2f& imageSize, const Vec2f& uv0 /*= {0, 0}*/, const Vec2f& uv1 /*= { 1, 1 }*/, int frame_padding /*= 0*/, const Vec4f& bg_col /*= Vec4f(0, 0, 0, 1)*/, const Vec4f& tint_col /*= Vec4f(1, 1, 1, 1)*/)
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

	bool Wrapper::UI::TextureToggleButtonWithText(ImTextureID texture, const char* label, bool* toggle,
		const Vec2f& imageSize, const Vec2f& uv0, const Vec2f& uv1, int frame_padding, const Vec4f& bg_col,
		const Vec4f& tint_col)
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

	bool Wrapper::UI::TextureToggleButtonWithText(Resource::Texture* texture, const char* label, bool* toggle,
	                                               const Vec2f& imageSize, const Vec2f& uv0 /*= { 0, 0 }*/,
	                                               const Vec2f& uv1 /*= { 1, 1 }*/, int frame_padding /*= 0*/,
	                                               const Vec4f& bg_col /*= Vec4f(0, 0, 0, 1)*/,
	                                               const Vec4f& tint_col /*= Vec4f(1, 1, 1, 1)*/)
	{
		return TextureToggleButtonWithText(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID())), label, toggle, imageSize, uv0, uv1, frame_padding, bg_col, tint_col);
	}

	void Wrapper::UI::TextureImage(Resource::Texture* texture, Vec2f size, const Vec2i& uv0 /*= Vec2i(0, 0)*/, const Vec2i& uv1 /*= Vec2i(1, 1)*/)
	{
		if (!texture || !texture->HasBeenSent())
			return;
		return ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture->GetID())), size, (Vec2f)uv0, (Vec2f)uv1);
	}

	void Wrapper::UI::TextureImage(ImTextureID texture, Vec2f size, const Vec2i& uv0, const Vec2i& uv1)
	{
		if (!texture)
			return;
		return ImGui::Image(texture, size, (Vec2f)uv0, (Vec2f)uv1);
	}

	bool Wrapper::UI::IconButton(const char* label,
	                ImTextureID icon,
	                const Vec2f& icon_size,
	                float spacing,
	                const Vec2f& button_size)
	{
	    ImGui::PushID(label);

	    // Apply custom Unreal-style colors and rounding
	    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    5.f);
	    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  0.2f);
	    ImGui::PushStyleColor(ImGuiCol_Button,              ImVec4(0.16f, 0.16f, 0.16f, 1.f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,       ImVec4(0.20f, 0.20f, 0.20f, 1.f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,        ImVec4(0.29f, 0.29f, 0.29f, 1.f));
	    ImGui::PushStyleColor(ImGuiCol_Text,                ImVec4(0.87f, 0.87f, 0.87f, 1.f));
	    ImGui::PushStyleColor(ImGuiCol_Border,              ImVec4(0.05f, 0.05f, 0.05f, 0.73f));
	    ImGui::PushStyleColor(ImGuiCol_BorderShadow,        ImVec4(0.05f, 0.05f, 0.05f, 0.73f));

	    // Compute the automatic size if user didn't specify
	    ImVec2 text_size = ImGui::CalcTextSize(label, nullptr, true);
	    float default_height = ImMax(icon_size.y, ImGui::GetFrameHeight());
	    float default_width  = ImGui::GetStyle().FramePadding.x * 2
	                         + icon_size.x + spacing + text_size.x;

	    // Final button size: use provided or fallback
	    ImVec2 final_size(
	        button_size.x > 0.f ? button_size.x : default_width,
	        button_size.y > 0.f ? button_size.y : default_height
	    );

	    // Draw the invisible button of the correct size
	    bool pressed = ImGui::Button("", final_size);

	    // Restore style state
	    ImGui::PopStyleColor(6);
	    ImGui::PopStyleVar(2);

	    // Get the actual drawn rect
	    ImVec2 item_min = ImGui::GetItemRectMin();
	    ImVec2 item_max = ImGui::GetItemRectMax();
	    ImVec2 avail    = item_max - item_min;

	    // Compute positions to center icon and text
	    float icon_x = item_min.x + (avail.x - (icon_size.x + spacing + text_size.x)) * 0.5f;
	    float icon_y = item_min.y + (avail.y - icon_size.y) * 0.5f;
	    ImVec2 icon_pos = ImVec2(icon_x, icon_y);
	    ImVec2 icon_end = ImVec2(icon_x + icon_size.x, icon_y + icon_size.y);

	    float text_x = icon_end.x + spacing;
	    float text_y = item_min.y + (avail.y - text_size.y) * 0.5f + 1.f;
	    ImVec2 text_pos = ImVec2(text_x, text_y);

	    // Draw icon and text
	    ImDrawList* dl = ImGui::GetWindowDrawList();
	    dl->AddImage(icon, icon_pos, icon_end);
	    dl->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), label);

	    ImGui::PopID();
	    return pressed;
	}

	void Wrapper::UI::ToggleButton(const char* name, bool* toggle, const Vec2f& size /*= Vec2f(0, 0)*/)
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

	void Wrapper::UI::TextSelectable(const std::string& label, const Vec4f& color /*= Vec4f(1)*/)
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

	bool Wrapper::UI::Splitter(const bool split_vertically, const float thickness, float* size1, float* size2, const float min_size1, const float min_size2, const float splitter_long_axis_size /*= -1.0f*/)
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
