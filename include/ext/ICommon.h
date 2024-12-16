#pragma once

#define FN_NAMEPROC(x)                                             \
	virtual const char* ModuleName() const noexcept { return x; }; \
	// virtual const char* LogPrefix() const noexcept { return "["  x  "] "; }; \
// virtual const char* LogPrefixWarning() const noexcept { return "<WARNING> ["  x  "] "; }; \
// virtual const char* LogPrefixError() const noexcept { return "<ERROR> ["  x  "] "; }; \
// virtual const char* LogPrefixFatal() const noexcept { return "<FATAL> ["  x  "] "; };

#define SKMP_FORCEINLINE __forceinline
#define SKMP_NOINLINE __declspec(noinline)
#define SKMP_ALIGN(x) __declspec(align(x))

#if defined(__AVX__) || defined(__AVX2__)
#	define SIMD_ALIGNMENT 32
#else
#	define SIMD_ALIGNMENT 16
#endif

#define SKMP_ALLOC_ALIGN SIMD_ALIGNMENT

#define SKMP_ALIGN_AUTO __declspec(align(SIMD_ALIGNMENT))

#include <exception>
#include <string>
//#include <xstddef>

#include "STLCommon.h"

namespace except
{
	class descriptor
	{
	public:
		descriptor() :
			m_desc(std::exception().what())
		{
		}

		descriptor(std::exception const& a_rhs)
		{
			m_desc = a_rhs.what();
		}

		descriptor& operator=(std::exception const& a_rhs)
		{
			m_desc = a_rhs.what();
			return *this;
		}

		descriptor& operator=(const char* a_desc)
		{
			m_desc = a_desc;
			return *this;
		}

		inline constexpr const char* what() const noexcept
		{
			return m_desc.c_str();
		}

	private:
		std::string m_desc;
	};
}

//#include "AddressLibrary.h"
//#include "GameHandles.h
#include "Hash.h"
#include "IHook.h"
//#include "ILogging.h"
#include "IMisc.h"
#include "IRandom.h"
#include "Math.h"
#include "Mem.h"
//#include "Patching.h"
#include "PerfCounter.h"
//#include "RTTI.h"
#include "STL.h"
#include "Threads.h"
#include "stl_optional.h"
#include "stl_stdio.h"
