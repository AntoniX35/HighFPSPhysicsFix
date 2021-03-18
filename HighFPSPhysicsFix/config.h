#pragma once
#include "f4se_common/f4se_version.h"

//-----------------------
// Plugin Information
//-----------------------
#define PLUGIN_VERSION              14
#define PLUGIN_VERSION_STRING       "0.8.1"
#define PLUGIN_NAME_SHORT           "HighFPSPhysicsFix"
#define PLUGIN_NAME_LONG            "High FPS Physics Fix"
#define SUPPORTED_RUNTIME_VERSION   CURRENT_RELEASE_RUNTIME
#define MINIMUM_RUNTIME_VERSION     RUNTIME_VERSION_1_10_163
#define COMPATIBLE(runtimeVersion)  (runtimeVersion == SUPPORTED_RUNTIME_VERSION)

#define PLUGIN_BASE_PATH            "Data\\F4SE\\Plugins\\"

static inline constexpr const char* PLUGIN_INI_FILE = PLUGIN_BASE_PATH PLUGIN_NAME_SHORT ".ini";