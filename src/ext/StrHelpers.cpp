#include "StrHelpers.h"

namespace StrHelpers
{
	wsconv_t g_strconverter;

	std::string ToString(const std::wstring& wstr)
	{
		return g_strconverter.to_bytes(wstr);
	}

	std::wstring ToWString(const std::string& str)
	{
		return g_strconverter.from_bytes(str);
	}

}