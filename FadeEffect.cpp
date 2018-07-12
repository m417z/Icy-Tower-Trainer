#include "FadeEffect.h"

int fade_out; // 0: Fade in, -1: Idle, 1: Fade out

void FadeIn(HWND hWnd)
{
	fade_out = 0;
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	EnableWindow(hWnd, FALSE);
	CloseHandle(CreateThread(NULL, NULL, FadeThread, hWnd, NULL, NULL));
}

DWORD WINAPI FadeThread(LPVOID pNotPointerJustWindowHandle)
{
	HWND hWnd;
	HDC hDC;
	RECT rcRect;
	SIZE sSize;
	HDC hDesktopDC;
	HDC hMemDC;
	HBITMAP hBitmap;
	HGDIOBJ hOldBmp;
	POINT ptZero;
	BLENDFUNCTION bfBlend;
	int effect;
	int effect_stage;
	BITMAPINFO bmiBitmapInfo;
	HDC hEffectDC;
	HBITMAP hBitmapWithEffect;
	HGDIOBJ hOldBmpWithEffect;
	UINT *ptPixels;
	DWORD dwTime1, dwTime2;

	// Recieve window handle
	hWnd = (HWND)pNotPointerJustWindowHandle;

	// Get window DC and size
	hDC = GetDC(hWnd);
	GetWindowRect(hWnd, &rcRect);
	sSize.cx = rcRect.right - rcRect.left;
	sSize.cy = rcRect.bottom - rcRect.top;

	// Get desktop DC
	hDesktopDC = GetDC(HWND_DESKTOP);

	// Print our window to hBitmap
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hDC, sSize.cx, sSize.cy);
	hOldBmp = SelectObject(hMemDC, hBitmap);
	SendMessage(hWnd, WM_PRINT, (WPARAM)hMemDC, PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND|PRF_CHILDREN);

	// Create a BITMAPINFO with minimal initialization for the CreateDIBSection
	ZeroMemory(&bmiBitmapInfo, sizeof(BITMAPINFO));
	bmiBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiBitmapInfo.bmiHeader.biWidth = sSize.cx;
	bmiBitmapInfo.bmiHeader.biHeight = sSize.cy;
	bmiBitmapInfo.bmiHeader.biPlanes = 1;
	bmiBitmapInfo.bmiHeader.biBitCount = 32;

	// Create a bitmap for fade effects, with direct pixel access
	hEffectDC = CreateCompatibleDC(hDC);
	hBitmapWithEffect = CreateDIBSection(hEffectDC, &bmiBitmapInfo, DIB_RGB_COLORS, (void **)&ptPixels, NULL, NULL);
	hOldBmpWithEffect = SelectObject(hEffectDC, hBitmapWithEffect);

	// Initialize stuff for UpdateLayeredWindow
	ZeroMemory(&bfBlend, sizeof(BLENDFUNCTION));
	bfBlend.BlendOp = AC_SRC_OVER;
	bfBlend.SourceConstantAlpha = 255;
	bfBlend.AlphaFormat = AC_SRC_ALPHA;

	ptZero.x = 0;
	ptZero.y = 0;

	// Initialize loop
	dwTime1 = timeGetTime();

	effect = dwTime1%5;
	if(fade_out == 0)
		effect_stage = 0x00;
	else
		effect_stage = 0x30-1;

	// Main loop
	while(effect_stage>=0x00 && effect_stage<0x30)
	{
		// Copy our bitmap, and add an effect to it
		BitBlt(hEffectDC, 0, 0, sSize.cx, sSize.cy, hMemDC, 0, 0, SRCCOPY);
		AddEffect(ptPixels, &sSize, effect, effect_stage);
		PreMultiplyRGBChannels(ptPixels, &sSize);

		// UpdateLayeredWindow
		UpdateLayeredWindow(hWnd, hDesktopDC, NULL, &sSize, hEffectDC, &ptZero, NULL, &bfBlend, ULW_ALPHA);

		// Delay
		dwTime2 = timeGetTime();
		if(dwTime2-dwTime1 < 10)
			Sleep(10-(dwTime2-dwTime1));
		dwTime1 = dwTime2;

		// Next!
		if(fade_out == 0)
			effect_stage++;
		else
			effect_stage--;
	}

	// Clean up
	DeleteObject(hBitmapWithEffect);
	SelectObject(hEffectDC, hOldBmpWithEffect);
	DeleteDC(hEffectDC);
	DeleteObject(hBitmap);
	SelectObject(hMemDC, hOldBmp);
	DeleteDC(hMemDC);
	ReleaseDC(HWND_DESKTOP, hDesktopDC);
	ReleaseDC(hWnd, hDC);

	// Show window normally?
	if(fade_out == 0)
	{
		// Make window non-layered, and enable it
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		EnableWindow(hWnd, TRUE);

		// Ask the window and its children to repaint
		RedrawWindow(hWnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN);
	}
	else
		PostMessage(hWnd, WM_ONFADEOUT, NULL, NULL);

	fade_out = -1;

	return 0;
}

