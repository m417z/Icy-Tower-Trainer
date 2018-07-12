// Icytower Trainer GUI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "bassmod.h"
#include <stdlib.h>

#define MAINEXE "icytower.exe"
//#define MAINEXE "C:\\Program Files\\icytower\\icytower.exe"

BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam,LPARAM lParam);
void beep();
int frz_wrld(PROCESS_INFORMATION, DWORD, HWND);
int ex_floor(PROCESS_INFORMATION, DWORD, HWND);
int unlock_f(PROCESS_INFORMATION, DWORD, HWND);
int set_scre(PROCESS_INFORMATION, DWORD, HWND, BOOL);
int crt_cmbo(PROCESS_INFORMATION, DWORD, HWND);
int ch_grvty(PROCESS_INFORMATION, DWORD, HWND);

STARTUPINFO         SI;
PROCESS_INFORMATION PI;

BYTE MZ[2];
BOOL game_running=false, sound=true;
DWORD seek[]={0x406669, 0x4096A3, 0x416180, 0x439380, 0x415390};
//	          frz wrld, ex floor, unlock f, set scre, ch grvty

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		if(!BASSMOD_Init(-1,44100,0))
			MessageBox(NULL, "Can't initialize device", "Error", MB_ICONHAND);
		else if(!BASSMOD_MusicLoad(TRUE, 
				LoadResource(NULL, FindResource(NULL, (char*)IDR_MOD, "BASS")),
				0, 16105, BASS_MUSIC_LOOP|BASS_MUSIC_RAMPS|BASS_MUSIC_SURROUND))
			MessageBox(NULL, "Can't play the file", "Error", MB_ICONHAND);
		else
			BASSMOD_MusicPlay();
		return true;
 
	case WM_TIMER:
		if(!ReadProcessMemory(PI.hProcess, (void*)0x400000, MZ, 2, NULL))
			PostQuitMessage(0);
		if(GetAsyncKeyState(VK_F2))
		{
			CheckDlgButton(hwnd, IDC_F2, IsDlgButtonChecked(hwnd, IDC_F2)^1);
			frz_wrld(PI, seek[0], hwnd);
		}
		if(GetAsyncKeyState(VK_F3))
		{
			CheckDlgButton(hwnd, IDC_F3, IsDlgButtonChecked(hwnd, IDC_F3)^1);
			ex_floor(PI, seek[1], hwnd);
		}
		if(GetAsyncKeyState(VK_F4))
			unlock_f(PI, seek[2], hwnd);
		if(GetAsyncKeyState(VK_F5))
			set_scre(PI, seek[3], hwnd, 0);
		if(GetAsyncKeyState(VK_F6))
			set_scre(PI, seek[3], hwnd, 1);
		if(GetAsyncKeyState(VK_F7))
			crt_cmbo(PI, seek[3], hwnd);
		if(GetAsyncKeyState(VK_F8))
		{
			CheckDlgButton(hwnd, IDC_F8, IsDlgButtonChecked(hwnd, IDC_F8)^1);
			ch_grvty(PI, seek[4], hwnd);
		}
		return true;

	case WM_LBUTTONDOWN:
		ReleaseCapture();
		SendMessage(hwnd, 0xA1, 2, 0);
		return true;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_INFO:
			MessageBox(hwnd, "+7 Trainer for Icy Tower 1.2.1\n"
							 "By RaMMicHaeL\n\n"
							 "Options:\n"
							 "F2 - Freeze the clock, the combo meter, and the automatic screen movement [Toggle]\n"
							 "F3 - Make all the floor length same as the screen width [Toggle]\n"
							 "F4 - Unlock any start floor\n"
							 "F5 - Define the score got by combo (Score got by floor is saved separately)\n"
							 "F6 - Increase the score got by combo\n"
							 "F7 - Create any combo\n"
							 "F8 - Change the gravity, default=0.8 [Toggle]\n\n"
							 "Note: Field containing a non-numerical value is treated as 0",
							 "Trainer iNFO", MB_ICONASTERISK);
			return true;

		case IDC_MUSIC:
			if(!game_running)
				if(BASSMOD_MusicIsActive()==BASS_ACTIVE_PLAYING)
				{
					BASSMOD_MusicPause();
					SetDlgItemText(hwnd, IDC_MUSIC, "Music ON");
				}
				else
				{
					BASSMOD_MusicPlay();
					SetDlgItemText(hwnd, IDC_MUSIC, "Music OFF");
				}
			else
				if(sound)
				{
					sound=false;
					SetDlgItemText(hwnd, IDC_MUSIC, "Sound ON");
				}
				else
				{
					sound=true;
					SetDlgItemText(hwnd, IDC_MUSIC, "Sound OFF");
				}
			return true;

		case IDC_EXIT:
			if(game_running &&
			   MessageBox(hwnd, "Exit Icy Tower?",
			   "Do you want to...", MB_ICONQUESTION|MB_YESNO)==IDYES)
				TerminateProcess(PI.hProcess, 0);
			PostQuitMessage(0);
			return true;

		case IDC_RUN:
			if(!CreateProcess(MAINEXE, NULL, NULL, NULL, FALSE,
				NORMAL_PRIORITY_CLASS, NULL, NULL, &SI, &PI))
				MessageBox(hwnd, "Error creating process \"icytower.exe\"", "Error", MB_ICONHAND);
			else
			{
				BASSMOD_Free();
				SetDlgItemText(hwnd, IDC_MUSIC, "Sound OFF");
				int i;
				for(i=0; i<7; i++)
					GetAsyncKeyState(VK_F2+i);
				for(i=0; i<12; i++)
					EnableWindow(GetDlgItem(hwnd, IDC_F2+i), true);
				EnableWindow(GetDlgItem(hwnd, IDC_RUN), false);
				BYTE combo[]={0x83, 0xF9, 0x01, 0x0F, 0x85, 0x52, 0x27,
							  0xFF, 0xFF, 0x49, 0xE9, 0xDE, 0x26, 0xFF, 0xFF};
				WriteProcessMemory(PI.hProcess, (void*)0x414251, &combo, 15, NULL);
				DWORD old_p;
				VirtualProtectEx(PI.hProcess, (void*)0x415390, 8, PAGE_READWRITE, &old_p);
				SetTimer(hwnd, 1, 100, NULL);
				game_running=1;
			}
			return true;

		case IDC_F2:
			frz_wrld(PI, seek[0], hwnd);
			return true;

		case IDC_F3:
			ex_floor(PI, seek[1], hwnd);
			return true;

		case IDC_F4:
			unlock_f(PI, seek[2], hwnd);
			return true;

		case IDC_F5:
			set_scre(PI, seek[3], hwnd, 0);
			return true;

		case IDC_F6:
			set_scre(PI, seek[3], hwnd, 1);
			return true;

		case IDC_F7:
			crt_cmbo(PI, seek[3], hwnd);
			return true;

		case IDC_F8:
			ch_grvty(PI, seek[4], hwnd);
			return true;

		case IDC_CHGRA:
			switch(HIWORD(wParam))
				case EN_KILLFOCUS:
					if(game_running && IsDlgButtonChecked(hwnd, IDC_F8)
						&& ch_grvty(PI, seek[4], hwnd))
						beep();
					break;
			return true;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwnd, IDOK);
		return true;

	case WM_DESTROY:
		if(game_running)
			KillTimer(hwnd, 1);
		else
			BASSMOD_Free();
		EndDialog(hwnd, IDOK);
		return true;

	default:
		return false;
	}
	return true;
}

