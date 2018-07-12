#include "crc32.h"

DWORD CalcFileCrc32(LPSTR lpFileName, LPBOOL pbSuccess)
{
	HANDLE hFile;
	DWORD m_pdwCrc32Table[256];
	DWORD dwCrc32 = 0xFFFFFFFF;
	DWORD dwByteCount;
	BYTE bBuffer[1024];
	LPBYTE pByte;

	// Prepare for the worst
	if(pbSuccess)
		*pbSuccess = FALSE;

	hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return NULL;

	MakeCrc32Table(m_pdwCrc32Table);

	if(!ReadFile(hFile, bBuffer, 1024, &dwByteCount, NULL))
	{
		CloseHandle(hFile);
		return NULL;
	}

	while(dwByteCount)
	{
		pByte = bBuffer;

		while(dwByteCount-- > 0)
		{
			dwCrc32 = ((dwCrc32) >> 8) ^ m_pdwCrc32Table[(*pByte) ^ ((dwCrc32) & 0x000000FF)];
			pByte++;
		}

		if(!ReadFile(hFile, bBuffer, 1024, &dwByteCount, NULL))
		{
			CloseHandle(hFile);
			return NULL;
		}
	}

	CloseHandle(hFile);

	dwCrc32 = ~dwCrc32;

	if(pbSuccess)
		*pbSuccess = TRUE;

	return dwCrc32;
}

DWORD CalcMemoryCrc32(LPBYTE pByte, DWORD dwByteCount)
{
	DWORD m_pdwCrc32Table[256];
	DWORD dwCrc32 = 0xFFFFFFFF;

	MakeCrc32Table(m_pdwCrc32Table);

	while(dwByteCount-- > 0)
	{
		dwCrc32 = ((dwCrc32) >> 8) ^ m_pdwCrc32Table[(*pByte) ^ ((dwCrc32) & 0x000000FF)];
		pByte++;
	}

	dwCrc32 = ~dwCrc32;

	return dwCrc32;
}

void MakeCrc32Table(DWORD m_pdwCrc32Table[])
{
	DWORD dwPolynomial = 0xEDB88320;
	DWORD dwCrc;
	int i, j;

	for(i=0; i<256; i++)
	{
		dwCrc = i;
		for(j=8; j>0; j--)
		{
			if(dwCrc & 1)
				dwCrc = (dwCrc >> 1) ^ dwPolynomial;
			else
				dwCrc >>= 1;
		}
		m_pdwCrc32Table[i] = dwCrc;
	}
}