void AddEffect(UINT *ptPixels, SIZE *sSize, int effect, int effect_stage)
{
	int a, b;
	int x, y;

	switch(effect+1)
	{
	case 1:
		// Fade left to right
		a = sSize->cx*(effect_stage%0x10)/0x20;
		b = sSize->cx/0x02;

		if(effect_stage < 0x10)
		{
			for(y=0; y<sSize->cy; y++)
				for(x=0; x<a; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF*(a-x)/b);
		}
		else if(effect_stage < 0x20)
		{
			for(y=0; y<sSize->cy; y++)
				for(x=0; x<a; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);

			for(y=0; y<sSize->cy; y++)
				for(x=a; x<a+b; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF-0xFF*(x-a)/b);
		}
		else
		{
			for(y=0; y<sSize->cy; y++)
				for(x=0; x<a+b; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);

			for(y=0; y<sSize->cy; y++)
				for(x=a+b; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF-0xFF*(x-a-b)/b);
		}
		break;

	case 2:
		// Fade from above
		a = sSize->cy*(effect_stage%0x10)/0x20;
		b = sSize->cy/0x02;

		if(effect_stage < 0x10)
		{
			for(y=0; y<a; y++)
				for(x=0; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF*(a-y)/b);
		}
		else if(effect_stage < 0x20)
		{
			for(y=0; y<a; y++)
				for(x=0; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);

			for(y=a; y<a+b; y++)
				for(x=0; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF-0xFF*(y-a)/b);
		}
		else
		{
			for(y=0; y<a+b; y++)
				for(x=0; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);

			for(y=a+b; y<sSize->cy; y++)
				for(x=0; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF-0xFF*(y-a-b)/b);
		}
		break;

	case 3:
		// Fade diagonally
		a = sSize->cy*(effect_stage%0x10)/0x10;
		b = sSize->cx*(effect_stage%0x10)/0x10;

		if(effect_stage < 0x10)
		{
			for(y=0; y<a; y++)
				for(x=0; x<b-y*sSize->cx/sSize->cy; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, (BYTE)(0xFF*(b-y*sSize->cx/sSize->cy-x)/sSize->cx));
		}
		else if(effect_stage < 0x20)
		{
			for(y=0; y<a; y++)
				for(x=0; x<b-y*sSize->cx/sSize->cy; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);

			for(y=0; y<a; y++)
				for(x=b-y*sSize->cx/sSize->cy; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, (BYTE)(0xFF-0xFF*(x-b+y*sSize->cx/sSize->cy)/sSize->cx));

			for(y=a; y<sSize->cy; y++)
				for(x=0; x<sSize->cx-(y-a)*sSize->cx/sSize->cy; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, (BYTE)(0xFF-0xFF*(x-b+y*sSize->cx/sSize->cy)/sSize->cx));
		}
		else
		{
			for(y=0; y<a; y++)
				for(x=0; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);

			for(y=a; y<sSize->cy; y++)
				for(x=0; x<sSize->cx-(y-a)*sSize->cx/sSize->cy; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);

			for(y=a; y<sSize->cy; y++)
				for(x=sSize->cx-(y-a)*sSize->cx/sSize->cy; x<sSize->cx; x++)
					SetPixelTransparency(ptPixels, sSize, x, y, (BYTE)(0xFF-0xFF*(x-b-(sSize->cy-y)*sSize->cx/sSize->cy)/sSize->cx));
		}
		break;

	case 4:
		// Fade to right and left
		a = sSize->cx*(effect_stage%0x18)/0x30;
		b = sSize->cx/0x02;

		if(effect_stage < 0x18)
		{
			for(y=0; y<sSize->cy; y++)
				for(x=b; x<a+b; x++)
				{
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF*(a-x)/b);
					SetPixelTransparency(ptPixels, sSize, sSize->cx-1-x, y, 0xFF*(a-x)/b);
				}
		}
		else
		{
			for(y=0; y<sSize->cy; y++)
				for(x=b; x<a+b; x++)
				{
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);
					SetPixelTransparency(ptPixels, sSize, sSize->cx-1-x, y, 0xFF);
				}

				for(y=0; y<sSize->cy; y++)
					for(x=a+b; x<sSize->cx; x++)
					{
						SetPixelTransparency(ptPixels, sSize, x, y, 0xFF-0xFF*(x-a-b)/b);
						SetPixelTransparency(ptPixels, sSize, sSize->cx-1-x, y, 0xFF-0xFF*(x-a-b)/b);
					}
		}
		break;

	case 5:
		// Fade up and down
		a = sSize->cy*(effect_stage%0x18)/0x30;
		b = sSize->cy/0x02;

		if(effect_stage < 0x18)
		{
			for(y=b; y<a+b; y++)
				for(x=0; x<sSize->cx; x++)
				{
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF*(a-y)/b);
					SetPixelTransparency(ptPixels, sSize, x, sSize->cy-1-y, 0xFF*(a-y)/b);
				}
		}
		else
		{
			for(y=b; y<a+b; y++)
				for(x=0; x<sSize->cx; x++)
				{
					SetPixelTransparency(ptPixels, sSize, x, y, 0xFF);
					SetPixelTransparency(ptPixels, sSize, x, sSize->cy-1-y, 0xFF);
				}

				for(y=a+b; y<sSize->cy; y++)
					for(x=0; x<sSize->cx; x++)
					{
						SetPixelTransparency(ptPixels, sSize, x, y, 0xFF-0xFF*(y-a-b)/b);
						SetPixelTransparency(ptPixels, sSize, x, sSize->cy-1-y, 0xFF-0xFF*(y-a-b)/b);
					}
		}
		break;
	}
}

void SetPixelTransparency(UINT *ptPixels, SIZE *sSize, int x, int y, BYTE bTransparency)
{
	ptPixels[(sSize->cy-1-y)*sSize->cx+x] |= bTransparency<<0x18;
}

void PreMultiplyRGBChannels(UINT *ptPixels, SIZE *sSize)
{
	LPBYTE pbPixel;
	int x, y;

	pbPixel = (LPBYTE)ptPixels;

	for(y=0; y<sSize->cy; y++)
	{
		for(x=0; x<sSize->cx; x++)
		{
			pbPixel[0] = pbPixel[0]*pbPixel[3]/255;
			pbPixel[1] = pbPixel[1]*pbPixel[3]/255;
			pbPixel[2] = pbPixel[2]*pbPixel[3]/255;

			pbPixel += 4;
		}
	}
}

void FadeOut(HWND hWnd)
{
	if(fade_out == -1)
	{
		fade_out = 1;
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		EnableWindow(hWnd, FALSE);
		CloseHandle(CreateThread(NULL, NULL, FadeThread, hWnd, NULL, NULL));
	}
	else
		fade_out = 1;
}
