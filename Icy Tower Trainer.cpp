#include <windows.h>
#include "buffer.h"
#include "FadeEffect.h"
#include "bassmod.h"
#include "crc32.h"
#include "atof_binary.h"
#include "TrainerFunc.h"
#include "resource.h"

LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void RegisterHotKeys(HWND hWnd);
void EnableOptions(HWND hWnd, BOOL bEnable);
HANDLE RunGame();
UINT GetExecutableDirectory(LPSTR lpBuffer, UINT uSize);
HANDLE RunIcyTower(LPSTR lpFileName);
HANDLE AttachGame();
DWORD WINAPI ProcessWaitThread(LPVOID pNotPointerJustProcessHandle);
BOOL SoundsExist();
void ActivateTrainerOption(HANDLE hProcess, int option, BOOL bSound);

HINSTANCE hInst;
HWND hWnd_;

int main()
{
	hInst = GetModuleHandle(NULL);

	ExitProcess(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc));
}

LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH m_hbrWall = NULL;
	static BOOL bBassModLoaded = FALSE;
	static HANDLE hProcess = NULL;
	static BOOL bSoundOn = FALSE;
	HBITMAP hBitmap;
	char szClassName[6];
	MSGBOXPARAMS MsgBoxParams;
	HWND hPopup;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		hWnd_ = hWnd;

		SetWindowText(hWnd, "Icy Tower +7 Trainer");
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));

		hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		if(hBitmap)
		{
			m_hbrWall = CreatePatternBrush(hBitmap);
			DeleteObject(hBitmap);
		}

		RegisterHotKeys(hWnd);
		EnableOptions(hWnd, FALSE);

		bBassModLoaded = LoadBassmod(MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
		if(bBassModLoaded)
		{
			if(!LoadBassFile(MAKEINTRESOURCE(IDR_RCDATA2), RT_RCDATA))
			{
				FreeBassmod();
				SetDlgItemText(hWnd, IDC_MUSIC, "No music");
				EnableWindow(GetDlgItem(hWnd, IDC_MUSIC), FALSE);
			}
			else
				BASSMOD_MusicPlay();
		}
		else
		{
			SetDlgItemText(hWnd, IDC_MUSIC, "No music");
			EnableWindow(GetDlgItem(hWnd, IDC_MUSIC), FALSE);
		}

		FadeIn(hWnd);
		break;

	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
		break;

	case WM_CTLCOLORDLG:
		return (LRESULT)m_hbrWall;

	case WM_CTLCOLORSTATIC:
		if(m_hbrWall)
		{
			GetClassName((HWND)lParam, szClassName, 6);
			if(*(LPDWORD)szClassName!=0x74696445 /* "Edit" */ || szClassName[4])
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)m_hbrWall;
			}
		}
		break;

	case WM_HOTKEY:
		if(hProcess && wParam>=0 && wParam<=6)
			ActivateTrainerOption(hProcess, wParam, bSoundOn);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDC_ATTACH:
			if(LOWORD(wParam) == IDOK)
				hProcess = RunGame();
			else
				hProcess = AttachGame();

			if(hProcess)
			{
				if(bBassModLoaded)
				{
					FreeBassmod();
					bBassModLoaded = FALSE;
				}

				EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_ATTACH), FALSE);

				if(!PrepareProcess(hProcess))
				{
					MessageBox(GetLastActivePopup(hWnd), "Process initialization failed", NULL, MB_ICONHAND);
					FadeOut(hWnd);
				}
				else
				{
					bSoundOn = SoundsExist();
					if(!bSoundOn)
					{
						SetDlgItemText(hWnd, IDC_MUSIC, "No sound");
						EnableWindow(GetDlgItem(hWnd, IDC_MUSIC), FALSE);
					}
					else
						SetDlgItemText(hWnd, IDC_MUSIC, "Sound OFF");

					EnableOptions(hWnd, TRUE);
					CloseHandle(CreateThread(NULL, NULL, ProcessWaitThread, hProcess, NULL, NULL));
				}
			}
			break;

		case IDC_F2:
		case IDC_F3:
		case IDC_F4:
		case IDC_F5:
		case IDC_F6:
		case IDC_F7:
		case IDC_F8:
			ActivateTrainerOption(hProcess, LOWORD(wParam)-IDC_F2, bSoundOn);
			break;

		case IDC_INFO:
			ZeroMemory(&MsgBoxParams, sizeof(MSGBOXPARAMS));

			MsgBoxParams.cbSize = sizeof(MSGBOXPARAMS);
			MsgBoxParams.hwndOwner = GetLastActivePopup(hWnd);
			MsgBoxParams.hInstance = hInst;
			MsgBoxParams.lpszText = 
				"+7 Trainer for Icy Tower 1.3.1\n"
				"Version 2.1\n"
				"By RaMMicHaeL\n"
				"http://rammichael.com/\n"
				"\n"
				"Compiled on: " __DATE__ "\n"
				"\n"
				"Options:\n"
				"F2 - Freeze the clock, the combo meter, and the automatic screen movement [Toggle]\n"
				"F3 - Make all the floor length same as the screen width [Toggle]\n"
				"F4 - Unlock any start floor\n"
				"F5 - Set a score\n"
				"F6 - Increase the score\n"
				"F7 - Create a combo\n"
				"F8 - Change the gravity, default is 0.8 [Toggle]";
			MsgBoxParams.lpszCaption = "Trainer iNFO";
			MsgBoxParams.dwStyle = MB_USERICON;
			MsgBoxParams.lpszIcon = MAKEINTRESOURCE(IDI_ICON1);

			MessageBoxIndirect(&MsgBoxParams);
			break;

		case IDC_MUSIC:
			if(!hProcess)
			{
				switch(BASSMOD_MusicIsActive())
				{
				case BASS_ACTIVE_PLAYING:
					BASSMOD_MusicPause();
					SetDlgItemText(hWnd, IDC_MUSIC, "Music ON");
					break;

				case BASS_ACTIVE_PAUSED:
					BASSMOD_MusicPlay();
					SetDlgItemText(hWnd, IDC_MUSIC, "Music OFF");
					break;
				}
			}
			else
			{
				if(bSoundOn)
				{
					bSoundOn = FALSE;
					SetDlgItemText(hWnd, IDC_MUSIC, "Sound ON");
				}
				else
				{
					bSoundOn = TRUE;
					SetDlgItemText(hWnd, IDC_MUSIC, "Sound OFF");
				}
			}
			break;

		case IDCANCEL:
			FadeOut(hWnd);
			break;
		}
		break;

	case WM_ONFADEOUT:
		hPopup = GetLastActivePopup(hWnd);
		SetActiveWindow(hPopup);
		EndDialog(hPopup, 0);

		while(hPopup != hWnd)
		{
			hPopup = GetLastActivePopup(hWnd);
			EndDialog(hPopup, 0);
		}
		break;

	case WM_DESTROY:
		if(m_hbrWall)
		{
			DeleteObject(m_hbrWall);
			m_hbrWall = NULL;
		}

		if(bBassModLoaded)
		{
			BASSMOD_MusicFree();
			FreeBassmod();
		}

		if(hProcess)
			CloseHandle(hProcess);
		break;
	}

	return FALSE;
}

