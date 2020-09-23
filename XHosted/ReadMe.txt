1. Add XHosted to the directory of your DLL project
2. Go to VC++ Directories and add
Include Directories
$(MSBuildProjectDirectory)\XHosted\curl
$(MSBuildProjectDirectory)\XHosted\json

Library Directories
$(MSBuildProjectDirectory)\XHosted\json
$(MSBuildProjectDirectory)\XHosted\curl

3. Go to C/C++ then Preprocessor
Preprocessor Definitions add
CURL_STATICLIB

4.Go to Linker then Input
Additional Dependencies Should have all of these
libcurl_a.lib
Ws2_32.lib
Crypt32.lib
Wldap32.lib
Normaliz.lib
jsoncpp.lib

5. Add #include "XHosted/XHosted.h" to a file
Call XHosted::Run(KEY HERE, VERSION HERE)//If you want always current version then do this

Call XHosted::Run(KEY HERE, XHosted::GetVersion())

How to use
Types
XHosted::JSON["types"]["lightuserdata"].asInt();
All Roblox Types

Addresses
XHosted::JSON["addresses"]["lua_getfield"]["addr"].asInt();
XHosted::JSON["addresses"]["lua_getfield"]["cc"].asString();
XHosted::JSON["addresses"]["lua_getfield"]["retcheck"].asBoolean();
auto rlua_getfield = XHosted::CreateTypeDef<void, DWORD, int, const char*>("lua_getfield");
Current has all addresses XDumper has

Offsets
XHosted::JSON["luastate"]["Top"]["offset"].asInt();
luastate->offsets->Top(int)
luastate->offsets->Base(int)
luastate->extra->symbol(string)
luastate->extra->scoffset(int)
luastate->extra->identity1(int)(1 is small one like 24)
luastate->extra->identity2(int)(2 is big one like 132)

Flags
XHosted::JSON["flags"]["getdatamodel"].asInt();
getdatamodel(int)

VFTables
XHosted::JSON["vftables"]["ScriptContext"].asInt();
ScriptContext(int)
Players(int)
Lighting(int)