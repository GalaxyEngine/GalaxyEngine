#include "pch.h"

#include "Utils/OS.h"

bool Debug::Log::LogToFile = true;
bool Debug::Log::m_isFileOpen = false;
std::ofstream Debug::Log::m_file;

Debug::Log::~Log()
{
}

void Debug::Log::OpenFile(const std::tm& calendar_time)
{
	char buf[MAX_LOG_SIZE];
	Utils::OS::Snprintf(buf, sizeof(buf), "%04d-%02d-%02d_%02d-%02d-%02d.log",
		calendar_time.tm_year + 1900, calendar_time.tm_mon + 1,
		calendar_time.tm_mday, calendar_time.tm_hour,
		calendar_time.tm_min, calendar_time.tm_sec);
	auto filePath = Utils::OS::GetEngineDataFolder() / LOG_PATH / buf;
	m_file.open(filePath, std::ios::out | std::ios::app);
	if (!m_file.is_open())
		PrintError("Fail to open Log file");
	m_isFileOpen = true; // Set to true to not spam the Open File
}

void Debug::Log::WriteToFile(LogType type, const std::tm& calendar_time, char* messageAndFile)
{
	char result[MAX_LOG_SIZE];
	const char* logTypeString = SerializeLogTypeValue(type);
	Utils::OS::Snprintf(result, MAX_LOG_SIZE, "[%02d:%02d:%02d] (%s) %s", calendar_time.tm_hour,
	calendar_time.tm_min, calendar_time.tm_sec, logTypeString, messageAndFile);
	m_file << std::string(result);
}

void Debug::Log::CloseFile()
{
	if (m_file.is_open())
	{
		m_file.close();
	}
}

#ifdef WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"

void Debug::Log::AddTextToConsole(const LogType type, const std::string& text)
{
	if (const auto EditorUIManager = Editor::UI::EditorUIManager::GetInstance())
		EditorUIManager->GetConsole()->AddText(type, text);
}
#endif
