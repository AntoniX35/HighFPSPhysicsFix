#pragma once

#define WMSG_INVOKELOCK		"mfd_invokelock"

namespace SDT
{
    class MsgProc
    {
        typedef std::function<void(HWND, UINT, WPARAM, LPARAM)> MsgProcFunc;
        typedef std::unordered_map<UINT, std::vector<MsgProcFunc>> MPMap;
    public:
        typedef std::vector<UINT> MsgList;

        void Add(UINT msg, const MsgProcFunc& f)
        {
            m_map.try_emplace(msg).first->second.emplace_back(f);
        }

        void Add(const MsgList& l, const MsgProcFunc& f)
        {
            for (const auto &msg : l)
                Add(msg, f);
        }

        inline bool HasProcessors() const noexcept
        {
            return !m_map.empty();
        }

        void Process(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            auto it = m_map.find(uMsg);
            if (it != m_map.end()) {
                for (const auto& f : it->second) {
                    f(hWnd, uMsg, wParam, lParam);
                }
            }
        }
    private:
        MPMap m_map;
    };

    class MonitorInfo
    {

        struct FindDesc
        {
            FindDesc() : found(false) {}

            bool found;
            HMONITOR handle;
        };

    public:

        static bool GetPrimary(HMONITOR& a_handle);

    private:

        static BOOL CALLBACK FindPrimary(HMONITOR, HDC, LPRECT, LPARAM);

    };

    class DWindow :
        public IDriver,
        IConfig
    {
        typedef HWND (WINAPI* CreateWindowExA_fn_t)(
            _In_ DWORD dwExStyle,
            _In_opt_ LPCSTR lpClassName,
            _In_opt_ LPCSTR lpWindowName,
            _In_ DWORD dwStyle,
            _In_ int X,
            _In_ int Y,
            _In_ int nWidth,
            _In_ int nHeight,
            _In_opt_ HWND hWndParent,
            _In_opt_ HMENU hMenu,
            _In_opt_ HINSTANCE hInstance,
            _In_opt_ LPVOID lpParam);

        typedef BOOL
        (WINAPI* GetClientRect_fn_t)(
            _In_ HWND hWnd,
            _Out_ LPRECT lpRect);

    public:
        static inline constexpr auto ID = DRIVER_ID::WINDOW;

        struct {
            int* iLocationX = nullptr;
            int* iLocationY = nullptr;
        }static inline m_gv;

        struct {
            bool disable_ghosting;
            bool lock_cursor;
            bool force_minimize;
            bool upscale;
            bool center_window;
            int offset_x;
            int offset_y;
        }static inline m_conf;

        FN_NAMEPROC("Window");
        FN_ESSENTIAL(false);
        FN_DRVDEF(4);
    private:

        typedef BOOL
        (WINAPI* GetClientRect_pfn)(
            _In_ HWND hWnd,
            _Out_ LPRECT lpRect);

        static HWND WINAPI CreateWindowExA_Hook(
            _In_ DWORD dwExStyle,
            _In_opt_ LPCSTR lpClassName,
            _In_opt_ LPCSTR lpWindowName,
            _In_ DWORD dwStyle,
            _In_ int X,
            _In_ int Y,
            _In_ int nWidth,
            _In_ int nHeight,
            _In_opt_ HWND hWndParent,
            _In_opt_ HMENU hMenu,
            _In_opt_ HINSTANCE hInstance,
            _In_opt_ LPVOID lpParam);

        void DoUpscale(HWND a_windowHandle, int& X, int& Y, int& nWidth, int& nHeight);
        void DoCenter(HWND a_windowHandle, int& X, int& Y, int nWidth, int nHeight);

        CreateWindowExA_fn_t m_createWindowExA_O;
        GetClientRect_fn_t m_getClientRect_O;

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        void SetupCursorLockMP();
        void SetupForceMinimizeMP();

        bool SetCursorLock(HWND hwnd);
        void CaptureCursor(HWND hwnd, bool sw);

        struct {
            bool resized = false;
            HWND hWnd = nullptr;
            RECT MonitorRes, resolution;
        } m_upscaling;

        WNDPROC pfnWndProc;

        static void OnD3D11PreCreate_Upscale(Event code, void* data);
        static void PostConfigLoad(Event code, void* data);

        static BOOL WINAPI
            GetClientRect_Hook(
                _In_ HWND hWnd,
                _Out_ LPRECT lpRect);

        static LRESULT CALLBACK WndProc_Hook(
            HWND hWnd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam);

        MsgProc m_mp;

        static DWindow m_Instance;
    };
}