void RegisterHotKeys(HWND hWnd)
{
	char szErrorMsg[sizeof("Could not register the following hotkeys:\nF2, F3, F4, F5, F6, F7, F8")];
	int error_count;
	int i;

	error_count = 0;
	lstrcpy(szErrorMsg, "Could not register the following hotkeys:\nF2, F2, F2, F2, F2, F2, F2");

	for(i=0; i<7; i++)
	{
		if(!RegisterHotKey(hWnd, i, NULL, VK_F2+i))
		{
			szErrorMsg[sizeof("Could not register the following hotkeys:\n")-1+error_count*4+1] += i;
			error_count++;
		}
	}

	if(error_count)
	{
		szErrorMsg[sizeof("Could not register the following hotkeys:\n")-1+error_count*4-2] = '\0';
		MessageBox(GetLastActivePopup(hWnd), szErrorMsg, NULL, MB_ICONHAND);
	}
}

void EnableOptions(HWND hWnd, BOOL bEnable)
{
	int i;

	for(i=IDC_F2; i<=IDC_F8; i++)
		EnableWindow(GetDlgItem(hWnd, i), bEnable);
}

HANDLE RunGame()
{
	char szFileName[MAX_PATH];
	OPENFILENAME ofn;

	if(
		(
			FindWindow("AllegroWindow", "Icy Tower v1.3") || 
			FindWindow("AllegroWindow", "Icy Tower v1.3.1")
		) && 
		MessageBox(
			GetLastActivePopup(hWnd_), "Icy Tower seems to be already running\n"
			"Are you sure you want to run another instance of the game?", 
			"Did you know?", MB_YESNO|MB_ICONASTERISK
		) == IDNO
	)
		return NULL;

	GetExecutableDirectory(szFileName, MAX_PATH);
	lstrcat(szFileName, "icytower13.exe");

	if(GetFileAttributes(szFileName) == INVALID_FILE_ATTRIBUTES)
	{
		lstrcpy(szFileName, "icytower13.exe");

		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetLastActivePopup(hWnd_);
		ofn.lpstrFilter = 
			"Icy Tower v1.3.1 (icytower13.exe)\0icytower13.exe\0"
			"Executable files (*.exe)\0*.exe\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = "C:\\games\\icytower1.3\\";
		ofn.lpstrTitle = "Choose your Icy Tower v1.3.1 executable";
		ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "exe";

		if(!GetOpenFileName(&ofn))
			return NULL;
	}

	return RunIcyTower(szFileName);
}

UINT GetExecutableDirectory(LPSTR lpBuffer, UINT uSize)
{
	int pos;

	pos = GetModuleFileName(NULL, lpBuffer, uSize)-1;
	while(pos>0 && lpBuffer[pos]!='\\')
		pos--;

	pos++;
	lpBuffer[pos] = '\0';

	return pos;
}

