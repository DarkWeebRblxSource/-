#pragma once
#include "Global.h"

static TValue* index2adr(lua_State* L, int idx);

namespace Env
{
	extern DWORD RobloxState;
	extern lua_State* GL;
	bool StartUp();
	bool StateWork();
	bool Execute(std::string Script);
	void Wrap(lua_State* L, DWORD RL, int Idx);
	void Wrap(DWORD RL, lua_State* L, int Idx);
	int RCalling(DWORD RL);
	int Calling(lua_State* L);
	int Resume(lua_State* L);
	int ResumeR(DWORD RL);
	void WrapGlobals();
	bool StateWork();

	//Custom functions
	int getrawmetatable(lua_State* L);
	int GetGenv(lua_State* L);
	int HttpGet(lua_State* L);
	int ReadFile(lua_State* L);
	int setreadonly(lua_State* L);
	int isreadonly(lua_State* L);
	int GetRenv(lua_State* L);
	int newcclosure(lua_State* L);
	int GetReg(lua_State* L);

	//MetaMethods
	int Index(lua_State* L);
	int NewIndex(lua_State* L);
	int GC(lua_State* L);
	int Len(lua_State* L);

	void FuncWrap(DWORD Thread, const char* LibName, const char* Name);
	int Decider(lua_State* L);
}