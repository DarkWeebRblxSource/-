#pragma once

#include "Global.h"


namespace Client
{
	extern HANDLE hPipe1, hPipe2;
	extern BOOL Finished;
	extern BOOL ConnectionMade;
	void MarkFinished();
	int MakeConnection(std::string PipeName1, std::string PipeName2);
	std::string RecieveInformation();
	int SendInformation(std::string Information);
	void WaitForReply();
}