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
            bool fixocbpspeed;
        }static inline m_conf;
        static inline uintptr_t SkipNoINI, SetThreadsNGAddress, ReturnThreadsNGAddress, ReturnThreadsNGJMPAddress, FixCPUThreadsJMP1Address, FixCPUThreadsJMP2Address, FixStuttering2Address, FixStuttering3Address,
            FixWhiteScreenAddress, FixWindSpeed1Address, FixWindSpeed2Address, FixRotationSpeedAddress, FixLockpickRotationAddress, FixWSRotationSpeedAddress, FixRepeateRateAddress, FixStuckAnimAddress, FixMotionFeedBackAddress,
            SittingRotationSpeedXAddress, FixSittingRotationXAddress, CalculateOCBP1Address;
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
        void* TrampHook64(void* src, void* dst, int len)
        {
            if (len < 14) return nullptr;

            BYTE stub[14] = {
            0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,				// jmp qword ptr instruction
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 8 byte ptr to jmp destination
            };

            void* pTrampoline = VirtualAlloc(0, len + sizeof(stub), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            DWORD oldProtect = 0;
            VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &oldProtect);

            uintptr_t jmpBackAddr = (uintptr_t)src + len;

            // copy trampoline jmpback addr to stub
            memcpy(stub + 6, &jmpBackAddr, 8);
            // copy stolen bytes to trampoline
            memcpy((void*)(pTrampoline), src, len);
            // copy stub to trampoline
            memcpy((void*)((uintptr_t)pTrampoline + len), stub, sizeof(stub));

            // copy dst to the stub, creating our jmp to our hook function
            memcpy(stub + 6, &dst, 8);
            // copy new stub to src
            memcpy(src, stub, sizeof(stub));

            // nop any stolen bytes in src
            for (int i = 14; i < len; i++)
            {
                *(BYTE*)((uintptr_t)src + i) = 0x90;
            }

            VirtualProtect(src, len, oldProtect, &oldProtect);
            return (void*)(pTrampoline);
        }
    };
}
