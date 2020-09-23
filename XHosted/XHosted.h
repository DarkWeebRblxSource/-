#if UpdateMethod < 3
#pragma once
/*
Name: XHosted.h
DateOfCreation: 13/03/2020
Info: This file will manage a few tasks:
-Hashing of security measures(This will be written in pseudo-code to prevent anyone understanding)
-Sending data and recieving the JSoN file from the website.
-Loading the data?
*/

//Includes
#include <Windows.h>
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <functional>
#include <curl.h>
#include <json.h>
#include <ctime>
#include "RetCheck.hpp"
#include "Tools.h"
#define aslr(x) (x - 0x400000 + (DWORD)GetModuleHandle(0))
#define noaslr(x) (x + 0x400000 - (DWORD)GetModuleHandle(0))
#define r_lua_tostring(rL,i)	r_lua_tolstring(rL, (i), NULL)
#define r_lua_pop(rL,n)		r_lua_settop(rL, -(n)-1)
#define r_lua_getglobal(rL,s)	r_lua_getfield(rL, LUA_GLOBALSINDEX, (s))
#define r_lua_setglobal(rL,s)	r_lua_setfield(rL, LUA_GLOBALSINDEX, (s))
#define r_lua_newtable(rL) r_lua_createtable(rL, 0, 0)
DWORD stack_last = 12;
DWORD stacksize = 20;

DWORD R_LUA_TNIL,R_LUA_TLIGHTUSERDATA,R_LUA_TNUMBER,R_LUA_TBOOLEAN,R_LUA_TSTRING,R_LUA_TTHREAD,R_LUA_TFUNCTION,R_LUA_TTABLE,R_LUA_TUSERDATA,R_LUA_TPROTO,R_LUA_TUPVALUE;
/*
Comment out ones you don't use or add ones you want to use
*/
std::function<void(DWORD, int, const char*)> r_lua_getfield;
std::function<char* (DWORD, int, size_t*)> r_lua_tolstring;
std::function<void(DWORD, int)> r_lua_settop;
std::function<bool(DWORD, int)> r_lua_toboolean;
std::function<void(DWORD, int)> r_lua_pushvalue;
std::function<int(int, double)> r_lua_pushnumber;
std::function<void(DWORD, const char*)> r_lua_pushstring;
std::function<int(DWORD, int, int, int)> r_lua_pcall;
std::function<void(DWORD)> r_lua_pushnil;
std::function<DWORD(DWORD, int)> r_lua_next;
std::function<void(DWORD, bool)> r_lua_pushboolean;
std::function<double(DWORD, int, int)> r_lua_tonumber;
std::function<void(DWORD, int, int, int, int)> r_lua_pushcclosure;
std::function<void(DWORD, int, int)> r_lua_createtable;
std::function<DWORD(DWORD)> r_lua_newthread;
std::function<void* (int, unsigned int, char)> r_lua_newuserdata;
std::function<void(DWORD, DWORD, DWORD)> r_lua_rawgeti;
std::function<void(DWORD, DWORD, DWORD)> r_lua_rawseti;
std::function<int (DWORD, int)> r_lua_getmetatable;
std::function<void (DWORD, int)> r_lua_setmetatable;
std::function<int(DWORD, int)> r_lua_touserdata;
std::function<int(DWORD, int)> r_lua_type;
std::function<const char*(DWORD, int)> r_lua_typename;
std::function<DWORD(DWORD, DWORD)> r_luaL_ref;
std::function<void(DWORD, DWORD, int)> r_luaL_unref;
std::function<void (DWORD, int)> r_lua_settable;
std::function<void(DWORD, void*)> r_lua_pushlightuserdata;
std::function<int(DWORD)> r_lua_gettop;
std::function<int(int, const char*)> print;
std::function<TValue* (DWORD, DWORD)> r_index2adr;
std::function<DWORD(DWORD, DWORD)> r_lua_tothread;
std::function<void(DWORD, DWORD, DWORD)> r_lua_rawset;
std::function<void(DWORD, DWORD)> r_lua_rawget;
std::function<void(DWORD, DWORD, DWORD)> r_lua_call;
std::function<void(DWORD, DWORD)> r_lua_remove;
std::function<const void* (DWORD, DWORD)> r_lua_topointer;
std::function<bool(DWORD, DWORD)> r_lua_iscfunction;
std::function<void(DWORD, DWORD)> r_luaD_growstack;
std::function<void(DWORD, DWORD)> r_luaD_reallocstack;
std::function<void(DWORD, DWORD)> r_lua_gettable;
std::function<size_t(DWORD, DWORD)> r_lua_objlen;
std::function<void(DWORD)> r_lua_pushthread;
std::function<void (DWORD, DWORD, DWORD)> r_lua_xmove;

