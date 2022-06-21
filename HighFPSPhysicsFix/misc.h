#pragma once
namespace SDT
{
    namespace Structures {

        class _SettingCollectionList  {
        public:
            virtual void Unk_01();
            virtual void Unk_02();
            virtual void Unk_03();
            virtual void Unk_04();
            virtual void Unk_05();
            virtual void Unk_06();
            virtual void Unk_07();
            virtual void Unk_08();
            virtual void Unk_09();
            virtual void LoadINI();

            char inipath[260]; // MAX_PATH
            std::uint8_t pad10C[12];
            SettingCollectionList::Node data;

            void LoadIni_Hook();
        };
       //static_assert(offsetof(_SettingCollectionList, data) == 0x118);
    }

    class IPluginInfo;

    class DMisc :
        public IDriver,
        IConfig
    {
    public:
        static inline constexpr auto ID = DRIVER_ID::MISC;

        struct {
            bool skipmissingini;
            static inline std::int32_t numthreads;
            bool fixstuttering;
            bool fixwhitescreen;
            bool fixwindspeed;
            bool fixrotspeed;
            bool fixsitrotspeed;
            bool fixwsrotspeed;
            bool fixloadmodel;
            bool fixstuckanim;
            bool fixresponsive;
            bool fixobcpspeed;
        }static inline m_conf;
        float SittingRotSpeedX;
        float SittingRotSpeedY;
        static inline HANDLE f4handle = NULL;
        static inline std::int32_t nMaxProcessorMaskNG;
        static inline std::int32_t nMaxProcessorAfterLoad;
        static void SetThreadsNG();
        static void ReturnThreadsNG();
        static inline bool LimitThreads = false;

        FN_NAMEPROC("Miscellaneous");
        FN_ESSENTIAL(false);
        FN_DRVDEF(6);
    private:
        DMisc();

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        
        void Patch_SetThreadsNG();

        static DMisc m_Instance;
    };
}