void beep()
{
	if(sound)
		__asm
		{
			push 0x32
			push 0x04
			mov  eax, 0x77D48466
			call eax
		}
}

int frz_wrld(PROCESS_INFORMATION PI, DWORD seek, HWND hwnd)
{
//	Freeze the clock, the combo meter, and the screen automatic movement
	BYTE temp[4], combo_on []={0xE9, 0x13, 0xD9, 0x00, 0x00},
				  combo_off[]={0x3B, 0xCD, 0x74, 0x6F, 0x49};

	ReadProcessMemory(PI.hProcess, (void*)seek, &temp, 1, NULL);
	if(*temp==0x7E)
		__asm mov dword ptr ds:[temp], 0x000DFFEB
	else if(*temp==0xEB)
		__asm mov dword ptr ds:[temp], 0x002D897E
	else
	{
		CheckDlgButton(hwnd, IDC_F2, BST_UNCHECKED);
		return 1;
	}

	WriteProcessMemory(PI.hProcess, (void*)seek, temp, 1, NULL);
	WriteProcessMemory(PI.hProcess, (void*)(seek+0x7A), temp+1, 2, NULL);

	ReadProcessMemory(PI.hProcess, (void*)0x406939, temp, 1, NULL);
	if(*temp==0x3B)
		WriteProcessMemory(PI.hProcess, (void*)0x406939, combo_on , 5, NULL);
	else if(*temp==0xE9)
		WriteProcessMemory(PI.hProcess, (void*)0x406939, combo_off, 5, NULL);

	beep();
	return 0;
}

