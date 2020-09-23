#include "Client.h"


HANDLE Client::hPipe1;
HANDLE Client::hPipe2;
BOOL Client::Finished;
BOOL Client::ConnectionMade;

int Client::MakeConnection(std::string PipeName1, std::string PipeName2)
{
	while (true) {
		hPipe1 = CreateFile((LPTSTR)PipeName1.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		hPipe2 = CreateFile((LPTSTR)PipeName2.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (hPipe1 != INVALID_HANDLE_VALUE && hPipe2 != INVALID_HANDLE_VALUE) {
			break;
		}
	}
	ConnectionMade = true;
	return 1;
}

int Client::SendInformation(std::string Information)
{
	char *buf = (char*)malloc(Information.length() - 1);
	DWORD cbWritten;
	//buf[Information.length() - 1];

	strcpy(buf, Information.c_str());
	WriteFile(hPipe2, buf, (DWORD)strlen(buf), &cbWritten, NULL);

	delete buf;
	return 1;
}

std::string Client::RecieveInformation()
{
	BOOL fSuccess;
	char chBuf[500000];
	DWORD cbRead;
	int i;
	while (hPipe1 != INVALID_HANDLE_VALUE)
	{
		fSuccess = ReadFile(hPipe1, chBuf, 500000, &cbRead, NULL);
		if (chBuf != NULL && fSuccess)
		{
			std::string s(chBuf);
			s.resize(cbRead);
#if Debug
			printf("Client Recieved: Bytes: %d\n", cbRead);
#endif
			return s;
		}
		Sleep(500);
	}
	return "";
}

void Client::WaitForReply()
{
	bool GotReply = false;
	BOOL fSuccess;
	char chBuf[500000];
	DWORD cbRead;
	while (!GotReply) {
		fSuccess = ReadFile(hPipe1, chBuf, 500000, &cbRead, NULL);
		if (chBuf != NULL && fSuccess) {
			GotReply = true;
		}
	}
}

void Client::MarkFinished()
{
	Finished = true;
	CloseHandle(hPipe1);
	CloseHandle(hPipe2);
}

//----------------------------------------\\