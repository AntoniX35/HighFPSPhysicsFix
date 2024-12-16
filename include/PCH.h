#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN

#define NOMB
#define NOMINMAX
#define NOSERVICE

#pragma warning(push)
#include "F4SE/F4SE.h"
#include "RE/Fallout.h"

#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif

#include <CLibUtil/simpleINI.hpp>
#include <CLibUtil/singleton.hpp>
#include <CLibUtil/string.hpp>

using namespace clib_util::singleton;

#pragma warning(pop)

#define DLLEXPORT __declspec(dllexport)

#include "ext/ICommon.h"
#include "ext/IErrors.h"
#include "ext/ITypes.h"
#include <xbyak/xbyak.h>

#include <ext/ID3D11.h>
#include <ext/IHook.h>
#include <ext/INIReader.h>
#include <ext/StrHelpers.h>
#include <ext/stl_containers.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <d3d11.h>
#include <dxgi1_6.h>
#include <shlobj.h>
#include <wrl/client.h>
//#include <ini.h>

#include <CommonStates.h>
#include <SpriteFont.h>

#include "common.h"
#include "getconfig.h"

#include "drv_base.h"

#include "dispatcher.h"

#include "data.h"

#include "config.h"
#include "drv_ids.h"
#include "events.h"
#include "game.h"
#include "havok.h"
#include "helpers.h"
#include "stats.h"
#include "misc.h"
#include "osd.h"
#include "papyrus.h"
#include "render.h"
#include "window.h"

namespace logger = F4SE::log;

using namespace std::literals;
using namespace clib_util::singleton;

namespace stl
{
	using namespace F4SE::stl;

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		F4SE::AllocTrampoline(14);

		auto& trampoline = F4SE::GetTrampoline();
		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class T>
	void write_thunk_jmp(std::uintptr_t a_src)
	{
		F4SE::AllocTrampoline(14);

		auto& trampoline = F4SE::GetTrampoline();
		T::func = trampoline.write_branch<5>(a_src, T::thunk);
	}

	template <class F, size_t index, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[index] };
		T::func = vtbl.write_vfunc(T::idx, T::thunk);
	}

	template <class F, class T>
	void write_vfunc()
	{
		write_vfunc<F, 0, T>();
	}
}

namespace RE
{
	using FormID = std::uint32_t;
	using RefHandle = std::uint32_t;
	using FormType = ENUM_FORM_ID;
}

#include "Version.h"

#endif  //PCH_H
