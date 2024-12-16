#include <osd.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <ext/InputMap.h>

using std::chrono::milliseconds;

namespace HFPF
{
	static constexpr const char* SECTION_MAIN = "Main";
	static constexpr const char* SECTION_OSD = "OSD";
	static constexpr const char* CKEY_ENABLE = "EnableOSD";
	static constexpr const char* CKEY_FONT = "Font";
	static constexpr const char* CKEY_FONTFILE = "FontFile";
	static constexpr const char* CKEY_FONTCOLOR = "Color";
	static constexpr const char* CKEY_FONTOUTLINECOLOR = "OutlineColor";
	static constexpr const char* CKEY_FONTOUTLINEOFFSET = "OutlineOffset";
	static constexpr const char* CKEY_FPS = "FPS";
	static constexpr const char* CKEY_BAREFPS = "Bare_FPS";
	static constexpr const char* CKEY_FRAMETIME = "FrameTime";
	static constexpr const char* CKEY_BAREFRAMETIME = "Bare_FrameTime";
	static constexpr const char* CKEY_COUNTER = "Counter";
	static constexpr const char* CKEY_VRAM = "VRAM";
	static constexpr const char* CKEY_ALL = "All";

	//static constexpr const char* CKEY_TOGGLEKEY = "ToggleKey";
	static constexpr const char* CKEY_ALIGN = "Align";
	static constexpr const char* CKEY_OFFSET = "Offset";
	static constexpr const char* CKEY_SCALE = "Scale";
	static constexpr const char* CKEY_AUTOSCALE = "AutoScale";
	static constexpr const char* CKEY_INTERVAL = "UpdateInterval";
	//static constexpr const char* CKEY_COMBOKEY = "ComboKey";
	static constexpr const char* CKEY_SCALETOWINDOW = "ScaleToWindow";

	static constexpr std::uint32_t F_SHOW_FPS = 1U << 0;
	static constexpr std::uint32_t F_SHOW_FPS_SIMPLE = 1U << 2;
	static constexpr std::uint32_t F_SHOW_FRAMETIME = 1U << 3;
	static constexpr std::uint32_t F_SHOW_FRAMETIME_SIMPLE = 1U << 4;
	static constexpr std::uint32_t F_SHOW_COUNTER = 1U << 5;
	static constexpr std::uint32_t F_SHOW_VRAM_USAGE = 1U << 6;
	static constexpr std::uint32_t F_SHOW_ALL = (F_SHOW_FPS | F_SHOW_FRAMETIME | F_SHOW_COUNTER | F_SHOW_VRAM_USAGE);

	using namespace DirectX;

	DOSD DOSD::m_Instance;

	DOSD::DOSD()
	{
		m_bufStats1[0] = 0x0;
		m_bufStats2[0] = 0x0;
		m_bufStats3[0] = 0x0;
		m_bufStats4[0] = 0x0;

		m_stats.lastUpdate = IPerfCounter::Query();
		m_stats.frameCounter = 0;
		m_stats.lastFrameCount = 0;
		m_stats.draw = false;
		m_stats.warmup = 0;
	}

