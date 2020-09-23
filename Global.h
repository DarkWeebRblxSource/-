#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <curl.h>
#include <json.h>
#include <ctime>
#include <psapi.h>
#include <fstream>
#include <streambuf>
extern "C"
{
#include "Lua/lua.h"
#include "Lua/lualib.h"
#include "Lua/lauxlib.h"
#include "Lua/luaconf.h"
#include "Lua/lapi.h"
#include "Lua/lualib.h"
#include "Lua/lobject.h"
#include "Lua/lstate.h"
#include "Lua/ldo.h"
}

#define aslr(x) (x - 0x400000 + (DWORD)GetModuleHandle(0))
#define noaslr(x) (x + 0x400000 - (DWORD)GetModuleHandle(0))

//Settings
#define DebugAll true
#define DebugWrapping false
#define DebugCalls false
#define DebugMetaMethods false
#define DebugHacks false
#define ConsoleOpen true
#define ConsoleInput true
#define PauseWithError true
#define UpdateMethod 1 // 1=XHosted 2=Addy file