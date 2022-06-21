#include "pch.h"
namespace SDT
{
	INIReader IConfig::m_confReader;
	INIReader IConfig::m_confReaderCustom;
	INIReader IConfig::m_confReaderGame;
	INIReader IConfig::m_confReaderGameCustom;
	INIReader IConfig::m_confReaderGamePrefs;

	int IConfig::LoadConfiguration()
	{
		m_confReader.Load(PLUGIN_INI_FILE);
		m_confReaderCustom.Load(PLUGIN_INI_CUSTOM_FILE);
		m_confReaderGame.Load(FALLOUT4_INI_FILE);
		m_confReaderGameCustom.Load(FALLOUT4_CUSTOM_INI_FILE);
		m_confReaderGamePrefs.Load(FALLOUT4_PREFS_INI_FILE);

		return m_confReader.ParseError();
	}

	void IConfig::ClearConfiguration()
	{
		m_confReader.Clear();
		m_confReaderCustom.Clear();
		m_confReaderGame.Clear();
	}
}