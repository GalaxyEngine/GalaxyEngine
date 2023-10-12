#pragma once
#include "Utils/Parser.h"

namespace GALAXY
{
	void Utils::Serializer::AddLine(const std::string& key, const std::string& value)
	{
		m_file << m_tab << "[" << key << "] : " << value << '\n';
	}

	void Utils::Serializer::PushTab()
	{
		m_tab.push_back('\t');
	}

	void Utils::Serializer::PopTab()
	{
		m_tab.pop_back();
	}
}
