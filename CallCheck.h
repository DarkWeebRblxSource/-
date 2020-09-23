#pragma once
#include "Global.h"
namespace CCheck
{
	extern std::vector<int> int3breakpoints;
	LONG WINAPI vehHandler(PEXCEPTION_POINTERS ex);
	DWORD locateINT3(DWORD Start);
	void VehHandlerpush();
}