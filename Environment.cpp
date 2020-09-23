#include "Environment.h"
#include "CallCheck.h"
#include "XHosted/XHosted.h"
#include "Tools.h"

namespace Env
{
	DWORD RobloxState;
	lua_State* GL;
	bool StartUp()
	{
		if (XHosted::Run(XHosted::GetVersion()))
		{
			RobloxState = XHosted::GetRState();
			XHosted::SetLevel(RobloxState, 6);
			if (!StateWork())
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		return false;
	}
	bool StateWork()
	{
		GL = lua_open();
		if (GL)
		{
			luaL_openlibs(GL);
			WrapGlobals();
			CCheck::VehHandlerpush();
			GL->RL = RobloxState;
			lua_register(GL, "getgenv", GetGenv);
			lua_register(GL, "getrenv", GetRenv);
			lua_register(GL, "getreg", GetReg);
			lua_register(GL, "readfile", ReadFile);
			lua_register(GL, "getrawmetatable", getrawmetatable);
			lua_register(GL, "setreadonly", setreadonly);
			lua_register(GL, "isreadonly", isreadonly);
			lua_register(GL, "HttpGet", HttpGet);
			lua_register(GL, "newcclosure", newcclosure);
			lua_newtable(GL);
			lua_setglobal(GL, "_G");

			DWORD Holder = r_lua_newthread(RobloxState);
			r_luaL_ref(RobloxState, LUA_REGISTRYINDEX);

			FuncWrap(Holder, "", "ipairs");
			FuncWrap(Holder, "", "pairs");
			FuncWrap(Holder, "", "rawget");
			FuncWrap(Holder, "", "rawset");
			FuncWrap(Holder, "table", "foreach");
			FuncWrap(Holder, "table", "foreachi");
			FuncWrap(Holder, "table", "insert");
			FuncWrap(Holder, "table", "concat");
			FuncWrap(Holder, "table", "remove");
			FuncWrap(Holder, "table", "sort");
			FuncWrap(Holder, "table", "getn");
			FuncWrap(Holder, "table", "setn");
			FuncWrap(Holder, "table", "maxn");
			FuncWrap(Holder, "", "next");
			FuncWrap(Holder, "", "tostring");
			FuncWrap(Holder, "", "unpack");
			FuncWrap(Holder, "", "setmetatable");
			FuncWrap(Holder, "", "getmetatable");
			FuncWrap(Holder, "", "getfenv");
			FuncWrap(Holder, "", "setfenv");

#if DebugAll
			printf("Loaded\n");
#endif
			//Env::Execute("local a=getrawmetatable(game).__index; setreadonly(getrawmetatable(game),false) getrawmetatable(game).__index=function(b,c) if c=='HttpGet'then return HttpGet else return a(b,c) end end");
			//Env::Execute(Tools::ReadWebsite("https://cdn.discordapp.com/attachments/703679178062561461/711826479452913664/DEX_V4.txt"));
			//Env::Execute(Tools::ReadWebsite("https://pastebin.com/raw/0B97qKwz"));
			//Env::Execute(Tools::ReadWebsite("https://raw.githubusercontent.com/EdgeIY/infiniteyield/master/source"));
			//Env::Execute(Tools::ReadWebsite("https://cdn.discordapp.com/attachments/703679178062561461/711819863701848102/darkdex.txt"));

			return true;
		}
		else
		{
			return false;
		}
	}
	int ResumeR(DWORD RL)
	{
#if DebugAll || DebugCalls
		printf("Attempting to resume thread\n");
#endif
		DWORD RefThread = r_lua_tonumber(RL, lua_upvalueindex(1), NULL);
		lua_State* Thread = (lua_State*)(int)r_lua_tonumber(RL, lua_upvalueindex(2), NULL);
		int NArgs = r_lua_gettop(RL);
		for (int arg = 1; arg <= NArgs; ++arg)
			Wrap(RL, Thread, arg);
		int Return = lua_resume(Thread, NArgs);
		lua_settop(Thread, 0);
		luaL_unref(GL, LUA_REGISTRYINDEX, RefThread);
		return Return;
	}

	void r_lua_pushobject(DWORD RL, DWORD Obj, DWORD Type)
	{//Top offset goes here.
		DWORD Offset = XHosted::JSON["luastate"]["offsets"]["Top"]["offset"].asInt();//Change to allow Both...later
		TValue*& Top = *(TValue**)(RL + Offset);
		Top->tt = Type;
		Top->value.p = (void*)Obj;
		(Top)++;
	}

	int RCalling(DWORD RL)
	{
		lua_State* Thread = lua_newthread(GL);
		DWORD RefThread = luaL_ref(GL, LUA_REGISTRYINDEX);
		DWORD Ref = r_lua_tonumber(RL, lua_upvalueindex(1), NULL);
		if (Ref == 0)
		{
			DWORD CI = *(DWORD*)(RL + 16);
			DWORD Env = *(DWORD*)(**(DWORD**)(CI + 12) + 16);
			r_lua_pushobject(RL, Env, R_LUA_TTABLE);
			r_lua_getfield(RL, -1, "__func");
			Ref = r_lua_tonumber(RL, -1, NULL);
			r_lua_pop(RL, 2);
		}
		lua_rawgeti(Thread, LUA_REGISTRYINDEX, Ref);
		DWORD TypeCheck = lua_type(Thread, -1);
#if DebugAll || DebugCalls
		printf("V->Calling: %d, TypeCheck: %d\n", Ref, TypeCheck);
#endif
		if (TypeCheck != LUA_TFUNCTION)
		{
#if DebugAll || DebugCalls
			printf("Failed TypeCheck\n");
#endif
			luaL_unref(GL, LUA_REGISTRYINDEX, RefThread);
			return 0;
		}
		DWORD Args = r_lua_gettop(RL);
		luaD_checkstack(Thread, Args);
		if (Args != 0)
		{
#if DebugAll || DebugCalls
			printf("Wrapping Args\n");
#endif
			for (int arg = 1; arg <= Args; ++arg)
				Wrap(RL, Thread, arg);
		}
#if DebugAll || DebugCalls
		printf("Calling Function\n");
#endif
		DWORD Result = lua_pcall(Thread, Args, LUA_MULTRET, 0);
		r_lua_settop(RL, 0);
		if (!Result)
		{
#if DebugAll || DebugCalls
			printf("Successfully Called\n");
#endif
			DWORD Returns = lua_gettop(Thread);
			if (Returns != 0)
			{
#if DebugAll || DebugCalls
				printf("Wrapping Returns\n");
#endif
				for (int arg = 1; arg <= Returns; ++arg)
					Wrap(Thread, RL, arg);
			}
			lua_settop(Thread, 0);
			luaL_unref(GL, LUA_REGISTRYINDEX, RefThread);
			return Returns;
		}
		else if (Result == 1)
		{
#if DebugAll || DebugCalls
			printf("Thread Yielded\n");
#endif
			r_lua_pushnumber(RL, RefThread);
			r_lua_pushlightuserdata(RL, (void*)Thread);
			r_lua_pushcclosure(RL, CCheck::int3breakpoints[1], NULL, 2, NULL);
			return -1;
		}
		else
		{
			if (lua_type(Thread, -1) == LUA_TSTRING)
			{
#if DebugAll || DebugCalls
				printf("Call Error\n");
#endif
				printf("[Error] %s\n", lua_tostring(Thread, -1));
#if PauseWithError && DebugAll
				system("pause");
#endif
				lua_settop(Thread, 0);
				luaL_unref(GL, LUA_REGISTRYINDEX, RefThread);
				return 0;
			}
		}
		return 0;
	}

	void Wrap(lua_State* L, DWORD RL, int Idx)
	{
		int Type = lua_type(L, Idx);
#if DebugAll || DebugWrapping
		printf("L->RL Type: %s\n", lua_typename(L, Type));
#endif
		switch (Type)
		{
		case LUA_TNIL:
		{
			r_lua_pushnil(RL);
			break;
		}
		case LUA_TBOOLEAN:
		{
			r_lua_pushboolean(RL, lua_toboolean(L, Idx));
			break;
		}
		case LUA_TNUMBER:
		{
			r_lua_pushnumber(RL, lua_tonumber(L, Idx));
			break;
		}
		case LUA_TLIGHTUSERDATA:
		{
			r_lua_pushlightuserdata(RL, lua_touserdata(L, Idx));
			break;
		}
		case LUA_TSTRING:
		{
			r_lua_pushstring(RL, lua_tostring(L, Idx));
			break;
		}
		case LUA_TTHREAD:
		{
			r_lua_pushthread(RL);
			break;
		}
		case LUA_TTABLE:
		{
			lua_pushvalue(L, Idx);
			r_lua_newtable(RL);
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				Wrap(L, RL, -2);
				Wrap(L, RL, -1);
				r_lua_settable(RL, -3);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
			break;
		}
		case LUA_TFUNCTION:
		{
			lua_pushvalue(L, Idx);
			r_lua_pushnumber(RL, luaL_ref(L, LUA_REGISTRYINDEX));
			r_lua_pushcclosure(RL, CCheck::int3breakpoints[0], NULL, 1, NULL);
			break;
		}
		case LUA_TUSERDATA:
		{
			if (!luaL_getmetafield(L, Idx, "__key"))
			{
				void* UD = (void*)lua_touserdata(L, Idx);
				r_lua_pushlightuserdata(RL, UD);
				r_lua_rawget(RL, LUA_REGISTRYINDEX);
				if (r_lua_type(RL, -1) != R_LUA_TUSERDATA)
				{
#if DebugAll || DebugWrapping
					printf("Unable To Find Cached\n");
#endif
					r_lua_pop(RL, 1);
					r_lua_pushnil(RL);
				}
#if DebugAll || DebugWrapping
				else
				{
					printf("Found Cached\n");
				}
#endif
			}
			else
			{
				DWORD Key = lua_tonumber(L, -1);
				lua_pop(L, 1);
				r_lua_rawgeti(RL, LUA_REGISTRYINDEX, Key);
#if DebugAll || DebugWrapping
				printf("Roblox Table Ref: %d\n", Key);
#endif
				if (r_lua_type(RL, -1) != R_LUA_TTABLE)
				{
#if DebugAll || DebugWrapping
					printf("Ref: %d Has Been Dereferenced\n", Key);
#endif
					r_lua_pop(RL, 1);
					r_lua_pushnil(RL);
				}
			}
			break;
		}
		default:
		{
#if DebugAll || DebugWrapping
			printf("Unsupported Type: %d\n", Type);
			r_lua_pushnil(RL);
#endif
		}
		}
	}
	int Resume(lua_State* L)
	{
#if DebugAll || DebugCalls
		printf("Attempting to resume thread\n");
#endif
		DWORD RefThread = lua_tonumber(L, lua_upvalueindex(1));
		lua_State* Thread = lua_tothread(L, lua_upvalueindex(2));
		int NArgs = lua_gettop(L);
		lua_xmove(L, Thread, NArgs);
		int Result =  lua_resume(Thread, NArgs);
		r_luaL_unref(RobloxState, LUA_REGISTRYINDEX, RefThread);
		return Result;
	}

	int Calling(lua_State* L)
	{
		DWORD RThread = r_lua_newthread(RobloxState);
		DWORD RefThread =  r_luaL_ref(RobloxState, LUA_REGISTRYINDEX);
		DWORD Ref = lua_tonumber(L, lua_upvalueindex(1));
		r_lua_rawgeti(RThread, LUA_REGISTRYINDEX, Ref);
		DWORD TypeCheck = r_lua_type(RThread, -1);
#if DebugAll || DebugCalls
		printf("R->Calling: %d, TypeCheck: %d\n", Ref, TypeCheck);
#endif
		if (TypeCheck != R_LUA_TFUNCTION)
		{
#if DebugAll || DebugCalls
			printf("Failed TypeCheck\n");
#endif
			r_luaL_unref(RobloxState, LUA_REGISTRYINDEX, RefThread);
			return 0;
		}
		DWORD Args = lua_gettop(L);
		r_luaD_checkstack(RThread, Args);
		if (Args != 0)
		{
#if DebugAll || DebugCalls
			printf("Wrapping Args\n");
#endif
			for (int arg = 1; arg <= Args; ++arg) 
			{
				Wrap(L, RThread, arg);
			}
		}
		//3
#if DebugAll || DebugCalls
		printf("Calling Function\n");
#endif
		int Result = r_lua_pcall(RThread, Args, LUA_MULTRET, 0);
		lua_settop(L, 0);
		if (!Result)
		{
#if DebugAll || DebugCalls
			printf("Successfully Called\n");
#endif
			DWORD Returns = r_lua_gettop(RThread);
			if (Returns != 0)
			{
#if DebugAll || DebugCalls
				printf("Wrapping Returns\n");
#endif
				for (int arg = 1; arg <= Returns; ++arg)
					Wrap(RThread, L, arg);
			}
			r_lua_settop(RThread, 0);
			r_luaL_unref(RobloxState, LUA_REGISTRYINDEX, RefThread);
			return Returns;
		}
		else
		{
			if (r_lua_type(RThread, -1) == R_LUA_TSTRING)
			{
				if (strcmp(r_lua_tostring(RThread, -1), "attempt to yield across metamethod/C-call boundary") == 0)
				{
					r_lua_pop(RThread, 1);
#if DebugAll || DebugCalls
					printf("Thread Yielded\n");
#endif
					lua_pushnumber(L, RefThread);
					lua_pushthread(L);
					lua_pushcclosure(L, Resume, 2);
					Wrap(L, RThread, -1);
					return lua_yield(L, 0);
				}
#if DebugAll || DebugCalls
				printf("Call Error\n");
#endif
				printf("[Error] %s\n", r_lua_tostring(RThread, -1));
#if PauseWithError && DebugAll
				system("pause");
#endif
			}
#if DebugAll || DebugCalls
			else
			{
				printf("[Error] %s\n", "Unknown Error");
			}
#endif
		}
		r_lua_settop(RThread, 0);
		r_luaL_unref(RobloxState, LUA_REGISTRYINDEX, RefThread);
		return 0;
	}

	void Wrap(DWORD RL, lua_State* L, int Idx)
	{
		int Type = r_lua_type(RL, Idx);
#if DebugAll || DebugWrapping
		printf("RL->L Type: %s\n", r_lua_typename(RL, Type));
#endif
		if (Type == R_LUA_TNIL)
		{
			lua_pushnil(L);
		}
		else if (Type == R_LUA_TBOOLEAN)
		{
			lua_pushboolean(L, r_lua_toboolean(RL, Idx));
		}
		else if (Type == R_LUA_TNUMBER)
		{
			lua_pushnumber(L, r_lua_tonumber(RL, Idx, NULL));
		}
		else if (Type == R_LUA_TSTRING)
		{
			lua_pushstring(L, r_lua_tostring(RL, Idx));
		}
		else if (Type == R_LUA_TTHREAD)
		{
			lua_newthread(L);
		}
		else if (Type == R_LUA_TTABLE)
		{
			r_lua_pushvalue(RL, Idx);
			DWORD TRef = r_luaL_ref(RL, LUA_REGISTRYINDEX);
#if DebugAll || DebugWrapping
			printf("Table Ref: %d\n", TRef);
#endif
			lua_newtable(L);
			TValue* IndexT = index2adr(L, -1);
			IndexT->value.gc->h.isreadonly = (*(DWORD*)r_index2adr(RL, Idx) + 8);
			lua_newtable(L);
			lua_pushnumber(L, TRef);
			lua_pushcclosure(L, Index, 1);
			lua_setfield(L, -2, "__index");
			lua_pushnumber(L, TRef);
			lua_pushcclosure(L, NewIndex, 1);
			lua_setfield(L, -2, "__newindex");
			lua_pushnumber(L, TRef);
			lua_pushcclosure(L, Len, 1);
			lua_setfield(L, -2, "__len");
			lua_pushnumber(L, TRef);
			lua_setfield(L, -2, "__key");
			lua_setmetatable(L, -2);
		}
		else if (Type == R_LUA_TFUNCTION)
		{
			r_lua_pushvalue(RL, Idx);
			DWORD TRef = r_luaL_ref(RL, LUA_REGISTRYINDEX);
#if DebugAll || DebugWrapping
			printf("Function Ref: %d\n", TRef);
#endif
			lua_pushnumber(L, TRef);
			lua_pushcclosure(L, Calling, 1);
		}
		else if (Type == R_LUA_TLIGHTUSERDATA)
		{
			lua_pushlightuserdata(L, (void*)r_lua_touserdata(RL, Idx));
		}
		else if (Type == R_LUA_TUSERDATA)
		{
			r_lua_pushvalue(RL, Idx);
			void* UD = (void*)r_lua_touserdata(RL, -1);
			lua_pushlightuserdata(L, UD);
			lua_rawget(L, LUA_REGISTRYINDEX);
			if (lua_isnil(L, -1))
			{
				r_luaL_getmetafield(RL, -1, "__type");
				std::string Type = Type = r_lua_tostring(RL, -1);
				r_lua_pop(RL, 1);
				lua_pop(L, 1);
				void* NUD = (void*)lua_newuserdata(L, 0);
				if (r_lua_getmetatable(RL, -1))
				{
#if DebugAll || DebugWrapping
					printf("Temporary Metatable Wrap Start-------\n");
#endif
					lua_newtable(L);
					r_lua_pushnil(RL);
					while (r_lua_next(RL, -2))
					{
						Wrap(RL, L, -2);
						Wrap(RL, L, -1);
						lua_settable(L, -3);
						r_lua_pop(RL, 1);
					}
					r_lua_pop(RL, 1);
					lua_pushcfunction(L, GC);
					lua_setfield(L, -2, "__gc");
					lua_setmetatable(L, -2);
#if DebugAll || DebugWrapping
					printf("Temporary Metatable Wrap End-------\n");
#endif
				}
				r_lua_pushlightuserdata(RL, NUD);
				r_lua_pushvalue(RL, -2);
				r_lua_rawset(RL, LUA_REGISTRYINDEX, NULL);
				if (Type == "Instance" || Type == "EnumItem")
 {
					lua_pushlightuserdata(L, UD);
					lua_pushvalue(L, -2);
					lua_rawset(L, LUA_REGISTRYINDEX);
				}
			}
			r_lua_pop(RL, 1);
		}
		else
		{
#if DebugAll || DebugWrapping
			printf("Unsupported Type: %d\n", Type);
#endif
			lua_pushnil(L);
		}
	}
	std::vector<const char*> Globals = {
"printidentity","game","Game","workspace","Workspace","warn", "spawn","Spawn","print"
,"Axes", "BrickColor", "CFrame", "Color3", "ColorSequence","ColorSequenceKeypoint",
"NumberRange","NumberSequence","NumberSequenceKeypoint","PhysicalProperties","Ray",
"Rect","Region3","Region3int16","TweenInfo","UDim","UDim2","Vector2", "shared",
"Vector2int16","Vector3","Vector3int16", "Enum", "Faces", "tick", "utf8",
"Instance","warn","typeof","type","ypcall", "pcall", "tick", "wait", "Wait", "delay", "Delay",
"os", "DockWidgetPluginGuiInfo", "DateTime", "bit32", "Stats", "UserSettings", "PluginManager",
"Version", "CellId", "version", "stats", "settings", "RaycastParams", "elapsedTime",
"PathWaypoint", "Random", "PluginDrag", "ElapsedTime", "require"
	};
	void WrapGlobals()
	{
		for (int i = 0; i < Globals.size(); i++)
		{
			r_lua_getglobal(RobloxState, Globals[i]);
			Wrap(RobloxState, GL, -1);
			if (lua_type(GL, -1) == LUA_TTABLE)
				index2adr(GL, -1)->value.gc->h.cache = 1;
			lua_setglobal(GL, Globals[i]);
			r_lua_pop(RobloxState, 1);
		}
#if DebugAll
		printf("Wrapped All Globals\n");
#endif
	}
	bool Execute(std::string Script)
	{
		lua_State* Thread = lua_newthread(GL);
		Script = "spawn(function() script = Instance.new('LocalScript') script.Disabled = true script.Parent = nil " + Script + " end)";
		luaL_loadbuffer(Thread, Script.c_str(), Script.length(), "@Wrapper");
		if (lua_pcall(Thread, 0, 0, 0))
		{
			printf("[Error] %s\n", lua_tostring(Thread, -1));
#if PauseWithError && DebugAll
			system("pause");
#endif
			return false;
		}
		else
		{
#if DebugAll
			printf("Ran Script\n");
#endif
			return true;
		}
	}
	//MetaMethods
	int GC(lua_State* L)
	{
		void* UD = lua_touserdata(L, 1);
		r_lua_pushlightuserdata(RobloxState, UD);
		r_lua_rawget(RobloxState, LUA_REGISTRYINDEX);
		if (r_lua_type(RobloxState, -1) != R_LUA_TNIL)
		{
			void* NUD = (void*)r_lua_touserdata(RobloxState, -1);
			lua_pushlightuserdata(L, NUD);
			lua_rawget(L, LUA_REGISTRYINDEX);
			if (lua_type(L, -1) != LUA_TNIL)
			{
				lua_pop(L, 1);
				lua_pushlightuserdata(L, NUD);
				lua_pushnil(L);
				lua_rawset(L, LUA_REGISTRYINDEX);
			}
			else
			{
				lua_pop(L, 1);
			}
			r_lua_pop(RobloxState, 1);
			r_lua_pushlightuserdata(RobloxState, UD);
			r_lua_pushnil(RobloxState);
			r_lua_rawset(RobloxState, LUA_REGISTRYINDEX, NULL);
		}
		else
		{
			r_lua_pop(RobloxState, 1);
			r_lua_pushlightuserdata(RobloxState, UD);
			r_lua_pushnil(RobloxState);
			r_lua_rawset(RobloxState, LUA_REGISTRYINDEX, NULL);
		}
		return 0;
	}


	int Index(lua_State* L)//1 is Table and 2 is key? I think I'm right////We don't actually need the table here tho because owo
	{
		DWORD Ref = lua_tonumber(L, lua_upvalueindex(1));
#if DebugAll || DebugMetaMethods
		printf("__index Called On %d\n", Ref);
#endif
		r_lua_rawgeti(RobloxState, LUA_REGISTRYINDEX, Ref);
		DWORD Type = r_lua_type(RobloxState, -1);
		if (Type == R_LUA_TTABLE)
		{
			Wrap(L, RobloxState, 2);
			r_lua_gettable(RobloxState, -2);
			Wrap(RobloxState, L, -1);
			r_lua_pop(RobloxState, 2);
			return 1;
		}
		else
		{
			r_lua_pop(RobloxState, 1);
		}
		return 0;
	}

	int NewIndex(lua_State* L)//1 is Table and 2 is key? I think I'm right
	{
		DWORD Ref = lua_tonumber(L, lua_upvalueindex(1));
#if DebugAll || DebugMetaMethods
		printf("__newindex Called On %d\n", Ref);
#endif
		r_lua_rawgeti(RobloxState, LUA_REGISTRYINDEX, Ref);
		DWORD Type = r_lua_type(RobloxState, -1);
		if (Type == R_LUA_TTABLE)
		{
			Wrap(L, RobloxState, 2);
			Wrap(L, RobloxState, 3);
			r_lua_settable(RobloxState, -3);
			r_lua_pop(RobloxState, 1);
		}
		else
		{
			const char* msg = lua_pushfstring(L, "%s expected, got %s","table", r_lua_typename(RobloxState, -1));
			r_lua_pop(RobloxState, 1);
			return luaL_argerror(L, 1, msg);
		}
		return 0;
	}

	int Len(lua_State* L)
	{
		DWORD Ref = lua_tonumber(L, lua_upvalueindex(1));
#if DebugAll || DebugMetaMethods
		printf("__len Called\n");
#endif
		r_lua_rawgeti(RobloxState, LUA_REGISTRYINDEX, Ref);
		if (r_lua_type(RobloxState, -1) == R_LUA_TTABLE)
		{
			lua_pushnumber(L, r_lua_objlen(RobloxState, -1));
			r_lua_pop(RobloxState, 1);
			return 1;
		}
		else
		{
			r_lua_pop(RobloxState, 1);
			return 0;
		}
	}

	//Custom functions
	int GetRenv(lua_State* L)
	{
		if (lua_gettop(L) != 0)
			throw std::runtime_error("GetGenv function does not accept arguments");
		Wrap(RobloxState, L, LUA_GLOBALSINDEX);
		return 1;
	}

	int GetGenv(lua_State* L)
	{
		if (lua_gettop(L) != 0)
			throw std::runtime_error("GetGenv function does not accept arguments");
		lua_pushvalue(L, LUA_GLOBALSINDEX);
		return 1;
	}

	int GetReg(lua_State* L)
	{
		if (lua_gettop(L) != 0)
			throw std::runtime_error("GetReg function does not accept arguments");
		Wrap(RobloxState, L, LUA_REGISTRYINDEX);
		return 1;
	}

	int HttpGet(lua_State* L)
	{
		DWORD Stack = lua_gettop(L);
		if (Stack > 2 || Stack < 1)
			throw std::runtime_error("HttpGet function requires at least 1 argument");
		if (lua_isstring(L, 1)) 
		{
			lua_pushstring(L, Tools::ReadWebsite(lua_tostring(L, 1)).c_str());
			//Add second argument soon(Add checks)
			return 1;
		}
		else
		{
#if DebugAll
			printf("Attempt to call HttpGet failed: Passed value is not a URL\n");
#endif
			return 0;
		}
	}

	int getrawmetatable(lua_State* L)
	{
#if FunctionNames
		printf("%s Called\n", __FUNCTION__);
#endif
		luaL_checkany(L, 1);
		switch (lua_type(L, 1))
		{
		case LUA_TTABLE:
			if (!luaL_getmetafield(L, 1, "__key"))
			{
				if (lua_getmetatable(L, 1))
				{
					return 1;
				}
				else
				{
					luaL_error(L, "Attempt to call getrawmetatable failed: Passed value does not have a metatable\n");
					return 0;
				}
			}
			else
			{
				DWORD Key = lua_tonumber(L, -1);
				lua_pop(L, 1);
				r_lua_rawgeti(L->RL, LUA_REGISTRYINDEX, Key);
				if (r_lua_getmetatable(L->RL, -1))
				{
					Wrap(L->RL, L, -1);
					r_lua_pop(L->RL, 2);
					return 1;
				}
				else
				{
					luaL_error(L, "Attempt to call getrawmetatable failed: Passed value does not have a metatable\n");
					return 0;
				}
			}
			return 1;
		case LUA_TUSERDATA:
		{
			void* UD = lua_touserdata(L, 1);
			r_lua_pushlightuserdata(L->RL, UD);
			r_lua_rawget(L->RL, LUA_REGISTRYINDEX);
			if (r_lua_type(L->RL, -1) == R_LUA_TUSERDATA)
			{
				if (r_lua_getmetatable(L->RL, -1))
				{
					Wrap(L->RL, L, -1);
					r_lua_pop(L->RL, 2);
					return 1;
				}
				else
				{
					luaL_error(L, "Attempt to call getrawmetatable failed: Passed value does not have a metatable\n");
					return 0;
				}
			}
			else
			{
				r_lua_pop(L->RL, 1);
				lua_getmetatable(L, 1);
				return 1;
			}
		}
		default:
			luaL_checktype(L, 1, LUA_TTABLE);
		}
	}

	int newcclosure(lua_State* L)//THIS FUNCTION IS USELESS ON A WRAPPER
	{//SO I'M JUST GONNA MAKE IT RETURN THE FUNCTION IT IS GIVEN
		//LOVE ME
		//HATE ME
		luaL_checktype(L, 1, LUA_TFUNCTION);
		return 1;
	}

	DWORD GetAddress(DWORD Closure)//Outdated
	{
		return (Closure + 24) ^ *(DWORD*)(Closure + 24);//
	}

	void SetAddress(DWORD Closure, DWORD NewAddress)//I'll tell you how to update tomorrow
	{
		DWORD F = Closure + 24;//(DWORD)NewAddress + (DWORD)F
		*(DWORD*)(F) = F ^ (DWORD)NewAddress;//(*-*) v9 + v13 + 24//
	}

	int HookFunction(lua_State* L)
	{
#if FunctionNames
		printf("%s Called\n", __FUNCTION__);
#endif
		DWORD Key;
		Wrap(L, L->RL, 2);
		Key = (DWORD)lua_topointer(L, 2);
		lua_pop(L, 1);

		//Calc replacement Address
		DWORD Replacement = GetAddress((DWORD)r_lua_topointer(L->RL, -1));
		Wrap(L, L->RL, 1);

		//Create Return
		DWORD Closure2 = (DWORD)r_lua_topointer(L->RL, -1);
		DWORD Old = GetAddress(Closure2);
		r_lua_pushcclosure(L->RL, (DWORD)Old, NULL, 0, NULL);//When this matters. I'll tell you
		Wrap(L->RL, L, -1);
		r_lua_pop(L->RL, 1);

		//Change Primary Function Address
		SetAddress(Closure2, Replacement);
		DWORD New = GetAddress((DWORD)r_lua_topointer(L->RL, -1));

		//Handle UpVal Alternative
		r_lua_newtable(L->RL);
		r_lua_pushstring(L->RL, "__func");
		r_lua_pushnumber(L->RL, Key);
		r_lua_settable(L->RL, -3);
		r_lua_setfenv(L->RL, -2);//Add this
		r_lua_pop(L->RL, 2);
		return 1;
	}

	int setreadonly(lua_State* L)//Table, Bool
	{
		if (lua_gettop(L) != 2)
			throw std::runtime_error("setreadonly function requires 2 argument");
		luaL_checktype(L, 1, LUA_TTABLE);
		luaL_checktype(L, 2, LUA_TBOOLEAN);
		*(byte*)(index2adr(L, 1)->value.gc->h.isreadonly) = lua_toboolean(L, 2);
		return 0;
	}

	int isreadonly(lua_State* L)//Table
	{
		if (lua_gettop(L) != 1)
			throw std::runtime_error("isreadonly function requires 1 argument");
		luaL_checktype(L, 1, LUA_TTABLE);
		lua_pushboolean(L, *(byte*)(index2adr(L, 1)->value.gc->h.isreadonly));
		return 1;
	}

	int ReadFile(lua_State* L)
	{
		if (lua_gettop(L) != 1)
			throw std::runtime_error("ReadFile function requires 1 argument");//One sec I'm thinking
		if (lua_isstring(L, 1))
		{
			std::ifstream t(lua_tostring(L, 1));
			std::stringstream buffer;
			buffer << t.rdbuf();
			lua_pushstring(L, buffer.str().c_str());
			return 1;
		}
#if DebugAll
		else
		{
			printf("Attempt to call ReadFile failed: Passed value is not a Directory\n");
		}
#endif
		return 0;
	}

	int GetObjects(lua_State* L)
	{
		//This isn't important rn. I'll add it when I deem it
		return 0;
	}

	//Hacky stuff

	void FuncWrap(DWORD Thread, const char* LibName, const char* Name)
	{
		if (LibName == "")
		{
			lua_getglobal(GL, Name);
			r_lua_getglobal(Thread, Name);
			Wrap(Thread, GL, -1);
			r_lua_pop(Thread, 1);
			lua_pushstring(GL, Name);
			lua_pushcclosure(GL, Decider, 3);
			lua_setglobal(GL, Name);
		}
		else
		{
			lua_getglobal(GL, LibName);
			lua_getfield(GL, -1, Name);
			r_lua_getglobal(Thread, LibName);
			r_lua_getfield(Thread, -1, Name);
			Wrap(Thread, GL, -1);
			r_lua_pop(Thread, 2);
			std::string New = std::string(LibName) + "." + std::string(Name);
			lua_pushstring(GL, New.c_str());
			lua_pushcclosure(GL, Decider, 3);
			lua_setfield(GL, -2, Name);
			lua_pop(GL, 1);
		}
	}

	int Decider(lua_State* L)
	{
#if FunctionNames
		printf("%s Called for %s\n", __FUNCTION__, lua_tostring(L, lua_upvalueindex(3)));
#endif
		DWORD Args = lua_gettop(L);
		for (int c = 1; c <= Args; c++)
		{
			DWORD Type = lua_type(L, c);
			if (Type == LUA_TFUNCTION)
			{
				if ((DWORD)lua_tocfunction(L, c) == (DWORD)Calling)
				{
#if DebugAll || DebugHacks
					printf("Roblox\n");
#endif
					lua_pushvalue(L, lua_upvalueindex(2));
					lua_insert(L, -((int)Args) - 1);
					if (lua_pcall(L, Args, LUA_MULTRET, 0) == 0)
					{
						return lua_gettop(L);
					}
					else
					{
						if (lua_type(L, -1) == LUA_TSTRING)
							return luaL_error(L, lua_tostring(L, -1));
						else
							return luaL_error(L, "Unkown Error");
					}
				}
			}
			else if (Type == LUA_TTABLE)
			{
				if (luaL_getmetafield(L, c, "__key"))
				{
#if DebugAll || DebugHacks
					printf("Roblox\n");
#endif
					lua_pop(L, 1);
					lua_pushvalue(L, lua_upvalueindex(2));
					lua_insert(L, -((int)Args) - 1);
					if (lua_pcall(L, Args, LUA_MULTRET, 0) == 0)
					{
						return lua_gettop(L);
					}
					else
					{
						if (lua_type(L, -1) == LUA_TSTRING)
							return luaL_error(L, lua_tostring(L, -1));
						else
							return luaL_error(L, "Unkown Error");
					}
				}
			}
		}
#if DebugAll || DebugHacks
		printf("Vanilla\n");
#endif
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_insert(L, -((int)Args) - 1);
		if (lua_pcall(L, Args, LUA_MULTRET, 0) == 0)
		{
			return lua_gettop(L);
		}
		else
		{
			if (lua_type(L, -1) == LUA_TSTRING)
				return luaL_error(L, lua_tostring(L, -1));
			else
				return luaL_error(L, "Unkown Error");
		}
	}
}

static TValue* index2adr(lua_State* L, int idx) 
{
	if (idx > 0) {
		TValue* o = L->base + (idx - 1);
		api_check(L, idx <= L->ci->top - L->base);
		if (o >= L->top) return cast(TValue*, luaO_nilobject);
		else return o;
	}
	else if (idx > LUA_REGISTRYINDEX) {
		api_check(L, idx != 0 && -idx <= L->top - L->base);
		return L->top + idx;
	}
	else switch (idx) {  /* pseudo-indices */
	case LUA_REGISTRYINDEX: return registry(L);
	case LUA_ENVIRONINDEX: {
		Closure* func = curr_func(L);
		sethvalue(L, &L->env, func->c.env);
		return &L->env;
	}
	case LUA_GLOBALSINDEX: return gt(L);
	default: {
		Closure* func = curr_func(L);
		idx = LUA_GLOBALSINDEX - idx;
		return (idx <= func->c.nupvalues)
			? &func->c.upvalue[idx - 1]
			: cast(TValue*, luaO_nilobject);
	}
	}
}