#pragma once

#include "drv_ids.h"

namespace HFPF
{

	class IDriver
	{
		friend class IDDispatcher;

	public:
		static inline constexpr auto ID = DRIVER_ID::INVALID;

		SKMP_FORCEINLINE bool IsInitialized() const { return m_Initialized; }
		SKMP_FORCEINLINE bool IsOK() const { return m_IsOK; }

		IDriver(const IDriver&) = delete;
		IDriver(IDriver&&) = delete;
		IDriver& operator=(const IDriver&) = delete;
		void     operator=(IDriver&&) = delete;

		FN_NAMEPROC("IDriver");
		FN_ESSENTIAL(false);
		FN_DRVDEF(-1);

	protected:
		IDriver();
		virtual ~IDriver() noexcept = default;

		SKMP_FORCEINLINE void SetOK(bool b) { m_IsOK = b; }

	private:
		virtual void               LoadConfig(){};
		virtual void               PostLoadConfig(){};
		virtual void               Patch(){};
		virtual void               RegisterHooks(){};
		virtual void               PostInit(){};
		virtual void               PostPatch(){};
		[[nodiscard]] virtual bool Prepare() { return false; };
		virtual void               OnGameConfigLoaded(){};

		[[nodiscard]] bool Initialize();

		bool m_Initialized;
		bool m_IsOK;
	};

}
