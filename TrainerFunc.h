#ifndef _TRAINERFUNC_H_
#define _TRAINERFUNC_H_

#include <windows.h>

BOOL PrepareProcess(HANDLE hProcess);
int FreezeWorld(HANDLE hProcess);
int ExFloor(HANDLE hProcess);
int UnlockFloor(HANDLE hProcess, BYTE bFloor);
long GetScore(HANDLE hProcess);
int SetScore(HANDLE hProcess, long score);
int CreateCombo(HANDLE hProcess, long combo);
int ChangeGravity(HANDLE hProcess, DWORD dwDoubleGravity[]);
BOOL MyReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize);
BOOL MyWriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize);

#endif // _TRAINERFUNC_H_
