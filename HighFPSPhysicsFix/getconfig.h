#pragma once
#include <string>

namespace SDT
{
    class IConfig
    {
    public:
        static int LoadConfiguration();
        static void ClearConfiguration();

        __forceinline static bool IsCustomLoaded() {
            return m_confReaderCustom.ParseError() == 0;
        }

        __forceinline const std::string GetConfigValue(const char* section, const char* key, const char* default) const
        {
            if (m_confReaderCustom.ParseError() == 0 && m_confReaderCustom.HasValue(section, key))
            {
                return m_confReaderCustom.Get(section, key, default);
            }
            return m_confReader.Get(section, key, default);
        }

        __forceinline float GetConfigValueFloat(const char* section, const char* key, float default) const
        {
            if (m_confReaderCustom.ParseError() == 0 && m_confReaderCustom.HasValue(section, key))
            {
                return m_confReaderCustom.GetFloat(section, key, default);
            }
            return m_confReader.GetFloat(section, key, default);
        }
        __forceinline float GetINIFloat(const char* section, const char* key, float default) const
        {
            if (m_confReaderGameCustom.ParseError() == 0 && m_confReaderGameCustom.HasValue(section, key))
            {
                return m_confReaderGameCustom.GetFloat(section, key, default);
            }
            if (m_confReaderGamePrefs.ParseError() == 0 && m_confReaderGamePrefs.HasValue(section, key))
            {
                return m_confReaderGamePrefs.GetFloat(section, key, default);
            }
            return m_confReaderGame.GetFloat(section, key, default);
        }

        __forceinline double GetConfigValueDouble(const char* section, const char* key, double default) const
        {
            if (m_confReaderCustom.ParseError() == 0 && m_confReaderCustom.HasValue(section, key))
            {
                return m_confReaderCustom.GetReal(section, key, default);
            }
            return m_confReader.GetReal(section, key, default);
        }

        __forceinline bool GetConfigValueBool(const char* section, const char* key, bool default) const
        {
            if (m_confReaderCustom.ParseError() == 0 && m_confReaderCustom.HasValue(section, key))
            {
                return m_confReaderCustom.GetBoolean(section, key, default);
            }
            return m_confReader.GetBoolean(section, key, default);
        }

        __forceinline bool GetGameConfigValueBool(const char* section, const char* key, bool default) const
        {
            if (m_confReader.ParseError() == 0 && m_confReader.HasValue(section, key))
            {
                return m_confReader.GetBoolean(section, key, default);
            }
            return m_confReaderGame.GetBoolean(section, key, default);
        }

        __forceinline long GetConfigValueInteger(const char* section, const char* key, long default) const
        {
            if (m_confReaderCustom.ParseError() == 0 && m_confReaderCustom.HasValue(section, key))
            {
                return m_confReaderCustom.GetInteger(section, key, default);
            }
            return m_confReader.GetInteger(section, key, default);;
        }

        template <typename T, typename = std::enable_if_t<!std::is_same_v<T, bool> && (std::is_integral_v<T> || std::is_enum_v<T>) && std::is_convertible_v<T, long>>>
        T GetConfigValue(const std::string& section, const std::string& key, T default) const
        {
            return static_cast<T>(m_confReader.GetInteger(section, key, static_cast<long>(default)));
        }

        __forceinline bool HasConfigValue(const char* section, const char* key) const
        {
            return Exists(section, key);
        }

    private:

        bool Exists(const char* a_section, const char* a_key) const
        {
            if (m_confReaderCustom.ParseError() == 0)
            {
                if (m_confReaderCustom.HasValue(a_section, a_key))
                    return true;
            }

            return m_confReader.HasValue(a_section, a_key);
        }

        static INIReader m_confReader;
        static INIReader m_confReaderCustom;
        static INIReader m_confReaderGame;
        static INIReader m_confReaderGameCustom;
        static INIReader m_confReaderGamePrefs;
    };

    class IConfigGame:
        public IConfig
    {
    public:
        IConfigGame(const char* a_path) :
            m_path(a_path),
            m_attemptedLoad(false)
        {
        }
        template <class T>
        bool Get(const char* a_section, const char* a_key, bool a_default, T& a_out)
        {
            if (m_reader.ParseError() != 0)
            {
                if (!m_attemptedLoad) {
                    m_attemptedLoad = true;
                    Load();

                    if (m_reader.ParseError() != 0) {
                        return false;
                    }
                }
                else {
                    return false;
                }
            }

            bool valstr = m_reader.HasValue(a_section, a_key);
            if (valstr)
            {
                a_out = m_reader.GetBoolean(a_section, a_key, a_default);
                return true;
            }

            return false;
        }


    private:

        void Load()
        {
            auto base = std::make_unique<char[]>(MAX_PATH);

            HRESULT hr = ::SHGetFolderPathA(
                nullptr,
                CSIDL_MYDOCUMENTS | CSIDL_FLAG_CREATE,
                nullptr,
                SHGFP_TYPE_CURRENT,
                base.get());

            if (SUCCEEDED(hr))
            {
                std::filesystem::path file;

                file = base.get();
                file /= m_path;

                m_reader.Load(file.string());
            }
        }

        std::string m_path;
        bool m_attemptedLoad;
        INIReader m_reader;
    };


}