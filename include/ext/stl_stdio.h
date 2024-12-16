#pragma once

#include <stdio.h>

namespace stl
{
	template <std::size_t _Size, class... Args>
	auto snprintf(char (&a_buffer)[_Size], const char* a_fmt, Args... a_args)
	{
		return _snprintf_s(a_buffer, _TRUNCATE, a_fmt, std::forward<Args>(a_args)...);
	}

    template <std::size_t _Len>
	auto strlen(const char (&a_string)[_Len])
	{
		return ::strnlen_s(a_string, _Len);
	}

}