HANDLE RunIcyTower(LPSTR lpFileName)
{
	STARTUPINFO SI;
	PROCESS_INFORMATION PI;
	DWORD dwCrc32;
	BOOL bSuccess;

	dwCrc32 = CalcFileCrc32(lpFileName, &bSuccess);
	if(!bSuccess)
	{
		MessageBox(GetLastActivePopup(hWnd_), "Could not calculate CRC32", NULL, MB_ICONHAND);
		return NULL;
	}

	switch(dwCrc32)
	{
	case 0xB0532179:
		// v1.3
		MessageBox(GetLastActivePopup(hWnd_), "You seem to have Icy Tower v1.3\nThis trainer supports v1.3.1 only", NULL, MB_ICONHAND);
		return NULL;

	case 0x1CB0543E:
		// v1.3.1
		// Fine
		break;

	default:
		if(MessageBox(GetLastActivePopup(hWnd_), "CRC32 does not match\nTry to run anyway?", NULL, MB_YESNO|MB_ICONASTERISK)!=IDYES)
			return NULL;
		break;
	}

	ZeroMemory(&SI, sizeof(STARTUPINFO));
	SI.cb = sizeof(STARTUPINFO);

	if(!CreateProcess(lpFileName, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &SI, &PI))
	{
		MessageBox(GetLastActivePopup(hWnd_), "Could not create process", NULL, MB_ICONHAND);
		return NULL;
	}

	CloseHandle(PI.hThread);
	return PI.hProcess;
}

HANDLE AttachGame()
{
	HWND hIcy;
	DWORD dwProcessId;
	HANDLE hProcess;

	hIcy = FindWindow("AllegroWindow", "Icy Tower v1.3.1");
	if(!hIcy)
	{
		if(FindWindow("AllegroWindow", "Icy Tower v1.3"))
			MessageBox(GetLastActivePopup(hWnd_), "Looks like Icy Tower v1.3 is running\nThis trainer supports v1.3.1 only", NULL, MB_ICONHAND);
		else
			MessageBox(GetLastActivePopup(hWnd_), "Icy Tower doesn't seem to be running", NULL, MB_ICONHAND);

		return NULL;
	}

	GetWindowThreadProcessId(hIcy, &dwProcessId);

	hProcess = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|
		PROCESS_QUERY_INFORMATION|SYNCHRONIZE, FALSE, dwProcessId);
	if(!hProcess)
	{
		MessageBox(GetLastActivePopup(hWnd_), "Could not open process", NULL, MB_ICONHAND);
		return NULL;
	}

	return hProcess;
}

DWORD WINAPI ProcessWaitThread(LPVOID pNotPointerJustProcessHandle)
{
	HANDLE hProcess;

	hProcess = (HANDLE)pNotPointerJustProcessHandle;
	WaitForSingleObject(hProcess, INFINITE);

	FadeOut(hWnd_);
	return 0;
}

BOOL SoundsExist()
{
	return 
		FindResource(NULL, MAKEINTRESOURCE(IDR_WAVE1), "WAVE") && 
		FindResource(NULL, MAKEINTRESOURCE(IDR_WAVE2), "WAVE");
}

void ActivateTrainerOption(HANDLE hProcess, int option, BOOL bSound)
{
	int int_field;
	char szBuffer[11];
	DWORD dwDoubleField[2];
	BOOL bTranslated;
	int result;

	if(option >= 2)
	{
		if(option == 6)
		{
			if(SendDlgItemMessage(hWnd_, IDC_F8_TEXT, WM_GETTEXTLENGTH, NULL, NULL) <= 10)
			{
				GetDlgItemText(hWnd_, IDC_F8_TEXT, szBuffer, 11);
				bTranslated = atof_binary(szBuffer, dwDoubleField, '.');
			}
			else
				bTranslated = FALSE;
		}
		else
			int_field = GetDlgItemInt(hWnd_, IDC_F4_TEXT+option-2, &bTranslated, TRUE);
	}
	else
		bTranslated = TRUE;

	if(bTranslated)
	{
		switch(option)
		{
		case 0:
			result = FreezeWorld(hProcess);
			break;

		case 1:
			result = ExFloor(hProcess);
			break;

		case 2:
			result = UnlockFloor(hProcess, int_field);
			break;

		case 3:
			result = SetScore(hProcess, int_field);
			break;

		case 4:
			result = SetScore(hProcess, GetScore(hProcess)+int_field);
			break;

		case 5:
			result = CreateCombo(hProcess, int_field);
			break;

		case 6:
			result = ChangeGravity(hProcess, dwDoubleField);
			break;
		}
	}
	else
		result = 0;

	if(bSound)
	{
		switch(result)
		{
		case 0:
			MessageBeep(MB_ICONEXCLAMATION);
			break;

		case 1:
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), hInst, SND_RESOURCE|SND_ASYNC);
			break;

		case 2:
			PlaySound(MAKEINTRESOURCE(IDR_WAVE2), hInst, SND_RESOURCE|SND_ASYNC);
			break;
		}
	}
}
