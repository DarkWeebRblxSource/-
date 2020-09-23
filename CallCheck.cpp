#include "CallCheck.h"
#include "Environment.h"
namespace CCheck
{
	std::vector<int> int3breakpoints;
	LONG WINAPI vehHandler(PEXCEPTION_POINTERS ex)
	{
		switch (ex->ExceptionRecord->ExceptionCode)
		{
		case (DWORD)0x80000003L:
		{
			
			if (ex->ContextRecord->Eip == int3breakpoints[0])
			{
				ex->ContextRecord->Eip = (DWORD)(Env::RCalling);
				return EXCEPTION_CONTINUE_EXECUTION;
			}
			
			if (ex->ContextRecord->Eip == int3breakpoints[1])
			{
				ex->ContextRecord->Eip = (DWORD)(Env::ResumeR);
				return EXCEPTION_CONTINUE_EXECUTION;
			}
			
			return -1;
		}
		default: return 0;
		}
		return 0;
	}

	DWORD locateINT3(DWORD Start)
	{
		DWORD _s = aslr(Start);
		const char i3_8opcode[8] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
		for (int i = 0; i < MAX_INT; i++) {
			if (memcmp((void*)(_s + i), i3_8opcode, sizeof(i3_8opcode)) == 0) {
				return (_s + i);
			}
		}
		return NULL;
	}

	void VehHandlerpush()
	{
		int3breakpoints.push_back(locateINT3(0x400000));
		int3breakpoints.push_back(locateINT3(int3breakpoints[0] + 0x100));
		AddVectoredExceptionHandler(1, vehHandler);
	}
}