	void DOSD::LoadConfig()
	{
		m_conf.enabled = GetConfigValue(SECTION_OSD, CKEY_ENABLE, false);
		m_conf.fps = GetConfigValue(SECTION_OSD, CKEY_FPS, true);
		m_conf.barefps = GetConfigValue(SECTION_OSD, CKEY_BAREFPS, false);
		m_conf.frametime = GetConfigValue(SECTION_OSD, CKEY_FRAMETIME, false);
		m_conf.bareframetime = GetConfigValue(SECTION_OSD, CKEY_BAREFRAMETIME, false);
		m_conf.counter = GetConfigValue(SECTION_OSD, CKEY_COUNTER, false);
		m_conf.vram = GetConfigValue(SECTION_OSD, CKEY_VRAM, true);
		m_conf.all = GetConfigValue(SECTION_OSD, CKEY_ALL, false);
		m_conf.interval = std::clamp(GetConfigValue(SECTION_OSD, CKEY_INTERVAL, 1.0f), 0.01f, 900.0f);
		//m_conf.combo_key = GetConfigValue<UInt32>(SECTION_OSD, CKEY_COMBOKEY, 160);
		//m_conf.key = GetConfigValue<UInt32>(SECTION_OSD, CKEY_TOGGLEKEY, 112);
		m_conf.align = ConfigGetStatsRendererAlignment(GetConfigValue(SECTION_OSD, CKEY_ALIGN, 1));
		m_conf.offset = GetConfigValue(SECTION_OSD, CKEY_OFFSET, "4 4");
		m_conf.font_scale = GetConfigValue(SECTION_OSD, CKEY_SCALE, "1 0.9");
		m_conf.font_autoscale = GetConfigValue(SECTION_OSD, CKEY_AUTOSCALE, true);
		m_conf.scale_to_window = GetConfigValue(SECTION_OSD, CKEY_SCALETOWINDOW, true);
		//m_conf.font = GetConfigValue<Font>(SECTION_OSD, CKEY_FONT, Font::DroidSans);
		m_conf.font_file = GetConfigValue(SECTION_OSD, CKEY_FONTFILE, "");
		m_conf.font_color = GetConfigValue(SECTION_OSD, CKEY_FONTCOLOR, "21 255 18 240");
		m_conf.font_outline_color = GetConfigValue(SECTION_OSD, CKEY_FONTOUTLINECOLOR, "0 0 0");
		m_conf.outline_size = GetConfigValue(SECTION_OSD, CKEY_FONTOUTLINEOFFSET, 1.0f);
	}

	void DOSD::PostLoadConfig()
	{
		auto rd = IDDispatcher::GetDriver<DRender>();
		if (!rd->IsOK()) {
			logger::info("[OSD] Render driver unavailable, disabling");
			m_conf.enabled = false;
		}

		if (m_conf.enabled) {
			// if (!m_conf.key || m_conf.key >= InputMap::kMaxMacros) {
			// logger::info("[OSD] Invalid toggle key, resetting to default");
			// m_conf.key = DIK_INSERT;
			// }

			m_stats.draw = true;
			m_stats.interval = static_cast<long long>(m_conf.interval * 1000000.0f);

			m_Instance.m_stats.warmup = 2;

			// if (!m_conf.font || m_conf.font >= Font::FontMax) {
			// m_conf.font = Font::DroidSans;
			// }

			ConfigParseColors(m_conf.font_color, m_stats.colors.font);
			ConfigParseColors(m_conf.font_outline_color, m_stats.colors.outline);
			ConfigParseScale(m_conf.font_scale, m_stats.scale);
			ConfigParseVector2(m_conf.offset, m_stats.offset);

			m_dRender = rd;
		}
	}

