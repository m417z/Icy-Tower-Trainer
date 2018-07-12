#ifndef _FADEEFFECT_H_
#define _FADEEFFECT_H_

#include <windows.h>
#include "buffer.h"

#define WM_ONFADEOUT WM_USER+1

void FadeIn(HWND hWnd);
DWORD WINAPI FadeThread(LPVOID pNotPointerJustWindowHandle);
void AddEffect(UINT *ptPixels, SIZE *sSize, int effect, int effect_stage);
void SetPixelTransparency(UINT *ptPixels, SIZE *sSize, int x, int y, BYTE bTransparency);
void PreMultiplyRGBChannels(UINT *ptPixels, SIZE *sSize);
void FadeOut(HWND hWnd);

#endif // _FADEEFFECT_H_
