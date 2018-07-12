#include "TrainerFunc.h"

DWORD dwGameInfoAddress;

BOOL PrepareProcess(HANDLE hProcess)
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD dwBuffer;

	WaitForInputIdle(hProcess, INFINITE);

	if(
		!MyReadProcessMemory(hProcess, (LPVOID)0x004CB908, &dwBuffer, 4) || 
		!MyReadProcessMemory(hProcess, (LPVOID)(0x004CB920+dwBuffer*4), &dwBuffer, 4)
	)
		return FALSE;

	while(!dwBuffer)
	{
		Sleep(10);
		if(
			!MyReadProcessMemory(hProcess, (LPVOID)0x004CB908, &dwBuffer, 4) || 
			!MyReadProcessMemory(hProcess, (LPVOID)(0x004CB920+dwBuffer*4), &dwBuffer, 4)
		)
			return FALSE;
	}

	dwGameInfoAddress = dwBuffer;

	dwBuffer = 0x0DFF90;
	if(
		!MyWriteProcessMemory(hProcess, (LPVOID)0x0040E90E, &dwBuffer, 3) || 
		!MyWriteProcessMemory(hProcess, (LPVOID)0x0040E915, &dwBuffer, 3)
	)
		return FALSE;

	VirtualQueryEx(hProcess, (LPVOID)0x004AD810, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
	if(mbi.Protect == PAGE_READONLY)
		VirtualProtectEx(hProcess, (LPVOID)0x004AD810, 8, PAGE_READWRITE, &dwBuffer);

	return TRUE;
}

int FreezeWorld(HANDLE hProcess)
{
	DWORD dwBuffer1, dwBuffer2;
	int retval;

	if(!MyReadProcessMemory(hProcess, (LPVOID)0x0040E829, &dwBuffer1, 2))
		return 0;

	switch(LOWORD(dwBuffer1))
	{
	case 0x8E0F:
		dwBuffer1 = 0xE990;
		dwBuffer2 = 0x75013C;
		retval = 1;
		break;

	case 0xE990:
		dwBuffer1 = 0x8E0F;
		dwBuffer2 = 0x74C085;
		retval = 2;
		break;

	default:
		return 0;
	}

	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x0040E829, &dwBuffer1, 2))
		return 0;

	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x0040E96F, &dwBuffer2, 3))
		return 0;

	return retval;
}

int ExFloor(HANDLE hProcess)
{
	DWORD dwBuffer[8];
	int retval;

	if(!MyReadProcessMemory(hProcess, (LPVOID)0x004119D8, dwBuffer, 3))
		return 0;

	switch(dwBuffer[0] & 0x00FFFFFF)
	{
	case 0x04518B:
		dwBuffer[0] = 0x90D233;
		dwBuffer[1] = 0x000291BA;
		dwBuffer[2] = 0x00;
		dwBuffer[3] = 0x02EBED33;
		dwBuffer[4] = 0x83909090;
		dwBuffer[5] = 0x7E28FD;
		dwBuffer[6] = 0x000014BE;
		dwBuffer[7] = 0x00;
		retval = 1;
		break;

	case 0x90D233:
		dwBuffer[0] = 0x04518B;
		dwBuffer[1] = 0x04E2C142;
		dwBuffer[2] = 0x42;
		dwBuffer[3] = 0xC8C4AA8B;
		dwBuffer[4] = 0xC8C8A839;
		dwBuffer[5] = 0x7F004C;
		dwBuffer[6] = 0x348DF8D1;
		dwBuffer[7] = 0x01;
		retval = 2;
		break;

	default:
		return 0;
	}

	// Check left
	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x004119D8, dwBuffer, 3))
		return 0;

	// Check right
	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x004119EE, dwBuffer+1, 5))
		return 0;

	// Draw left
	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x004089F3, dwBuffer+3, 4))
		return 0;

	// Draw right
	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x00408AB3, dwBuffer+4, 7))
		return 0;

	// Draw label
	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x00408C32, dwBuffer+6, 5))
		return 0;

	return retval;
}

int UnlockFloor(HANDLE hProcess, BYTE bFloor)
{
	if(bFloor < 0 || bFloor > 9)
		return 0;

	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x0049E238, &bFloor, 1))
		return 0;

	return 1;
}

long GetScore(HANDLE hProcess)
{
	long floor, score;

	if(!MyReadProcessMemory(hProcess, (LPVOID)(dwGameInfoAddress+0x28), &floor, 4))
		return 0;

	if(!MyReadProcessMemory(hProcess, (LPVOID)(dwGameInfoAddress+0x2C), &score, 4))
		return 0;

	return floor*10+score;
}

int SetScore(HANDLE hProcess, long score)
{
	long floor;

	if(!MyReadProcessMemory(hProcess, (LPVOID)(dwGameInfoAddress+0x28), &floor, 4))
		return 0;

	score -= floor*10;
	if(!MyWriteProcessMemory(hProcess, (LPVOID)(dwGameInfoAddress+0x2C), &score, 4))
		return 0;

	return 1;
}

int CreateCombo(HANDLE hProcess, long combo)
{
	DWORD dwBuffer;

	dwBuffer = 1;
	if(!MyWriteProcessMemory(hProcess, (LPVOID)(dwGameInfoAddress+0x40), &dwBuffer, 4))
		return 0;

	if(!MyWriteProcessMemory(hProcess, (LPVOID)(dwGameInfoAddress+0x44), &combo, 4))
		return 0;

	dwBuffer = 2;
	if(!MyWriteProcessMemory(hProcess, (LPVOID)(dwGameInfoAddress+0x48), &dwBuffer, 4))
		return 0;

	return 1;
}

int ChangeGravity(HANDLE hProcess, DWORD dwDoubleGravity[])
{
	DWORD dwOriginalDoubleGravity[2] = {0x9999999A, 0x3FE99999};
	DWORD dwCurrentDoubleGravity[2];

	if(!MyReadProcessMemory(hProcess, (LPVOID)0x004AD810, dwCurrentDoubleGravity, 8))
		return 0;

	if(dwDoubleGravity[0] == dwCurrentDoubleGravity[0] && dwDoubleGravity[1] == dwCurrentDoubleGravity[1])
	{
		if(!MyWriteProcessMemory(hProcess, (LPVOID)0x004AD810, dwOriginalDoubleGravity, 8))
			return 0;

		return 2;
	}

	if(!MyWriteProcessMemory(hProcess, (LPVOID)0x004AD810, dwDoubleGravity, 8))
		return 0;

	return 1;
}

BOOL MyReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize)
{
	DWORD dwNumberOfBytesRead;

	return ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &dwNumberOfBytesRead) && 
		dwNumberOfBytesRead == nSize;
}

BOOL MyWriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize)
{
	DWORD dwNumberOfBytesWritten;

	return WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &dwNumberOfBytesWritten) && 
		dwNumberOfBytesWritten == nSize;
}