void r_luaD_checkstack(DWORD RL, int Args);

int r_luaL_getmetafield(DWORD RL, int obj, const char* event)
{
    if (!r_lua_getmetatable(RL, obj))  /* no metatable? */
        return 0;
    r_lua_pushstring(RL, event);
    r_lua_rawget(RL, -2);
    if (r_lua_type(RL, -1) == R_LUA_TNIL) {
        r_lua_pop(RL, 2);  /* remove metatable and metafield */
        return 0;
    }
    else {
        r_lua_remove(RL, -2);  /* remove only metatable */
        return 1;
    }
}
int r_luaL_callmeta(DWORD RL, int obj, const char* event)
{
    if (!r_luaL_getmetafield(RL, obj, event))  /* no metafield? */
        return 0;
    r_lua_pushvalue(RL, obj);
    r_lua_pcall(RL, 1, 1, 0);
    return 1;
}
//Namespace

namespace
{
    std::size_t callback(
        const char* in,
        std::size_t size,
        std::size_t num,
        std::string* out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}
namespace XHosted
{
    //Values
    std::string URLBase = "www.xhosted.xyz/Test.json";
    std::string Key = "";
    Json::Value JSON;
    struct AddressS
    {
        DWORD Address;
        std::string CC;
        bool RetCheck;
    };
    //Functions
    void SetUp();
    std::vector<std::string> Split(const std::string s, const char delim)
    {
        std::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
        {
            elems.push_back(item);
        }
        return elems;
    }
    std::string GetVersion()
    {
        //Load up file path into a char
        char c_fpath[MAX_PATH];
        GetModuleFileNameA(GetModuleHandleA(0), c_fpath, sizeof(c_fpath));

        //Convert to string
        std::string Dir = std::string(c_fpath);

        //Split into a vector by char '/'
        std::vector<std::string> Parts = Split(Dir, '\\');

        //Check Split Worked
        if (Parts.size() > 1)
        {
            //Find Correct One For Version Name
            for (int i = 0; i < Parts.size(); i++)
            {
                std::vector<std::string> Breakdown = Split(Parts[i], '-');
                //Got it
                if (Breakdown[0] == "version")
                {
                    return Parts[i];
                }
            }
            return "";
        }
        else
        {
            return "";
        }
    }
    std::string encrypt(std::string text, int s)
    {
        std::string result = "";

        // traverse text 
        for (int i = 0; i < text.length(); i++)
        {
            // apply transformation to each character 
            // Encrypt Uppercase letters 
            if (isupper(text[i]))
                result += char(int(text[i] + s - 65) % 26 + 65);

            // Encrypt Lowercase letters 
            else
                result += char(int(text[i] + s - 97) % 26 + 97);
        }

        // Return the resulting string 
        return result;
    }

