// Icytower Trainer GUI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "bassmod.h"

#pragma comment(lib, "BASSMOD.lib")

#define LWA_ALPHA       0x00000002
#define WS_EX_LAYERED   0x00080000

#define MAINEXE         "icytower13.exe"

VOID Initialize(HINSTANCE hInstance, LPTSTR lpFileName);
LRESULT CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
VOID BeepSound();
VOID Prepare2Work();
VOID FadeAndExit();
BOOL FreezeWorld();
BOOL ExFloor();
BOOL UnlockFloor();
BOOL SetOrAddScore(BOOL bAdd);
BOOL CreateCombo();
BOOL ChangeGravity();

BOOL (WINAPI* SetLayeredWindowAttributes)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
BYTE bAlpha;
BOOL bFadeOut;
PROCESS_INFORMATION PI;
OPENFILENAME ofn;
HWND hWnd_;
HINSTANCE hInst;
BOOL GameRunning, SoundOn=TRUE;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char szFileName[MAX_PATH]=MAINEXE;

	Initialize(hInstance, szFileName);

	return DialogBox(hInstance, (LPTSTR)IDD_MAIN, NULL, (DLGPROC)DlgProc);
}

VOID Initialize(HINSTANCE hInstance, LPTSTR lpFileName)
{
	hInst=hInstance;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.lpstrFilter=
		"Icy Tower v1.3.1 (icytower13.exe)\0icytower13.exe\0"
		"Executable files (*.exe)\0*.exe\0";
	ofn.lpstrFile=lpFileName;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrInitialDir="C:\\Program Files\\icytower1.3";
	ofn.lpstrTitle="Choose your Icy Tower v1.3.1 Executable";
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	ofn.lpstrDefExt="exe";

    SetLayeredWindowAttributes = (BOOL (WINAPI*)(HWND, COLORREF, BYTE, DWORD)) 
		GetProcAddress(LoadLibrary("user32.dll"), "SetLayeredWindowAttributes");
}

