#include "bassmod.h"

BOOL (WINAPI* BASSMOD_Init)(int device, DWORD freq, DWORD flags);
BOOL (WINAPI* BASSMOD_MusicLoad)(BOOL mem, void *file, DWORD offset, DWORD length, DWORD flags);
BOOL (WINAPI* BASSMOD_MusicPlay)();
DWORD (WINAPI* BASSMOD_MusicIsActive)();
BOOL (WINAPI* BASSMOD_MusicPause)();
void (WINAPI* BASSMOD_MusicFree)();
void (WINAPI* BASSMOD_Free)();

char szTempFile[MAX_PATH];
HMODULE hBassMod;

BOOL LoadBassmod(LPCSTR lpResName, LPCSTR lpResType)
{
	char szTempDir[MAX_PATH];
	HANDLE hFile;
	HRSRC hRsrc;
	LPVOID pData;
	DWORD dwSizeofResource;
	DWORD dwNumberOfBytesWritten;

	GetTempPath(MAX_PATH, szTempDir);
	GetTempFileName(szTempDir, "bassmod", 0, szTempFile);

	hRsrc = FindResource(NULL, lpResName, lpResType);
	if(!hRsrc)
		return FALSE;

	pData = LoadResource(NULL, hRsrc);
	dwSizeofResource = SizeofResource(NULL, hRsrc);

	hFile = CreateFile(szTempFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	WriteFile(hFile, pData, dwSizeofResource, &dwNumberOfBytesWritten, NULL);
	CloseHandle(hFile);

	hBassMod = LoadLibrary(szTempFile);
	if(!hBassMod)
	{
		DeleteFile(szTempFile);
		return FALSE;
	}

	if(
		!(BASSMOD_Init = (BOOL (WINAPI*)(int, DWORD, DWORD))GetProcAddress(hBassMod, "BASSMOD_Init")) || 
		!(BASSMOD_MusicLoad = (BOOL (WINAPI*)(BOOL, void*, DWORD, DWORD, DWORD))GetProcAddress(hBassMod, "BASSMOD_MusicLoad")) || 
		!(BASSMOD_MusicPlay = (BOOL (WINAPI*)())GetProcAddress(hBassMod, "BASSMOD_MusicPlay")) || 
		!(BASSMOD_MusicIsActive = (DWORD (WINAPI*)())GetProcAddress(hBassMod, "BASSMOD_MusicIsActive")) || 
		!(BASSMOD_MusicPause = (BOOL (WINAPI*)())GetProcAddress(hBassMod, "BASSMOD_MusicPause")) || 
		!(BASSMOD_MusicFree = (void (WINAPI*)())GetProcAddress(hBassMod, "BASSMOD_MusicFree")) || 
		!(BASSMOD_Free = (void (WINAPI*)())GetProcAddress(hBassMod, "BASSMOD_Free"))
	)
	{
		FreeLibrary(hBassMod);
		DeleteFile(szTempFile);
		return FALSE;
	}

	if(!BASSMOD_Init(-1, 44100, 0))
	{
		FreeLibrary(hBassMod);
		DeleteFile(szTempFile);
		return FALSE;
	}

	return TRUE;
}

BOOL LoadBassFile(LPCSTR lpResName, LPCSTR lpResType)
{
	HRSRC hRsrc;
	LPVOID pData;
	DWORD dwSizeofResource;

	hRsrc = FindResource(NULL, lpResName, lpResType);
	if(!hRsrc)
		return FALSE;

	pData = LoadResource(NULL, hRsrc);
	dwSizeofResource = SizeofResource(NULL, hRsrc);

	return BASSMOD_MusicLoad(TRUE, pData, 0, dwSizeofResource, BASS_MUSIC_LOOP|BASS_MUSIC_RAMPS|BASS_MUSIC_SURROUND);
}

void FreeBassmod()
{
	BASSMOD_Free();
	FreeLibrary(hBassMod);
	DeleteFile(szTempFile);
}