int ex_floor(PROCESS_INFORMATION PI, DWORD seek, HWND hwnd)
{
//	Make all the floor length same as the screen width
	BYTE temp, on[]={0xBE, 0x1D, 0x00, 0x00, 0x00},
			  off[]={0x8B, 0xF2, 0x83, 0xC6, 0x06};

	ReadProcessMemory(PI.hProcess, (void*)seek, &temp, 1, NULL);
	if(temp==0x8B)
		WriteProcessMemory(PI.hProcess, (void*)seek, on, 5, NULL);
	else if(temp==0xBE)
		WriteProcessMemory(PI.hProcess, (void*)seek, off, 5, NULL);
	else
	{
		CheckDlgButton(hwnd, IDC_F3, BST_UNCHECKED);
		return 1;
	}

	beep();
	return 0;
}

int unlock_f(PROCESS_INFORMATION PI, DWORD seek, HWND hwnd)
{
//	Unlock any start floor
	char str[2];

	GetDlgItemText(hwnd, IDC_LEVEL, str, 3);
	*str-='0';
	if(*str>=0 && *str<=9 && *(str+1)=='\0')
		WriteProcessMemory(PI.hProcess, (void*)seek, str, 1, NULL);
	else
		return 1;

	beep();
	return 0;
}

int set_scre(PROCESS_INFORMATION PI, DWORD seek, HWND hwnd, BOOL add)
{
//	Set the score got by combo
	char str[11];
	long score;

	GetDlgItemText(hwnd, IDC_SCORECH+add*2, str, 12);
	if(!*str)
		return 1;

	ReadProcessMemory(PI.hProcess, (void*)seek, &seek, 4, NULL);
	ReadProcessMemory(PI.hProcess, (void*)(seek*4+0x41F320), &seek, 4, NULL);
	seek+=0x2C;

	if(add)
	{
		ReadProcessMemory(PI.hProcess, (void*)seek, &score, 4, NULL);
		score+=atoi(str);
	}
	else
		score=atoi(str);

	WriteProcessMemory(PI.hProcess, (void*)seek, &score, 4, NULL);

	beep();
	return 0;
}

int crt_cmbo(PROCESS_INFORMATION PI, DWORD seek, HWND hwnd)
{
//	Create a combo
	BYTE temp=0x01;
	char str[11];
	long combo;

	GetDlgItemText(hwnd, IDC_CRTCMB, (LPTSTR)str, 12);
	if(!*str)
		return 1;
	combo=atoi(str);

	ReadProcessMemory(PI.hProcess, (void*)seek, &seek, 4, NULL);
	ReadProcessMemory(PI.hProcess, (void*)(seek*4+0x41F320), &seek, 4, NULL);

	WriteProcessMemory(PI.hProcess, (void*)(seek+0x40), &temp, 1, NULL);
	WriteProcessMemory(PI.hProcess, (void*)(seek+0x44), &combo, 4, NULL);
	temp++;
	WriteProcessMemory(PI.hProcess, (void*)(seek+0x48), &temp, 1, NULL);

	beep();
	return 0;
}

int ch_grvty(PROCESS_INFORMATION PI, DWORD seek, HWND hwnd)
{
//	Change the gravity
	char str[11];
	double grvty;

	GetDlgItemText(hwnd, IDC_CHGRA, str, 12);
	if(!IsDlgButtonChecked(hwnd, IDC_F8))
		grvty=0.8;
	else if(!*str)
	{
		CheckDlgButton(hwnd, IDC_F8, BST_UNCHECKED);
		return 1;
	}
	else
		grvty=atof(str);

	WriteProcessMemory(PI.hProcess, (void*)seek, &grvty, 8, NULL);

	beep();
	return 0;
}
