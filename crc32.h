#ifndef _CRC32_H_
#define _CRC32_H_

#include <windows.h>

DWORD CalcFileCrc32(LPSTR lpFileName, LPBOOL pbSuccess);
DWORD CalcMemoryCrc32(LPBYTE pByte, DWORD dwByteCount);
void MakeCrc32Table(DWORD m_pdwCrc32Table[]);

#endif  // _CRC32_H_