    Json::Value GrabJSON()
    {
        std::string Reply = Tools::ReadWebsite(URLBase.c_str());
        if (Reply != "")
        {
            const auto rawJsonLength = static_cast<int>((Reply).length());
            JSONCPP_STRING err;
            Json::Value root;

            Json::CharReaderBuilder builder;
            const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            if (!reader->parse((Reply).c_str(), Reply.c_str() + rawJsonLength, &root,
                &err)) {
                std::cout << "Invalid reply-2452" << std::endl;
                return EXIT_FAILURE;
            }
            return root;
        }
        else
        {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE;
    }

    template<class Ret, class... Args>
    std::function<Ret(Args...)> Call(DWORD Address, std::string Convention)
    {
        if (Convention == "Stdcall")
        {
            typedef Ret(__stdcall* Fn_t)(Args...);
            return (Fn_t)Address;
        }
        else if (Convention == "Cdecl")
        {
            typedef Ret(__cdecl* Fn_t)(Args...);
            return (Fn_t)Address;
        }
        else if (Convention == "Fastcall")
        {
            typedef Ret(__fastcall* Fn_t)(Args...);
            return (Fn_t)Address;
        }
        else if (Convention == "Thiscall")
        {
            typedef Ret(__thiscall* Fn_t)(Args...);
            return (Fn_t)Address;
        }
    }

    bool Run(std::string Version)
    {
        std::time_t result = std::time(nullptr);
        DWORD Hours = 0;
        URLBase = "";
        Json::Value JSONConnected = GrabJSON();
        if (JSONConnected != EXIT_FAILURE)
        {
            XHosted::JSON = JSONConnected;
            SetUp();
            return true;
        }
        else
        {
            return false;
        }
    }
    bool CheckLiveJSON()
    {
        if (JSON <= EXIT_FAILURE)
            return Run(GetVersion());
        return true;
    }

    template<class Ret, class... Args>
    auto CreateTypeDef(std::string FuncName)//You will need to add your way of retcheck bypass here if you chose to use this function
    {
        if (CheckLiveJSON()) {
            return Call<Ret, Args...>(unprotect(aslr(JSON["addresses"][FuncName.c_str()]["addr"].asInt())), JSON["addresses"][FuncName.c_str()]["cc"].asString().c_str());
        }
    }

    void XDumper()
    {
        if (CheckLiveJSON())
        {
            Json::Value Addresses = JSON["addresses"];
            for (auto const& id : Addresses.getMemberNames()) {
                std::cout << id << ": 0x" << std::hex << std::uppercase << Addresses[id.c_str()]["addr"].asInt() << " Return Check: " << std::boolalpha << Addresses[id.c_str()]["retcheck"].asBool() << " Calling Convention: " << Addresses[id.c_str()]["cc"].asString() << std::endl;
            }
        }
    }

    DWORD GetRState()
    {
        if (CheckLiveJSON())
        {
            volatile DWORD StackPad[16]{}; // 200 iq
            static DWORD DMPad[16]{}; // MUST be static thanks to MSVCtism
            auto GetDataModel = XHosted::Call<DWORD, DWORD, DWORD>(aslr(JSON["addresses"]["getdatamodel"]["addr"].asInt()), "Thiscall");
            auto GetAddress = XHosted::Call<DWORD>(aslr(JSON["addresses"]["getdatamodel2"]["addr"].asInt()), "Cdecl");//0xE18010
            GetDataModel(GetAddress(), (DWORD)DMPad);
            DWORD DM = DMPad[0];
            DM = DM + 0x44;
            auto FindFirstChild = XHosted::Call<DWORD, DWORD, const std::string&>(aslr(JSON["addresses"]["findfirstchild"]["addr"].asInt()), "Thiscall");
            DWORD SC = FindFirstChild(DM, "Script Context");
            std::string Symbol = JSON["luastate"]["extra"]["symbol"].asString();
            DWORD StateOffset = JSON["luastate"]["extra"]["scoffset"].asInt();
            DWORD StateOrder = JSON["luastate"]["extra"]["scorder"].asInt();
            if (StateOrder == 1) {
                if (Symbol == "^")
                {
                    return *(DWORD*)(SC + StateOffset) ^ (SC + StateOffset);
                }
                else if (Symbol == "*")
                {
                    return *(DWORD*)(SC + StateOffset) * (SC + StateOffset);
                }
                else if (Symbol == "-")
                {
                    return *(DWORD*)(SC + StateOffset) - (SC + StateOffset);
                }
                else if (Symbol == "+")
                {
                    return *(DWORD*)(SC + StateOffset) + (SC + StateOffset);
                }
                else if (Symbol == "/")
                {
                    return *(DWORD*)(SC + StateOffset) / (SC + StateOffset);
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                if (Symbol == "^")
                {
                    return (SC + StateOffset) ^ *(DWORD*)(SC + StateOffset);
                }
                else if (Symbol == "*")
                {
                    return (SC + StateOffset) * *(DWORD*)(SC + StateOffset);
                }
                else if (Symbol == "-")
                {
                    return (SC + StateOffset) - *(DWORD*)(SC + StateOffset);
                }
                else if (Symbol == "+")
                {
                    return (SC + StateOffset) + *(DWORD*)(SC + StateOffset);
                }
                else if (Symbol == "/")
                {
                    return (SC + StateOffset) / *(DWORD*)(SC + StateOffset);
                }
                else
                {
                    return 0;
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }

    int GetType(const char* Name)
    {
        if (CheckLiveJSON())
        {
            return XHosted::JSON["types"][Name].asInt();
        }
        else
        {
            return 0;
        }
    }

    void SetLevel(DWORD RobloxState, int Level)
    {
        int v3 = *(DWORD*)(RobloxState + XHosted::JSON["luastate"]["extra"]["identity2"].asInt());
        *(DWORD*)(v3 + XHosted::JSON["luastate"]["extra"]["identity1"].asInt()) = Level;
    }

    void SetUp()
    {
        R_LUA_TNIL = XHosted::GetType("nil");
        R_LUA_TLIGHTUSERDATA = XHosted::GetType("lightuserdata");
        R_LUA_TNUMBER = XHosted::GetType("number");
        R_LUA_TBOOLEAN = XHosted::GetType("boolean");
        R_LUA_TSTRING = XHosted::GetType("string");
        R_LUA_TTHREAD = XHosted::GetType("thread");
        R_LUA_TFUNCTION = XHosted::GetType("function");
        R_LUA_TTABLE = XHosted::GetType("table");
        R_LUA_TUSERDATA = XHosted::GetType("userdata");
        R_LUA_TPROTO = XHosted::GetType("proto");
        R_LUA_TUPVALUE = XHosted::GetType("upval");
        /*
        Comment out ones you don't use or add ones you want to use
        */
        r_lua_getfield = XHosted::CreateTypeDef<void, DWORD, int, const char*>("lua_getfield");
        r_lua_tolstring = XHosted::CreateTypeDef<char*, DWORD, int, size_t*>("lua_tolstring");
        r_lua_settop = XHosted::CreateTypeDef<void, DWORD, int>("lua_settop");
        r_lua_toboolean = XHosted::CreateTypeDef<bool, DWORD, int>("lua_toboolean");
        r_lua_pushvalue = XHosted::CreateTypeDef<void, DWORD, int>("lua_pushvalue");
        r_lua_pushnumber = XHosted::CreateTypeDef<int, int, double>("lua_pushnumber");
        r_lua_pushstring = XHosted::CreateTypeDef<void, DWORD, const char*>("lua_pushstring");
        r_lua_pcall = XHosted::CreateTypeDef<int, DWORD, int, int, int>("lua_pcall");
        r_lua_pushnil = XHosted::CreateTypeDef<void, DWORD>("lua_pushnil");
        r_lua_next = XHosted::CreateTypeDef<DWORD, DWORD, int>("lua_next");
        r_lua_pushboolean = XHosted::CreateTypeDef<void, DWORD, bool>("lua_pushboolean");
        r_lua_tonumber = XHosted::CreateTypeDef<double, DWORD, int, int>("lua_tonumber");
        r_lua_pushcclosure = XHosted::CreateTypeDef<void, DWORD, int, int, int, int>("lua_pushcclosure");
        r_lua_createtable = XHosted::CreateTypeDef<void, DWORD, int, int>("lua_createtable");
        r_lua_newthread = XHosted::CreateTypeDef<DWORD, DWORD>("lua_newthread");
        r_lua_newuserdata = XHosted::CreateTypeDef<void*, DWORD, size_t, int>("lua_newuserdata");
        r_lua_rawgeti = XHosted::CreateTypeDef<void, DWORD, DWORD, DWORD>("lua_rawgeti");
        r_lua_rawseti = XHosted::CreateTypeDef<void, DWORD, DWORD, DWORD>("lua_rawseti");
        r_lua_getmetatable = XHosted::CreateTypeDef<int, DWORD, int>("lua_getmetatable");
        r_lua_setmetatable = XHosted::CreateTypeDef<void, DWORD, int>("lua_setmetatable");
        r_lua_touserdata = XHosted::CreateTypeDef<int, DWORD, int>("lua_touserdata");
        r_lua_type = XHosted::CreateTypeDef<int, DWORD, int>("lua_type");
        r_lua_typename = XHosted::CreateTypeDef<const char*, DWORD, int>("lua_typename");
        r_luaL_ref = XHosted::CreateTypeDef<DWORD, DWORD, int>("luaL_ref");
        r_luaL_unref = XHosted::CreateTypeDef<void, DWORD, DWORD, int>("luaL_unref");
        r_lua_settable = XHosted::CreateTypeDef<void, DWORD, int>("lua_settable");
        r_lua_pushlightuserdata = XHosted::CreateTypeDef<void, DWORD, void*>("lua_pushlightuserdata");
        r_lua_gettop = XHosted::CreateTypeDef<int, DWORD>("lua_gettop");
        print = XHosted::CreateTypeDef<int, int, const char*>("print");
        r_index2adr = XHosted::CreateTypeDef<TValue*, DWORD, DWORD>("index2adr");
        r_lua_tothread = XHosted::CreateTypeDef<DWORD, DWORD, DWORD>("lua_tothread");
        r_lua_rawset = XHosted::CreateTypeDef<void, DWORD, DWORD, DWORD>("lua_rawset");
        r_lua_rawget = XHosted::CreateTypeDef<void, DWORD, DWORD>("lua_rawget");
        r_lua_call = XHosted::CreateTypeDef<void, DWORD, DWORD, DWORD>("lua_call");
        r_lua_remove = XHosted::CreateTypeDef<void, DWORD, DWORD>("lua_remove");
        r_lua_topointer = XHosted::CreateTypeDef<const void*, DWORD, DWORD>("lua_topointer");
        r_lua_iscfunction = XHosted::CreateTypeDef<bool, DWORD, DWORD>("lua_iscfunction");
        r_luaD_growstack = XHosted::CreateTypeDef<void, DWORD, DWORD>("luaD_growstack");
        r_luaD_reallocstack = XHosted::CreateTypeDef<void, DWORD, DWORD>("luaD_reallocstack");
        r_lua_gettable = XHosted::CreateTypeDef<void, DWORD, DWORD>("lua_gettable");
        r_lua_objlen = XHosted::CreateTypeDef<size_t, DWORD, DWORD>("lua_objlen");
        r_lua_xmove = XHosted::CreateTypeDef<void, DWORD, DWORD, DWORD>("lua_xmove");
        r_lua_pushthread = XHosted::CreateTypeDef<void, DWORD>("lua_pushthread");

    }
}
void r_luaD_checkstack(DWORD RL, int Args)
{
    if ((char*)*(DWORD*)(RL + stack_last) - (char*)*(DWORD*)(RL + XHosted::JSON["luastate"]["Top"]["offset"].asInt()) <= (Args) * (int)sizeof(TValue)) \
        r_luaD_growstack(RL, Args); \
    else condhardstacktests(r_luaD_reallocstack(RL, *(DWORD*)(RL + stacksize) - EXTRA_STACK - 1));//Stacksize
}
#endif