LRESULT CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			int i;

			ofn.hwndOwner=hWnd;
			hWnd_=hWnd;

			BASSMOD_Init(-1, 44100, 0);
			BASSMOD_MusicLoad(
				TRUE, 
				LoadResource(NULL, FindResource(NULL, (LPTSTR)IDR_MOD, "BASS")), 
				0, 
				16105, 
				BASS_MUSIC_LOOP|BASS_MUSIC_RAMPS|BASS_MUSIC_SURROUND
			);
			BASSMOD_MusicPlay();

			SendMessage(hWnd, WM_SETTEXT, NULL, (LPARAM)"Icy Tower +7 Trainer");

			EnableWindow(GetDlgItem(hWnd, IDC_F3), FALSE);
			for(i=0; i<6; i++)
				EnableWindow(GetDlgItem(hWnd, IDC_F2+i*2), FALSE);

			SendMessage(GetDlgItem(hWnd, IDC_LEVEL), EM_SETLIMITTEXT, 1, NULL);
			for(i=0; i<4; i++)
				SendMessage(GetDlgItem(hWnd, IDC_SCORECH+i*2), EM_SETLIMITTEXT, 10, NULL);

			if(SetLayeredWindowAttributes)
			{
				SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetTimer(hWnd, 0x01, 10, NULL);
			}
			break;

		case WM_TIMER:
			switch(wParam)
			{
				case 1:
					SetLayeredWindowAttributes(hWnd, NULL, bFadeOut?bAlpha-=0x0F:bAlpha+=0x0F, LWA_ALPHA);
					if(bAlpha==0xFF)
					{
						SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) ^ WS_EX_LAYERED);
						KillTimer(hWnd, 0x01);
					}
					else if(bAlpha==0x00)
					{
						KillTimer(hWnd, 0x01);
						DestroyWindow(hWnd);
					}
					break;

				case 2:
			 		WORD wMZ;
					WORD wID;

					if(!ReadProcessMemory(PI.hProcess, (LPVOID)0x00400000, &wMZ, sizeof(WORD), NULL))
					{
						FadeAndExit();
						return TRUE;
					}

					for(i=0; i<7; i++) if(GetAsyncKeyState(VK_F2+i))
					{
						wID = IDC_F2+(i>2?i*2-2:i);
						if(i<2)
							CheckDlgButton(hWnd, wID, IsDlgButtonChecked(hWnd, wID) ^ 1);
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(wID, NULL), (LPARAM)GetDlgItem(hWnd, wID));
					}
					break;
			}
			break;

		case WM_LBUTTONDOWN:
			SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
			break;

		case WM_PAINT:
			HBITMAP hBitmap;
			RECT rc;
			PAINTSTRUCT ps;
			HDC hPaintDC, hTempDC;

			hBitmap=LoadBitmap(hInst, (LPTSTR)IDB_BACK);
			GetClientRect(hWnd, &rc);
			hPaintDC=BeginPaint(hWnd, &ps);
			hTempDC=CreateCompatibleDC(NULL);
			SelectObject(hTempDC, hBitmap);
			BitBlt(hPaintDC, 0, 0, rc.right, rc.bottom, hTempDC, 0, 0, SRCCOPY);
			DeleteDC(hTempDC);
			EndPaint(hWnd, &ps);
			DeleteObject(hBitmap);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_INFO:
					MSGBOXPARAMS MsgBoxParams;

					ZeroMemory(&MsgBoxParams, sizeof(MSGBOXPARAMS));
					MsgBoxParams.cbSize=sizeof(MSGBOXPARAMS);
					MsgBoxParams.hwndOwner=hWnd;
					MsgBoxParams.hInstance=hInst;
					MsgBoxParams.lpszText=
						"+7 Trainer for Icy Tower 1.3.1\n"
						"Version 1.2\n"
						"By RaMMicHaeL\n\n"
						"Options:\n"
						"F2 - Freeze the clock, the combo meter, and the automatic screen movement [Toggle]\n"
						"F3 - Make all the floor length same as the screen width [Toggle]\n"
						"F4 - Unlock any start floor\n"
						"F5 - Define the score got by combo (Score got by floor is saved separately)\n"
						"F6 - Increase the score got by combo\n"
						"F7 - Create a combo\n"
						"F8 - Change the gravity, default is 0.8";
					MsgBoxParams.lpszCaption="Trainer iNFO";
					MsgBoxParams.dwStyle=MB_USERICON;
					MsgBoxParams.lpszIcon=(LPTSTR)IDI_MAINICON;

					MessageBoxIndirect(&MsgBoxParams);
					break;

				case IDC_MUSIC:
					if(!GameRunning)
						if(BASSMOD_MusicIsActive()==BASS_ACTIVE_PLAYING)
						{
							BASSMOD_MusicPause();
							SetDlgItemText(hWnd, IDC_MUSIC, "Music ON");
						}
						else
						{
							BASSMOD_MusicPlay();
							SetDlgItemText(hWnd, IDC_MUSIC, "Music OFF");
						}
					else
						if(SoundOn)
						{
							SoundOn=FALSE;
							SetDlgItemText(hWnd, IDC_MUSIC, "Sound ON");
						}
						else
						{
							SoundOn=TRUE;
							SetDlgItemText(hWnd, IDC_MUSIC, "Sound OFF");
						}
					break;

				case IDCANCEL:
					FadeAndExit();
					return TRUE;

				case IDC_RUN:
					if(GameRunning)
					{
						MessageBox(hWnd, "Huh?", NULL, MB_ICONQUESTION);
						break;
					}

					STARTUPINFO SI;

					if(FindWindow("AllegroWindow", "Icy Tower v1.3.1") && 
						MessageBox(hWnd, "Icy Tower seems to be already running\n"
							"Are you sure you want to run another instance of the game?", 
							"Did you know?", MB_ICONQUESTION|MB_YESNO)==IDNO)
						break;

					ZeroMemory(&SI, sizeof(STARTUPINFO));
					SI.cb=sizeof(STARTUPINFO);
					if(!CreateProcess(MAINEXE, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &SI, &PI))
					{
						if(!GetOpenFileName(&ofn))
							break;
						else if(!CreateProcess(ofn.lpstrFile, NULL, NULL, NULL, FALSE, 
							NORMAL_PRIORITY_CLASS, NULL, NULL, &SI, &PI))
						{
							MessageBox(hWnd, "Error creating process", NULL, MB_ICONASTERISK);
							break;
						}
					}

					CloseHandle(PI.hThread);
					Prepare2Work();
					break;

				case IDC_FIND:
					if(GameRunning)
					{
						MessageBox(hWnd, "Huh?", NULL, MB_ICONQUESTION);
						break;
					}

					HWND hIcy;

					hIcy=FindWindow("AllegroWindow", "Icy Tower v1.3.1");
					if(!hIcy)
					{
						MessageBox(hWnd, "Icy Tower doesn't seem to be running", NULL, MB_ICONASTERISK);
						break;
					}

					GetWindowThreadProcessId(hIcy, &PI.dwProcessId);
					PI.hProcess=OpenProcess(PROCESS_ALL_ACCESS, FALSE, PI.dwProcessId);
					Prepare2Work();
					break;

				case IDC_F2:
					FreezeWorld();
					break;

				case IDC_F3:
					ExFloor();
					break;

				case IDC_F4:
					UnlockFloor();
					break;

				case IDC_F5:
					SetOrAddScore(FALSE);
					break;

				case IDC_F6:
					SetOrAddScore(TRUE);
					break;

				case IDC_F7:
					CreateCombo();
					break;

				case IDC_F8:
					ChangeGravity();
					break;

				default:
					return false;
			}
			break;

		case WM_DESTROY:
			if(GameRunning)
			{
				KillTimer(hWnd, 0x01);
				CloseHandle(PI.hProcess);
			}
			else
 				BASSMOD_Free();
			break;

		default:
			return false;
	}
	return 0;
}