	void DOSD::RegisterHooks()
	{
		if (m_conf.enabled) {
			IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_OSD);
			IEvents::RegisterForEvent(Event::OnD3D11PostPostCreate, OnD3D11PostPostCreate_OSD);

			//m_inputEventHandler.SetKeys(m_conf.combo_key, m_conf.key);

			m_dRender->AddPresentCallbackPre(Present_Pre);
			m_dRender->AddPresentCallbackPost(Present_Post);
		}
	}

	bool DOSD::Prepare()
	{
		return IDDispatcher::DriverOK(DRender::ID);
	}

	// int DOSD::ConfigGetFontResource(Font param)
	// {
	// return IDR_DROIDSANS;
	// }

	StatsRenderer::Align DOSD::ConfigGetStatsRendererAlignment(std::int32_t param)
	{
		switch (param) {
		case 1:
			return StatsRenderer::Align::TOP_LEFT;
		case 2:
			return StatsRenderer::Align::TOP_RIGHT;
		case 3:
			return StatsRenderer::Align::BOTTOM_LEFT;
		case 4:
			return StatsRenderer::Align::BOTTOM_RIGHT;
		default:
			return StatsRenderer::Align::TOP_LEFT;
		}
	}

	void DOSD::ConfigParseColors(const std::string& in, XMVECTORF32& out)
	{
		std::vector<float> cols;
		StrHelpers::SplitString<float>(in, ' ', cols);
		if (cols.size() > 2) {
			for (int i = 0; i < 3; i++) {
				out.f[i] = std::clamp(cols[i], 0.0f, 255.0f) / 255.0f;
			}
			if (cols.size() > 3) {
				out.f[3] = std::clamp(cols[3], 0.0f, 255.0f) / 255.0f;
			} else {
				out.f[3] = 1.0f;
			}
		} else {
			out = Colors::White;
		}
	}

	void DOSD::ConfigParseScale(const std::string& in, DirectX::XMFLOAT2A& out)
	{
		std::vector<float> scale;
		StrHelpers::SplitString<float>(in, ' ', scale);
		if (scale.size() > 0) {
			if (scale.size() > 1) {
				out.y = scale[1];
			} else {
				out.y = scale[0];
			}

			out.x = scale[0];
		} else {
			out.x = 1.0f;
			out.y = 1.0f;
		}
	}

	void DOSD::ConfigParseVector2(const std::string& in, DirectX::XMFLOAT2A& out)
	{
		std::vector<float> v2;
		StrHelpers::SplitString<float>(in, ' ', v2);
		if (v2.size() > 0) {
			out.x = v2[0];
			if (v2.size() > 1) {
				out.y = v2[1];
			}
		}
	}

	StatsRenderer::StatsRenderer(
		ID3D11Device*        a_pDevice,
		ID3D11DeviceContext* a_pDeviceContext,
		UINT a_bufferX, UINT a_bufferY,
		const XMFLOAT2A&   a_offset,
		float              a_outlineSize,
		Align              a_alignment,
		const XMFLOAT2A&   a_scale,
		const XMVECTORF32& a_fontColor,
		const XMVECTORF32& a_outlineColor) :
		m_isLoaded(false),
		m_pDevice(a_pDevice),
		m_pDeviceContext(a_pDeviceContext),
		m_bufferSize(
			static_cast<float>(a_bufferX),
			static_cast<float>(a_bufferY)),
		m_offset(a_offset),
		m_alignment(a_alignment),
		m_scale(XMLoadFloat2A(std::addressof(a_scale))),
		m_fontColor(a_fontColor),
		m_outlineColor(a_outlineColor),
		m_origin(g_XMZero)
	{
		m_commonStates = std::make_unique<CommonStates>(a_pDevice);
		m_spriteBatch = std::make_unique<SpriteBatch>(a_pDeviceContext);

		m_blendState = m_commonStates->NonPremultiplied();

		XMFLOAT2A tmp(a_outlineSize, a_outlineSize);
		m_outlineSize = XMLoadFloat2A(std::addressof(tmp));

		switch (m_alignment) {
		case Align::TOP_LEFT:
			m_pos = XMLoadFloat2A(&m_offset);
			break;
		case Align::TOP_RIGHT:
			tmp = XMFLOAT2A(m_bufferSize.x, m_offset.y);
			m_pos = XMLoadFloat2A(std::addressof(tmp));
			break;
		case Align::BOTTOM_LEFT:
			tmp = XMFLOAT2A(m_offset.x, m_bufferSize.y);
			m_pos = XMLoadFloat2A(std::addressof(tmp));
			break;
		case Align::BOTTOM_RIGHT:
			m_pos = XMLoadFloat2A(std::addressof(m_bufferSize));
			break;
		}

		m_outlinePos[3] = g_XMZero.v;

		AdjustOutline();
	}

	bool StatsRenderer::Load(int resource)
	{
		auto handle = GetModuleHandle(L"Fallout4.exe");

		HRSRC hRes = ::FindResource(handle, MAKEINTRESOURCE(resource), RT_RCDATA);
		if (hRes == nullptr) {
			return false;
		}

		HGLOBAL hData = ::LoadResource(handle, hRes);
		if (hData == nullptr) {
			return false;
		}

		DWORD dSize = ::SizeofResource(handle, hRes);

		LPVOID pData = ::LockResource(hData);
		if (pData == nullptr) {
			goto finish;
		}

		try {
			m_font = std::make_unique<SpriteFont>(m_pDevice, reinterpret_cast<std::uint8_t* const>(pData), dSize);
		} catch (const std::exception& e) {
			m_lastException = e;
			goto finish;
		}

		m_isLoaded = true;

finish:
		FreeResource(hData);

		return m_isLoaded;
	}

	bool StatsRenderer::Load(const wchar_t* filename)
	{
		try {
			m_font = std::make_unique<SpriteFont>(m_pDevice, filename);
		} catch (const std::exception& e) {
			m_lastException = e;
			return false;
		}

		return (m_isLoaded = true);
	}

	void StatsRenderer::DrawStrings()
	{
		if (!m_isLoaded) {
			return;
		}

		D3D11StateBackup _(std::addressof(m_backup), m_pDeviceContext);

		try {
			auto text = m_drawString.c_str();
			auto sb = m_spriteBatch.get();

			m_spriteBatch->Begin(SpriteSortMode_Deferred, m_blendState);
			m_font->DrawString(sb, text, m_outlinePos[0], m_outlineColor, 0.0f, m_origin, m_scale);
			m_font->DrawString(sb, text, m_outlinePos[1], m_outlineColor, 0.0f, m_origin, m_scale);
			m_font->DrawString(sb, text, m_outlinePos[2], m_outlineColor, 0.0f, m_origin, m_scale);
			m_font->DrawString(sb, text, m_outlinePos[3], m_outlineColor, 0.0f, m_origin, m_scale);
			m_font->DrawString(sb, text, m_pos, m_fontColor, 0.0f, m_origin, m_scale);
			m_spriteBatch->End();

		} catch (const std::exception&) {
		}
	}

	void StatsRenderer::Update()
	{
		if (!m_isLoaded) {
			return;
		}

		try {
			UpdateStrings();
			AdjustPosition();
		} catch (const std::exception&) {
		}
	}

	void StatsRenderer::UpdateStrings()
	{
		std::wostringstream ss;

		for (const auto& f : m_callbacks) {
			auto buf = f();
			if (buf != nullptr && buf[0] != 0x0) {
				ss << buf << std::endl;
			}
		}

		m_drawString = ss.str();
	}

	void StatsRenderer::AdjustPosition()
	{
		switch (m_alignment) {
		case Align::TOP_RIGHT:
			{
				auto w = m_font->MeasureString(m_drawString.c_str(), false);

				auto x = m_bufferSize.x - (w.m128_f32[0] * m_scale.m128_f32[0]) - m_offset.x;
				if (x < m_pos.m128_f32[0]) {
					m_pos.m128_f32[0] = x;
					AdjustOutline();
				}
			}
			break;
		case Align::BOTTOM_LEFT:
			{
				auto w = m_font->MeasureString(m_drawString.c_str(), false);

				m_pos.m128_f32[1] = m_bufferSize.y - (w.m128_f32[1] * m_scale.m128_f32[1]) - m_offset.y;

				AdjustOutline();
			}
			break;
		case Align::BOTTOM_RIGHT:
			{
				auto w = m_font->MeasureString(m_drawString.c_str(), false);

				auto x = m_bufferSize.x - (w.m128_f32[0] * m_scale.m128_f32[0]) - m_offset.x;
				if (x < m_pos.m128_f32[0]) {
					m_pos.m128_f32[0] = x;
				}
				m_pos.m128_f32[1] = m_bufferSize.y - (w.m128_f32[1] * m_scale.m128_f32[1]) - m_offset.y;

				AdjustOutline();
			}
			break;
		}
	}

	void StatsRenderer::AdjustOutline()
	{
		m_outlinePos[0] = _mm_add_ps(m_pos, m_outlineSize);
		m_outlinePos[1] = _mm_addsub_ps(m_pos, m_outlineSize);
		m_outlinePos[2] = _mm_sub_ps(m_pos, m_outlineSize);
		m_outlinePos[3].m128_f32[0] = m_pos.m128_f32[0] + m_outlineSize.m128_f32[0];
		m_outlinePos[3].m128_f32[1] = m_pos.m128_f32[1] - m_outlineSize.m128_f32[1];
	}

	const wchar_t* DOSD::StatsRendererCallback_FPS()
	{
		::_snwprintf_s(m_Instance.m_bufStats1, _TRUNCATE,
			L"FPS: %.1Lf",
			1.0L / (m_Instance.m_stats.cur.frametime / 1000000.0L));
		return m_Instance.m_bufStats1;
	}

	const wchar_t* DOSD::StatsRendererCallback_SimpleFPS()
	{
		::_snwprintf_s(m_Instance.m_bufStats1, _TRUNCATE,
			L"%.1Lf",
			1.0L / (m_Instance.m_stats.cur.frametime / 1000000.0L));

		return m_Instance.m_bufStats1;
	}

	const wchar_t* DOSD::StatsRendererCallback_Frametime()
	{
		::_snwprintf_s(m_Instance.m_bufStats2, _TRUNCATE,
			L"Frametime: %.2Lf ms",
			m_Instance.m_stats.cur.frametime / 1000.0L);

		return m_Instance.m_bufStats2;
	}

	const wchar_t* DOSD::StatsRendererCallback_SimpleFrametime()
	{
		::_snwprintf_s(m_Instance.m_bufStats2, _TRUNCATE,
			L"%.2Lf",
			m_Instance.m_stats.cur.frametime / 1000.0L);

		return m_Instance.m_bufStats2;
	}

	const wchar_t* DOSD::StatsRendererCallback_Counter()
	{
		::_snwprintf_s(m_Instance.m_bufStats3, _TRUNCATE,
			L"Frames: %llu",
			m_Instance.m_stats.frameCounter);

		return m_Instance.m_bufStats3;
	}

	const wchar_t* DOSD::StatsRendererCallback_VRAM()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO info;
		if (SUCCEEDED(m_Instance.m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, std::addressof(info)))) {
			::_snwprintf_s(m_Instance.m_bufStats4, _TRUNCATE,
				L"VRAM: %llu/%llu",
				info.CurrentUsage / (1024 * 1024),
				info.Budget / (1024 * 1024));
		} else {
			::_snwprintf_s(m_Instance.m_bufStats4, _TRUNCATE, L"VRAM: QUERY ERROR");
		}

		return m_Instance.m_bufStats4;
	}

	void DOSD::Present_Pre(IDXGISwapChain*)
	{
		if (!m_Instance.m_statsRenderer.get())
			return;

		if (!m_Instance.m_stats.draw)
			return;

		if (m_Instance.m_stats.warmup)
			return;

		m_Instance.m_statsRenderer->DrawStrings();
	}

	void DOSD::Present_Post(IDXGISwapChain* a_swapChain)
	{
		if (!m_Instance.m_statsRenderer.get())
			return;

		m_Instance.m_stats.frameCounter++;

		if (!m_Instance.m_stats.draw) {
			return;
		}

		auto e = IPerfCounter::Query();
		auto deltaT = IPerfCounter::delta_us(m_Instance.m_stats.lastUpdate, e);

		if (m_Instance.m_stats.warmup || deltaT >= m_Instance.m_stats.interval) {
			auto deltaFC =
				m_Instance.m_stats.frameCounter -
				m_Instance.m_stats.lastFrameCount;

			m_Instance.m_stats.cur.frametime =
				static_cast<long double>(deltaT) / static_cast<long double>(deltaFC);

			m_Instance.m_stats.lastFrameCount =
				m_Instance.m_stats.frameCounter;
			m_Instance.m_stats.lastUpdate = e;

			if (m_Instance.m_stats.warmup) {
				m_Instance.m_stats.warmup--;
			}

			m_Instance.m_statsRenderer->Update();
		}
	}

	void DOSD::Initialize(D3D11CreateEventPost* a_data)
	{
		try {
			if (m_statsRenderer.get()) {
				throw std::exception("Already initialized");
			}

			if (m_conf.scale_to_window) {
				RECT rect;
				if (::GetClientRect(a_data->m_pSwapChainDesc->OutputWindow, std::addressof(rect)) == TRUE) {
					float ww = static_cast<float>(rect.right - rect.left);
					float wh = static_cast<float>(rect.bottom - rect.top);
					float bw = static_cast<float>(a_data->m_pSwapChainDesc->BufferDesc.Width);
					float bh = static_cast<float>(a_data->m_pSwapChainDesc->BufferDesc.Height);

					float ws = bw / ww;
					float hs = bh / wh;

					m_stats.scale.x *= ws;
					m_stats.scale.y *= hs;
				}
			}

			auto renderer = std::make_unique<StatsRenderer>(
				a_data->m_pDevice, a_data->m_pImmediateContext,
				a_data->m_pSwapChainDesc->BufferDesc.Width,
				a_data->m_pSwapChainDesc->BufferDesc.Height,
				m_stats.offset,
				m_conf.outline_size,
				m_conf.align,
				m_stats.scale,
				m_stats.colors.font,
				m_stats.colors.outline);

			bool res = false;
			bool isCustom = false;

			if (!m_conf.font_file.empty()) {
				std::wostringstream ss;
				ss << OSD_FONT_PATH << StrHelpers::ToWString(m_conf.font_file);

				auto file = ss.str();

				logger::info("[OSD] Loading OSD font");

				if (!(res = renderer->Load(file.c_str()))) {
					logger::info("[OSD] Couldn't load font, falling back to built-in");
				} else {
					isCustom = true;
				}
			}

			// if (!res) {
			// res = renderer->Load(
			// ConfigGetFontResource(m_conf.font));
			// }

			if (res) {
				if (m_conf.barefps && !m_conf.all) {
					renderer->AddCallback(StatsRendererCallback_SimpleFPS);
				} else if (m_conf.fps || m_conf.all) {
					renderer->AddCallback(StatsRendererCallback_FPS);
				}

				if (m_conf.bareframetime && !m_conf.all) {
					renderer->AddCallback(StatsRendererCallback_SimpleFrametime);
				} else if (m_conf.frametime || m_conf.all) {
					renderer->AddCallback(StatsRendererCallback_Frametime);
				}

				if (m_conf.counter || m_conf.all) {
					renderer->AddCallback(StatsRendererCallback_Counter);
				}

				if (m_conf.vram || m_conf.all) {
					if (SUCCEEDED(a_data->m_pAdapter->QueryInterface(IID_PPV_ARGS(m_adapter.GetAddressOf())))) {
						renderer->AddCallback(StatsRendererCallback_VRAM);
					} else {
						logger::info("[OSD] Failed to get IDXGIAdapter3, DXGI 1.4 or later is required to display VRAM usage info");
					}
				}

				m_statsRenderer = std::move(renderer);
			} else {
				logger::info("[OSD] Couldn't load OSD renderer");
			}
		} catch (const std::exception& e) {
			logger::info("[OSD] Exception thrown while initializing stats renderer: {}", e.what());
		}
	}

	void DOSD::OnD3D11PostCreate_OSD(Event code, void* data)
	{
		auto info = reinterpret_cast<D3D11CreateEventPost*>(data);
		m_Instance.Initialize(info);
	}

	void DOSD::OnD3D11PostPostCreate_OSD(Event code, void* data)
	{
		if (!m_Instance.m_statsRenderer.get()) {
			return;
		}

		if (!m_Instance.m_statsRenderer->IsLoaded())
			return;

		if (m_Instance.m_conf.font_autoscale) {
			auto numCallbacks = m_Instance.m_statsRenderer->GetNumCallbacks();
			if (numCallbacks == 1) {
				m_Instance.m_statsRenderer->MulScale(0.7f);
			}
			if (numCallbacks > 2) {
				m_Instance.m_statsRenderer->MulScale(0.6f);
			} else if (numCallbacks > 1) {
				m_Instance.m_statsRenderer->MulScale(0.7f);
			}
		}
	}
}
