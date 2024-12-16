#include "pch.h"

namespace HFPF
{
	IDriver::IDriver() :
		m_Initialized(false),
		m_IsOK(false)
	{
		IDDispatcher::RegisterDriver(this);
	};

	bool IDriver::Initialize()
	{
		if (m_Initialized) {
			return false;
		}

		LoadConfig();
		PostLoadConfig();
		RegisterHooks();

		if (IsOK()) {
			Patch();
		}

		m_Initialized = true;

		return true;
	};
}