VOID BeepSound()
{
	if(SoundOn)
		MessageBeep(MB_ICONASTERISK);
}

VOID Prepare2Work()
{
	BYTE ClockPatch[]={0x90, 0xFF, 0x0D};
	int i;

	BASSMOD_Free();

	SetDlgItemText(hWnd_, IDC_MUSIC, "Sound OFF");

	EnableWindow(GetDlgItem(hWnd_, IDC_F3), TRUE);
	for(i=0; i<6; i++)
		EnableWindow(GetDlgItem(hWnd_, IDC_F2+i*2), TRUE);
	EnableWindow(GetDlgItem(hWnd_, IDC_RUN), FALSE);
	EnableWindow(GetDlgItem(hWnd_, IDC_FIND), FALSE);

	WaitForInputIdle(PI.hProcess, INFINITE);
	WriteProcessMemory(PI.hProcess, (LPVOID)0x40E90E, &ClockPatch, 3, NULL);
	WriteProcessMemory(PI.hProcess, (LPVOID)0x40E915, &ClockPatch, 3, NULL);

	for(i=0; i<7; i++)
		GetAsyncKeyState(VK_F2+i);

	SetTimer(hWnd_, 2, 100, NULL);
	GameRunning=TRUE;
}

VOID FadeAndExit()
{
	if(SetLayeredWindowAttributes && bAlpha)
	{
		bFadeOut=TRUE;
		if(bAlpha==0xFF)
		{
			SetWindowLong(hWnd_, GWL_EXSTYLE, GetWindowLong(hWnd_, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetTimer(hWnd_, 0x01, 10, NULL);
		}
	}
	else
		DestroyWindow(hWnd_);
}

BOOL FreezeWorld()
{
//	Freeze the clock, the combo meter, and the screen automatic movement
	BYTE temp[2], ComboOn []={0x3C, 0x01, 0x75},
				  ComboOff[]={0x85, 0xC0, 0x74};

	ReadProcessMemory(PI.hProcess, (LPVOID)0x40E829, temp, 1, NULL);
	if(*temp==0x0F)
		*(LPWORD)temp=0xE990;
	else if(*temp==0x90)
		*(LPWORD)temp=0x8E0F;
	else
	{
		CheckDlgButton(hWnd_, IDC_F2, BST_UNCHECKED);
		return 1;
	}

	WriteProcessMemory(PI.hProcess, (LPVOID)0x40E829, temp, 2, NULL);

	ReadProcessMemory(PI.hProcess, (LPVOID)0x40E96F, temp, 1, NULL);
	if(*temp==*ComboOff)
		WriteProcessMemory(PI.hProcess, (LPVOID)0x40E96F, ComboOn , 3, NULL);
	else if(*temp==*ComboOn)
		WriteProcessMemory(PI.hProcess, (LPVOID)0x40E96F, ComboOff, 3, NULL);

	BeepSound();
	return 0;
}

BOOL ExFloor()
{
//	Make all the floor length same as the screen width
	BYTE temp, On[]={0x90, 0xB2, 0x22},
			  Off[]={0x83, 0xC2, 0x01, 0xDA};

	ReadProcessMemory(PI.hProcess, (LPVOID)0x411819, &temp, 1, NULL);
	if(temp==*Off)
	{
		WriteProcessMemory(PI.hProcess, (LPVOID)0x411819, On, 2, NULL);
		WriteProcessMemory(PI.hProcess, (LPVOID)0x411822, On+1, 2, NULL);
	}
	else if(temp==*On)
	{
		WriteProcessMemory(PI.hProcess, (LPVOID)0x411819, Off, 2, NULL);
		WriteProcessMemory(PI.hProcess, (LPVOID)0x411822, Off+2, 2, NULL);
	}
	else
	{
		CheckDlgButton(hWnd_, IDC_F3, BST_UNCHECKED);
		return 1;
	}

	BeepSound();
	return 0;
}

BOOL UnlockFloor()
{
//	Unlock any start floor
	BOOL bTranslated;
	int Floor;

	Floor=GetDlgItemInt(hWnd_, IDC_LEVEL, &bTranslated, TRUE);
	if(bTranslated && Floor>=0 && Floor<=9)
		WriteProcessMemory(PI.hProcess, (LPVOID)0x49E238, &Floor, 1, NULL);
	else
		return 1;

	BeepSound();
	return 0;
}

BOOL SetOrAddScore(BOOL bAdd)
{
//	Set the score got by combo
	DWORD dwSeek=0x4CB908;
	BOOL bTranslated;
	long Score;

	Score=GetDlgItemInt(hWnd_, IDC_SCORECH+bAdd*2, &bTranslated, TRUE);
	if(!bTranslated)
		return 1;

	ReadProcessMemory(PI.hProcess, (LPVOID)dwSeek, &dwSeek, 4, NULL);
	ReadProcessMemory(PI.hProcess, (LPVOID)(dwSeek*4+0x4CB920), &dwSeek, 4, NULL);
	dwSeek+=0x2C;

	if(bAdd)
	{
		long temp;

		ReadProcessMemory(PI.hProcess, (LPVOID)dwSeek, &temp, 4, NULL);
		Score+=temp;
	}

	WriteProcessMemory(PI.hProcess, (LPVOID)dwSeek, &Score, 4, NULL);

	BeepSound();
	return 0;
}

BOOL CreateCombo()
{
//	Create a combo
	DWORD dwSeek=0x4CB908;
	BOOL bTranslated;
	BYTE temp=0x01;
	long Combo;

	Combo=GetDlgItemInt(hWnd_, IDC_CRTCMB, &bTranslated, TRUE);
	if(!bTranslated)
		return 1;

	ReadProcessMemory(PI.hProcess, (LPVOID)dwSeek, &dwSeek, 4, NULL);
	ReadProcessMemory(PI.hProcess, (LPVOID)(dwSeek*4+0x4CB920), &dwSeek, 4, NULL);

	WriteProcessMemory(PI.hProcess, (LPVOID)(dwSeek+0x40), &temp, 1, NULL);
	WriteProcessMemory(PI.hProcess, (LPVOID)(dwSeek+0x44), &Combo, 4, NULL);
	temp++;
	WriteProcessMemory(PI.hProcess, (LPVOID)(dwSeek+0x48), &temp, 1, NULL);

	BeepSound();
	return 0;
}

BOOL ChangeGravity()
{
//	Change the gravity
	char szValue[11];
	double dValue;

	GetDlgItemText(hWnd_, IDC_CHGRA, szValue, 12);
	if(!*szValue)
		return 1;
	else
		dValue=atof(szValue);

	WriteProcessMemory(PI.hProcess, (LPVOID)0x4AD810, &dValue, 8, NULL);

	BeepSound();
	return 0;
}
