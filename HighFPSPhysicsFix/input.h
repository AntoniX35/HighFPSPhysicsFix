#pragma once

namespace SDT
{
    enum KeyEvent
    {
        KeyDown = 0,
        KeyUp = 1
    };

    typedef void (*KeyEventCallback)(KeyEvent, UInt32);

    class KeyEventHandler
    {
    public:
    };

    class ComboKeyPressHandler
    {
    public:
        ComboKeyPressHandler() :
            m_comboKey(0),
            m_key(0),
            m_comboKeyDown(false)
        {}

        __forceinline void SetComboKey(UInt32 a_key) {
            m_comboKey = a_key;
            m_comboKeyDown = false;
        }

        __forceinline void SetKey(UInt32 a_key) {
            m_key = a_key;
        }

        __forceinline void SetKeys(UInt32 a_comboKey, UInt32 a_key) {
            m_comboKey = a_comboKey;
            m_key = a_key;
            m_comboKeyDown = false;
        }

    protected:

    private:

        bool m_comboKeyDown;

        UInt32 m_comboKey;
        UInt32 m_key;

    };

    class DInput :
        public IDriver,
        IConfig
    {

    public:
        static inline constexpr auto ID = DRIVER_ID::INPUT;

        static void RegisterForKeyEvents(KeyEventHandler* const handler);

        FN_NAMEPROC("Input");
        FN_ESSENTIAL(false);
        FN_DRVDEF(1);
    private:
        DInput() = default;

        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        static void MessageHandler(Event m_code, void* a_args);

        std::vector<KeyEventHandler*> m_handlers;

        static DInput m_Instance;
    };
}