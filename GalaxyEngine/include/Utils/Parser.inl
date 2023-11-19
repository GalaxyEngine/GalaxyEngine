#pragma once
#include "Utils/Parser.h"

namespace GALAXY
{
	void Utils::Serializer::AddLine(const std::string& key, const std::string& value)
	{
		m_content << m_tab << "[" << key << "] : " << value << '\n';
	}

	void Utils::Serializer::AddLine(const std::string& line)
	{
		m_content << m_tab << line << '\n';
	}

	void Utils::Serializer::PushTab()
	{
		m_tab.push_back(' ');
	}

	void Utils::Serializer::PopTab()
	{
		m_tab.pop_back();
	}
}
