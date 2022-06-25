#ifndef PCH_H
#define PCH_H

#include <common/IMemPool.h>

#include <f4se_common/SafeWrite.h>
#include <f4se_common/BranchTrampoline.h>

#include <f4se/PluginAPI.h>
#include <xbyak/xbyak.h>

#include <f4se/ICommon.h>
#include <f4se/IHook.h>
#include <f4se/Patching.h>
#include <f4se/JITASM.h>
#include <f4se/ITasks.h>
#include <f4se/StrHelpers.h>
#include <f4se/ID3D11.h>
#include <f4se/INIReader.h>
#include <f4se/GameData.h>
#include <f4se/GameEvents.h>
#include <f4se/GameMenus.h>
#include <f4se/GameSettings.h>
#include "f4se/InputMap.h"

#include <string>
#include <vector>
#include <sstream> 
#include <algorithm>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <filesystem>

#include <shlobj.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <Inc/SpriteFont.h>
#include <Inc/CommonStates.h>

#include "resource.h"
#include "config.h"
#include "helpers.h"
#include "common.h"
#include "stats.h"
#include "f4se.h"
#include "game.h"
#include "getconfig.h"
#include "drv_ids.h"
#include "drv_base.h"
#include "dispatcher.h"
#include "events.h"
#include "input.h"
#include "render.h"
#include "osd.h"
#include "dllmain.h"
#include "window.h"
#include "papyrus.h"
#include "misc.h"

#endif //PCH_H