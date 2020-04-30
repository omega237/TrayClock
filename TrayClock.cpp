/*
Tray Clock ist a simple clock program that resides in the taskbar notification area

Copyright (C) 2004  omega237

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


// TRANSFEATURESENABLED SETZEN UND ABFRAGEN!!!!!!!!!!!
// NOCH SKINFUNKTION EINBAUEN
// settings laden. einstellen und ¸bernehmen ¸berarbeiten -> am besten das tray menu ‰ndern, daﬂ man dort nur about, alarms und settings machen kann

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "CleanCompile.h" // switches some unnecessary compiler warnings off for warning level 4
#include "AnalogueClock.h"
#include "resource.h"


// macros for trackbar
#define TrackBar_GetPos(hwndCtl)                   ((int)(DWORD)SNDMSG((hwndCtl), TBM_GETPOS, 0L, 0L))
#define TrackBar_SetPos(hwndCtl,tbpos)             ((int)(DWORD)SNDMSG((hwndCtl), TBM_SETPOS, true, tbpos))
#define TrackBar_SetMinRange(hwndCtl, tbmin)       ((BOOL)(DWORD)SNDMSG((hwndCtl), TBM_SETRANGEMIN, 0L, tbmin))
#define TrackBar_SetMaxRange(hwndCtl, tbmax)       ((BOOL)(DWORD)SNDMSG((hwndCtl), TBM_SETRANGEMAX, 0L, tbmax))

// macros for combo box
#define ComboBox_GetCursel(hwnd)				   SendMessage(hwnd, CB_GETCURSEL, 0, 0)
#define ComboVox_SetCursel(hwnd, i)				   SendMessage(hwnd, CB_SETCURSEL, (WPARAM)i, 0)

// macros for edit boxes
#define SetEditText(hwndEdit, text)				   SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)text)
#define GetEditTextLength(hwndEdit)				   SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0)
#define GetEditText(hwndEdit, buf)				   SendMessage(hwndEdit, WM_GETTEXT, (WPARAM)GetEditTextLength(hwndEdit), (LPARAM)buf)

// some constants
#define WS_EX_LAYERED	0x80000 // window style for layered windows
#define LWA_COLORKEY	1
#define LWA_ALPHA		2
#define WM_TRAYICON		WM_USER+88 // user-defined message, sent by tray icon

HINSTANCE hAppInstance = NULL; // application instance
HWND hMainWindow = NULL; // main window handle
HMENU hTrayMenu = NULL; // tray menu handle
HICON hTrayIcon = NULL; // tray icon handle
ATOM hAtom = NULL;

AnalogueClock *acClock = NULL; // THE analog clock

UINT uShowTimerId = 0; // auto hide timer id
UINT uShowTimerTime = 1000; // auto hide timer time
UINT uTrayIconId = 1; // tray icon id

bool bLayered = false; // window transparent?
int iBlendFactor = 100; // transparency factor
bool bShaped = false; // clock shaped window?
bool bAutoHide = true; // auto hide clock window
bool bVisible = false; // is window visible
bool bChangeSettings = false; // changing settings?
bool bRound = false; // regionned window?
bool bShowDate = false;

const char szClassName[] = "TrayClockClass";
const char szAppTitle[] = "TrayClock";
const char szSettingsFile[] = "config.tcc";
const char szAlarmFile[] = "alarms.tcc";

char szError[] = "Error";
char szFRegisterWc[] = "Could not register window class";
char szFCreateWnd[] = "Could not create main window";
char szFCreateClock[] = "Could not create clock window";
char szFLoadRes[] = "Could not load resources";
char szFailSave[] = "Could not save settings/alarms";
char szErrW2kReq[] = "This feature requires at least Windows 2000";
char szMaxAlarms[] = "You can only have a maximum of 15 alarms.";

char szDisclaimer[] = "TrayClock Version 1.0 by omega237\r\nThis program was created for the pure programming experience of the author.\r\n\r\nLEGAL DISCLAIMER\r\nThe TrayClock program provided by omega237 may be freely distributed, provided that no charge above the cost of distribution is levied, and that the disclaimer below is always attached to it. TrayClock is provided as is without any guarantees or warranty. Although the author has attempted to find and correct any bugs in TrayClock, the author is not responsible for any damage or losses of any kind caused by the use or misuse of TrayClock. The author is under no obligation to provide support, service, corrections, or upgrades to TrayClock.\r\n\r\nE-Mail: hidden\r\n\r\nTHIS PROGRAM WAS CREATED USING THE C++ LANGUAGE AND THE WIN32 APPLICATION PROGRAMMING INTERFACE";
char szHowto[] = "How To...\nCreate an alarm\n\nFirst select the alarm mode from the combobox near the label \"Alarm Mode\". The alarm mode determines which values you have to enter for time and date.\n\nOne Shot:\tThe alert will be activated on the specified time and date and the will be deactivated.\nDaily:\t\tThe alert will be activated every day on the specified time. You can leave the preset date, it will be ignored.\nWeekly:\t\tThe alert will be activated on the specified time and date and from since on every 7 days on the specified time.\nWeekdays:\tThe alert will be activated on every weekday on the specified time, date is ignored.\nWeekends:\tSame as Weekdays but on weekends.\n\nAfter entering a time and if necessary a date, select the action you want the alarm to execute if it is activated.\n\nPlay Wave:\tPlays the sound file specified in the Path edit box. To search for a file hit search.\nExecute:\t\tWill start the file specified in Path. Is like a double click on the file in windows explorer.\nShutdown:\tWill shut down your PC. No questions.\nShow MsgBox:\tWill show a messagebox with the text specified in the edit box. Maximum length is 255 chars.\n\n\nREMARKS:\n- IT IS BETTER TO ENTER ONLY VALID VALUES FOR TIME AND DATE OTHERWISE THE ALARM COULD NEVER FIRE OR WILL FIRE NOT AS EXPECTED.\n- IF YOU WANNA HEAR A SOUND ON ALERT, SELECT A SOUND FILE! OTHERWISE YOU WILL HEAR NOTHING.";

enum ClockPos { UpperLeft = 1, UpperRight = 2, LowerLeft = 3, LowerRight = 4, Middle = 5, UserDefined = 6 };
ClockPos cpPosition = LowerRight;
int cpUserX = 0, cpUserY = 0;

enum ClockSize { Size1 = 100, Size2 = 200, Size3 = 300, Size4 = 400 };
ClockSize csSize = Size2;

enum ClockAppearance { Classic = 1, Round = 2, Transparent = 3, TransparentRound = 4, Clocked = 5, Skin = 6 };
ClockAppearance caAppearance = Classic;

COLORREF clBackground = RGB(95,95,95); // background color
COLORREF clForeground = RGB(215,215,215); // foreground color
COLORREF clSpecial = RGB(255,128,64); // special color

struct ProgramSettings
{
	ClockSize cs;
	ClockPos cp;
	int x,y; // used if pos is user defined;
	DWORD Foreground;
	DWORD Background;
	DWORD Special;
	bool Autohide;
	ClockAppearance Appearance;
	int BlendFactor;
	UINT Timer;
	bool ShowDate;
	bool Visible;
};

enum AlarmMode { OneShot = 0, Daily = 1, Weekly = 2, Weekdays = 3, Weekends = 4 };
enum AlarmAction { PlayWav = 0, Exec = 1, Shutdown = 2, ShowMsg = 3 };

struct AlarmEvent
{
	bool Active; // is alarm active
	AlarmMode aMode; // alert mode
	AlarmAction aAction; // alert action
	SYSTEMTIME EventTime; // alert time
	char Path[MAX_PATH]; // alert path for actions 0 and 1
};

AlarmEvent aeAlarms[15]; // alert list
int iNumAlarms = 0;

bool bMouseDown = false;
POINT pOldPos;


//===== SHUTDOWN TAKEN FROM PLATFORM SDK ======================================
// shuts down windows 2000
BOOL ShutdownSystem()
{
   HANDLE hToken; 
   TOKEN_PRIVILEGES tkp; 
 
   // Get a token for this process. 
 
   if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
      return( FALSE ); 
 
   // Get the LUID for the shutdown privilege. 
 
   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 
 
   tkp.PrivilegeCount = 1;  // one privilege to set    
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
   // Get the shutdown privilege for this process. 
 
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
 
   if (GetLastError() != ERROR_SUCCESS) 
      return FALSE; 
 
   // Shut down the system and force all applications to close. 
 
   if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0)) 
      return FALSE; 

   return TRUE;
}


//===== List box specific ===========================================

// creates the listbox line for an alarm event
void CreateAlarmText(char *buf, AlarmEvent *ae)
{
	buf[0] = '\0';
	if(ae->Active)
	{ strcat(buf, "Active | "); }
	else
	{ strcat(buf, "Disabled | "); }

	char timestr[100];
	wsprintf(timestr, "%.2d:%.2d:%.2d | ", ae->EventTime.wHour, ae->EventTime.wMinute, ae->EventTime.wSecond);
	strcat(buf, timestr);
	wsprintf(timestr, "%.2d.%.2d:%.4d | ", ae->EventTime.wDay, ae->EventTime.wMonth, ae->EventTime.wYear);
	strcat(buf, timestr);

	switch(ae->aMode)
	{
	case OneShot:
		strcat(buf, "Once | ");
		break;
	case Daily:
		strcat(buf, "Daily | ");
		break;
	case Weekly:
		strcat(buf, "Weekly | ");
		break;
	case Weekdays:
		strcat(buf, "Weekdays | ");
		break;
	case Weekends:
		strcat(buf, "Weekends | ");
		break;
	};

	switch(ae->aAction)
	{
	case PlayWav:
		strcat(buf, "Play Sound | ");
		strcat(buf, ae->Path);
		break;
	case Exec:
		strcat(buf, "Execute | ");
		strcat(buf, ae->Path);
		break;
	case Shutdown:
		strcat(buf, "Shutdown");
		break;
	case ShowMsg:
		strcat(buf, "Show Message | ");
		strcat(buf, ae->Path);
		break;
	};
	
	return;
};

// fills the list
void RefillList(HWND hwndList)
{
	SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
	if(iNumAlarms > 0)
	{
		char str[1000];
		for(int i=0; i<iNumAlarms; i++)
		{
			CreateAlarmText(str, &aeAlarms[i]);
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)str);
		}
	}
};

// ads an alarm event to list
void AddAlarmEvent(AlarmEvent ae)
{
	if(iNumAlarms < 15)
	{
		aeAlarms[iNumAlarms].aAction = ae.aAction;
		aeAlarms[iNumAlarms].Active = ae.Active;
		aeAlarms[iNumAlarms].aMode = ae.aMode;
		aeAlarms[iNumAlarms].EventTime = ae.EventTime;
		strcpy(aeAlarms[iNumAlarms].Path, ae.Path);	
		iNumAlarms++;
	};
};

// deletes an alarm event from list
void DeleteAlarmEvent(int idx)
{
	if(idx < (iNumAlarms-1))
	{
		for(int i=(idx+1); i<iNumAlarms; i++)
		{
			aeAlarms[i-1].aAction = aeAlarms[i].aAction;
			aeAlarms[i-1].Active = aeAlarms[i].Active;
			aeAlarms[i-1].aMode = aeAlarms[i].aMode;
			aeAlarms[i-1].EventTime = aeAlarms[i].EventTime;
			strcpy(aeAlarms[i-1].Path, aeAlarms[i].Path);
		}
	}
	iNumAlarms--;
};

// changes a specified alarm event
void ModifyAlarmEvent(AlarmEvent* mod, AlarmEvent ae)
{
	mod->aAction = ae.aAction;
	mod->Active = ae.Active;
	mod->aMode = ae.aMode;
	mod->EventTime = ae.EventTime;
	strcpy(mod->Path, ae.Path);
};

// goes through list of alarms
AlarmEvent* MatchAlarmEvent(DayTime dt)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	for(int i=0; i<iNumAlarms; i++)
	{
		if(aeAlarms[i].Active)
		{
			if(dt == acClock->SystemtimeToIntTime(aeAlarms[i].EventTime))
			{
				switch(aeAlarms[i].aMode)
				{
				case Daily: // w/o date
					return &aeAlarms[i];
					break;
				case Weekends:
					if((st.wDayOfWeek < 1) || (st.wDayOfWeek > 5))
						return &aeAlarms[i];
					break;
				case Weekdays:
					if((st.wDayOfWeek > 0) && (st.wDayOfWeek < 6))
						return &aeAlarms[i];
					break;
				case OneShot: // w date
					if((st.wDay == aeAlarms[i].EventTime.wDay) && (st.wMonth == aeAlarms[i].EventTime.wMonth) && (st.wYear == aeAlarms[i].EventTime.wYear))
					{
						aeAlarms[i].Active = false;
						return &aeAlarms[i];
					}
					break;
				case Weekly:
					if(st.wDayOfWeek == aeAlarms[i].EventTime.wDayOfWeek)
						return &aeAlarms[i];
					break;
				}
			}
		}
	}
	return NULL;
};

// saves alarms to alarms file
void SaveAlarms()
{
	if(!iNumAlarms) return;

	HANDLE AlarmsFile;
	DWORD numbytes;

	AlarmsFile = CreateFile(szAlarmFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(AlarmsFile == INVALID_HANDLE_VALUE)
	{ 
		MessageBox(hMainWindow, szFailSave, szError, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	WriteFile(AlarmsFile, &iNumAlarms, sizeof(iNumAlarms), &numbytes, NULL);
	for(int i=0; i<iNumAlarms; i++)
	{
		WriteFile(AlarmsFile, &aeAlarms[i], sizeof(AlarmEvent), &numbytes, NULL);
	}
	CloseHandle(AlarmsFile);

};

// loads alarms from alarms file
void LoadAlarms()
{
	HANDLE AlarmsFile;
	DWORD numbytes;

	AlarmsFile = CreateFile(szAlarmFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(AlarmsFile == INVALID_HANDLE_VALUE)
	{ 
		return;
	}
	ReadFile(AlarmsFile, &iNumAlarms, sizeof(iNumAlarms), &numbytes, NULL);
	for(int i=0; i<iNumAlarms; i++)
	{
		ReadFile(AlarmsFile, &aeAlarms[i], sizeof(AlarmEvent), &numbytes, NULL);
	}
	CloseHandle(AlarmsFile);
};


//-------------------------------------------------------------------


//===== Checks for Windows 2000 or above OS =========================

bool Win2000()
{
	OSVERSIONINFO vi = { sizeof(vi) };
    GetVersionEx(&vi);
    if ((vi.dwPlatformId != VER_PLATFORM_WIN32_NT) && (vi.dwMajorVersion < 5)) 
	{
		return false;   
	}
	return true;
};

//===== Tray Icon related ===========================================

// deletes icon from tna
int DeleteTrayIcon(HWND hwnd, UINT uID)
{
	int res;
	NOTIFYICONDATA tnid;

	tnid.cbSize		= sizeof(NOTIFYICONDATA);
	tnid.hWnd		= hwnd;
	tnid.uID		= uID;

	res = Shell_NotifyIcon(NIM_DELETE, &tnid);

	return res;
};

// adds icon to tna
int AddTrayIcon(HWND hwnd, UINT uID, HICON hIcon)
{
	int res;
	NOTIFYICONDATA tnid;

	tnid.cbSize				= sizeof(NOTIFYICONDATA);
	tnid.hWnd				= hwnd;
	tnid.uID				= uID;
	tnid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uCallbackMessage	= WM_TRAYICON;
	tnid.hIcon				= hIcon;

	lstrcpyn(tnid.szTip, szAppTitle, sizeof(tnid.szTip));

	res = Shell_NotifyIcon(NIM_ADD, &tnid);

	return res;
};

// message handler for tray icon messages
void TrayIconMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UINT uID;
	UINT uMouseMsg;

	uID = (UINT) wParam;
	uMouseMsg = (UINT) lParam;

	if(!bChangeSettings)
	{
		switch(uMouseMsg)
		{
			case WM_MOUSEMOVE:
				break;
			case WM_LBUTTONDOWN:
				if(bAutoHide)
				{
					ShowWindow(hwnd, SW_SHOW);
					SetForegroundWindow(hwnd);
					bVisible = true;
					SetTimer(hwnd, 0, uShowTimerTime, NULL);
				}
				else
				{
					SetForegroundWindow(hwnd);
					ShowWindow(hwnd, (bVisible?SW_HIDE:SW_SHOW));
					bVisible = !bVisible;
				}
				break;
			case WM_RBUTTONDOWN:
				POINT cpos;
				GetCursorPos(&cpos);
				SetForegroundWindow(hwnd);
				TrackPopupMenuEx(hTrayMenu, TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON, cpos.x, cpos.y, hwnd, NULL);	
				PostMessage(hwnd, WM_NULL, 0, 0);
				break;
		}
	}
};

//-------------------------------------------------------------------

//===== Transparency related ========================================

// applies transparency settings to a window
void BlendWindow(HWND hwnd, COLORREF cl, int blend_factor, bool transcol = false)
{
	typedef DWORD (WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);
	PSLWA pSetLayeredWindowAttributes;
	HMODULE hDLL = LoadLibrary ("USER32");
	DWORD col;
	DWORD style;

	if(blend_factor == 100) // means total opacity
	{
		SetWindowLong (hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE ) & ~WS_EX_LAYERED);
		return;
	}

	pSetLayeredWindowAttributes = (PSLWA) GetProcAddress(hDLL,"SetLayeredWindowAttributes");
	
	if (pSetLayeredWindowAttributes != NULL)
	{
		col = cl;
		if(!transcol)
		{
			style = LWA_ALPHA;
			SetWindowLong (hwnd , GWL_EXSTYLE, GetWindowLong (hwnd, GWL_EXSTYLE ) | WS_EX_LAYERED ) ;
			pSetLayeredWindowAttributes (hwnd, 	col, (unsigned char)((255 * blend_factor)/100), style);
		}
		else
		{
			style = LWA_COLORKEY;
			SetWindowLong (hwnd , GWL_EXSTYLE, GetWindowLong (hwnd, GWL_EXSTYLE ) | WS_EX_LAYERED ) ;
			pSetLayeredWindowAttributes (hwnd, 	col, (unsigned char)((255 * blend_factor)/100), style);
		}
	}
};

//-------------------------------------------------------------------

//===== initialization and cleanup ==================================

// loads menus and ressources
bool LoadProgramData()
{
	HMENU hm = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_TRAYMENU));
	hTrayMenu = GetSubMenu(hm, 0);
	if(hTrayMenu == NULL) return false;
	return true;
};

// deletes vars and so on
void CleanUp()
{
	DeleteTrayIcon(hMainWindow, uTrayIconId);
	DestroyMenu(hTrayMenu);
	DestroyWindow(hMainWindow);
	GlobalDeleteAtom(hAtom);
	delete acClock;
};

// sets the main window to a new position
void PosClockWindow()
{
	RECT screenRect;
	int x = 0, y = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);
	switch(cpPosition)
	{
	case UpperLeft:
		break;
	case UpperRight:
		x = screenRect.right-csSize;
		break;
	case LowerLeft:
		y = screenRect.bottom-csSize;
		break;
	case LowerRight:
		x = screenRect.right-csSize;
		y = screenRect.bottom-csSize;
		break;
	case Middle:
		x = (screenRect.right-csSize)/2;
		y = (screenRect.bottom-csSize)/2;
		break;
	case UserDefined:
		x = cpUserX;
		y = cpUserY;
		break;
	}
	SetWindowPos(hMainWindow, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
};

// resizes the main window
void ResizeWindow()
{
	SetWindowPos(hMainWindow, HWND_TOP, 0, 0, csSize, csSize, SWP_NOMOVE);
	RECT dim;
	dim.left = dim.top = 0;
	dim.bottom = dim.right = csSize;
	acClock->SetDimensions(dim);
	PosClockWindow();
};

//-------------------------------------------------------------------

// saves the programs settings to the config file
void SaveProgramSettings()
{
	HANDLE SettingsFile;
	DWORD numbytes;

	ProgramSettings ps;
	ps.Autohide = bAutoHide;
	ps.Background = clBackground;
	ps.Foreground = clForeground;
	ps.Special = clSpecial;
	ps.cp = cpPosition;
	RECT r;
	GetWindowRect(hMainWindow, &r);
	ps.x = r.left;
	ps.y = r.top;
	ps.cs = csSize;
	ps.Appearance = caAppearance;
	ps.Timer = uShowTimerTime;
	ps.ShowDate = bShowDate;
	ps.Visible = bVisible;
	ps.BlendFactor = iBlendFactor;

	SettingsFile = CreateFile(szSettingsFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(SettingsFile == INVALID_HANDLE_VALUE)
	{ 
		MessageBox(hMainWindow, szFailSave, szError, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	WriteFile(SettingsFile, &ps, sizeof(ps), &numbytes, NULL);
	CloseHandle(SettingsFile);
};

// loads the programs settings from the settings file
void LoadProgramSettings()
{
	ProgramSettings ps;
	HANDLE SettingsFile;
	DWORD numbytes;

	SettingsFile = CreateFile(szSettingsFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(SettingsFile == INVALID_HANDLE_VALUE)
	{ 
		return;
	}
	ReadFile(SettingsFile, &ps, sizeof(ps), &numbytes, NULL);
	CloseHandle(SettingsFile);

	bAutoHide = ps.Autohide;
	clBackground = ps.Background;
	clForeground = ps.Foreground;
	clSpecial = ps.Special;
	cpPosition = ps.cp;
	cpUserX = ps.x;
	cpUserY = ps.y;
	csSize = ps.cs;
	caAppearance = ps.Appearance;
	uShowTimerTime = ps.Timer;
	bShowDate = ps.ShowDate;
	iBlendFactor = ps.BlendFactor;
};

//-------------------------------------------------------------------

//===== GETS CALLED EVERY SECOND BY ANALOGUE CLOCK ============================
void AlarmCallback(AnalogueClock *ac, DayTime pt)
{
	// add alarm functionality
	// if match then do the action
	AlarmEvent *ev;
	ev = MatchAlarmEvent(pt);
	if(ev != NULL)
	{
		switch(ev->aAction)
		{
		case PlayWav:
			PlaySound(ev->Path, NULL, SND_ASYNC|SND_NOWAIT|SND_FILENAME);
			break;
		case Exec:
			ShellExecute(NULL, "open", ev->Path, NULL, NULL, SW_SHOWNORMAL);
			break;
		case Shutdown:
			ShutdownSystem();
			break;
		case ShowMsg:
			MessageBox(hMainWindow, ev->Path, "TrayClock Alarm", MB_OK);
			break;
		}
		if(ev->aMode == OneShot)
		{ ev->Active = false; };
	}
	return;
};


//===== the procedure for the edit or new alarm dialog ========================
BOOL CALLBACK AddAlarmProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char szFile[MAX_PATH];
	static bool Edit;
	static AlarmEvent* myae;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		if(lParam == NULL)
		{ // select first entries from combo boxes
			Edit = false;
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"One Shot");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Daily");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Weekly");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Weekdays");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Weekends");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Play Wave");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Execute");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Shutdown");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Show MessageBox");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_SETCURSEL, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_SETCURSEL, 0, 0);
			SYSTEMTIME s;
			GetLocalTime(&s);
			SetDlgItemInt(hwndDlg, IDC_ALARMHOUR, s.wHour, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMMINUTE, s.wMinute, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMSECOND, s.wSecond, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMDAY, s.wDay, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMMONTH, s.wMonth, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMYEAR, s.wYear, false);
			CheckDlgButton(hwndDlg, IDC_ALARMACTIVE, BST_CHECKED);
		}
		else
		{ // fill dlg with lParam Data
			Edit = true;
			myae = (AlarmEvent*)lParam;
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"One Shot");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Daily");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Weekly");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Weekdays");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_ADDSTRING, 0, (LPARAM)"Weekends");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Play Wave");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Execute");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Shutdown");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_ADDSTRING, 0, (LPARAM)"Show MessageBox");
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_SETCURSEL, myae->aMode, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_SETCURSEL, myae->aAction, 0);
			SetDlgItemInt(hwndDlg, IDC_ALARMHOUR, myae->EventTime.wHour, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMMINUTE, myae->EventTime.wMinute, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMSECOND, myae->EventTime.wSecond, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMDAY, myae->EventTime.wDay, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMMONTH, myae->EventTime.wMonth, false);
			SetDlgItemInt(hwndDlg, IDC_ALARMYEAR, myae->EventTime.wYear, false);
			SetDlgItemText(hwndDlg, IDC_ALARMPATH, myae->Path);
			if(myae->Active)
			{ CheckDlgButton(hwndDlg, IDC_ALARMACTIVE, BST_CHECKED); };
		}
		return 0;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK: // check if event is in the past and if so show msgbox and let user reenter event time!!!!!!!
			AlarmEvent newae;
			newae.EventTime.wHour = (unsigned short)GetDlgItemInt(hwndDlg, IDC_ALARMHOUR, NULL, false);
			newae.EventTime.wMinute = (unsigned short)GetDlgItemInt(hwndDlg, IDC_ALARMMINUTE, NULL, false);
			newae.EventTime.wSecond = (unsigned short)GetDlgItemInt(hwndDlg, IDC_ALARMSECOND, NULL, false);
			newae.EventTime.wDay = (unsigned short)GetDlgItemInt(hwndDlg, IDC_ALARMDAY, NULL, false);
			newae.EventTime.wMonth = (unsigned short)GetDlgItemInt(hwndDlg, IDC_ALARMMONTH, NULL, false);
			newae.EventTime.wYear = (unsigned short)GetDlgItemInt(hwndDlg, IDC_ALARMYEAR, NULL, false);
			newae.aMode = (AlarmMode)SendMessage(GetDlgItem(hwndDlg, IDC_ALARMMODE), CB_GETCURSEL, 0, 0);
			newae.aAction = (AlarmAction)SendMessage(GetDlgItem(hwndDlg, IDC_ALARMACTION), CB_GETCURSEL, 0, 0);
			/*if(!Win2000() && newae.aAction == Shutdown)
			{
				MessageBox(hwndDlg, szErrW2kReq, szError, MB_ICONINFORMATION);
				EndDialog(hwndDlg, 0);
			}*/
			GetDlgItemText(hwndDlg, IDC_ALARMPATH, newae.Path, MAX_PATH);
			if(IsDlgButtonChecked(hwndDlg, IDC_ALARMACTIVE) == BST_CHECKED)
			{ newae.Active = true; } else { newae.Active = false; };
			if(Edit)
			{ ModifyAlarmEvent(myae, newae); }
			else
			{ AddAlarmEvent(newae); }
			EndDialog(hwndDlg, 0);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		case IDC_SEARCH:
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Alle\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if(GetOpenFileName(&ofn))
			{ SetDlgItemText(hwndDlg, IDC_ALARMPATH, szFile); };

			break;
		case IDC_HOWTO:
			MessageBox(hwndDlg, szHowto, "How To...", MB_ICONINFORMATION);
			break;
		case IDC_ALARMACTION:
			// if 2 shutdown then check for win2000 if not the sel first entry
			if(ComboBox_GetCursel(GetDlgItem(hwndDlg, IDC_ALARMACTION))==2)
			{
				if(!Win2000())
				{
					MessageBox(hwndDlg, "Shutdown only available on Windows 2000", "Error", MB_ICONEXCLAMATION);
					ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_ALARMACTION), 0);
				}
			};

			break;
		}
		break;

	}
	return false;
};


//===== the dialog procedure for the alarm list dialog ========================
BOOL CALLBACK AlarmsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int sel;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		RefillList(GetDlgItem(hwndDlg, IDC_ALARMSLIST));
		SendMessage(GetDlgItem(hwndDlg, IDC_ALARMSLIST), LB_SETHORIZONTALEXTENT, (WPARAM)1000, NULL);
		return 0;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, 0);
			break;
		case IDC_ADD: // open add dlg box
			if(iNumAlarms < 15)
			{
				DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_ADDALARM), hwndDlg, AddAlarmProc, 0);
				RefillList(GetDlgItem(hwndDlg, IDC_ALARMSLIST));
			}
			else
			{
				MessageBox(hwndDlg, szMaxAlarms, szError, MB_ICONEXCLAMATION);
			}
			break;
		case IDC_EDIT: // open add dlg box with param
			sel = SendMessage(GetDlgItem(hwndDlg, IDC_ALARMSLIST), LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR)
			{
				DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_ADDALARM), hwndDlg, AddAlarmProc, (LPARAM)&aeAlarms[sel]);
				RefillList(GetDlgItem(hwndDlg, IDC_ALARMSLIST));
			}
			break;
		case IDC_REMOVE: // get current index and call RemoveAlarmEvent
			sel = SendMessage(GetDlgItem(hwndDlg, IDC_ALARMSLIST), LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR)
			{
				DeleteAlarmEvent(sel);
				RefillList(GetDlgItem(hwndDlg, IDC_ALARMSLIST));
			}
			break;
		case IDC_CLEARALL:
			iNumAlarms = 0;
			RefillList(GetDlgItem(hwndDlg, IDC_ALARMSLIST));
			break;
		case IDC_CLEARD:
			for(int i=0; i<iNumAlarms; i++)
			{ 
				if(!aeAlarms[i].Active)
				{
					DeleteAlarmEvent(i);
					i--; // if next is disabled we wont miss that and to prevent infinite loops
				}
			}
			RefillList(GetDlgItem(hwndDlg, IDC_ALARMSLIST));			
			break;
		}
		break;

	}
	return false;
};

//===== the Dialog Procedure for the About Dialog
BOOL CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_ABOUTTEXT, szDisclaimer);
		return 0;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, 0);
			break;
		}
		break;

	}
	return false;
};

// zeichnet einen "virtuellen bildschirm auf dem hdc mit einem bevel, der BevelWidth Pixel breit ist und einem Frame der FrameWidth breit ist
void DrawScreen(int BevelWidth, int FrameWidth, HDC dc)
{
	HBRUSH hBlackBr = CreateSolidBrush(RGB(0,0,0));
	HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255,255,255));
	HBRUSH hShadowBrush = CreateSolidBrush(RGB(119,119,119));
	HBRUSH hGreyBrush = CreateSolidBrush(RGB(215,215,215));

	RECT r;
	r.left = 25;
	r.top = 27;
	r.bottom = 140;
	r.right = 150;
	
	FillRect(dc, &r, hWhiteBrush); // auﬂerer Highlight
	r.top += BevelWidth;
	r.left += BevelWidth;
	FillRect(dc, &r, hShadowBrush); // schatten auﬂen
	r.right-=BevelWidth;r.bottom-=BevelWidth;
	FillRect(dc, &r, hGreyBrush); // rahmen
	r.top += FrameWidth; r.left+= FrameWidth; r.right -= FrameWidth; r.bottom-=FrameWidth;
	FillRect(dc, &r, hShadowBrush); // Schatten innen
	r.top += BevelWidth; r.left+=BevelWidth;
	FillRect(dc, &r, hWhiteBrush); // highlight innen
	r.bottom -= BevelWidth; r.right-=BevelWidth;
	FillRect(dc, &r, hBlackBr);


	DeleteObject(hBlackBr);
	DeleteObject(hWhiteBrush);
	DeleteObject(hShadowBrush);
	DeleteObject(hGreyBrush);
};


//===== the Dialog Procedure for the Settings Dialogue ========================
BOOL CALLBACK SettingsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char str[1000];
	INITCOMMONCONTROLSEX icce;

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hwndDlg;
	cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	static COLORREF acrCustClr[16]; 
	cc.lpCustColors = (LPDWORD) acrCustClr;
	switch(uMsg)
	{
		case WM_DRAWITEM:
			HBRUSH br; 
			br = NULL;
			switch(wParam)
			{
			case IDC_BKGNDINDSTATIC: // background indicator wants to be drawn
				br = CreateSolidBrush(clBackground);
				FillRect(((LPDRAWITEMSTRUCT)lParam)->hDC, &((LPDRAWITEMSTRUCT)lParam)->rcItem, br);
				break;
			case IDC_FGNDINDSTATIC: // foreground indicator wants to be drawn
				br = CreateSolidBrush(clForeground);
				FillRect(((LPDRAWITEMSTRUCT)lParam)->hDC, &((LPDRAWITEMSTRUCT)lParam)->rcItem, br);
				break;
			case IDC_SPINDSTATIC: // special indicator wants to be drawn
				br = CreateSolidBrush(clSpecial);
				FillRect(((LPDRAWITEMSTRUCT)lParam)->hDC, &((LPDRAWITEMSTRUCT)lParam)->rcItem, br);
				break;
			}
			DeleteObject(br);
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
			case IDC_VISIBILITYSLIDER:
				int t;
				t = TrackBar_GetPos(GetDlgItem(hwndDlg,IDC_VISIBILITYSLIDER));
				wsprintf(str, "%d ms", t);
				SetDlgItemText(hwndDlg, IDC_VISIBILITYTIME, str);
				uShowTimerTime = t; // set timer value, time becomes active next time the window is hidden
				break;
			case IDC_TRANSPARENCYSLIDER:
				int v;
				v = TrackBar_GetPos(GetDlgItem(hwndDlg,IDC_TRANSPARENCYSLIDER));
				wsprintf(str, "%d%%", v);
				SetDlgItemText(hwndDlg, IDC_TRANSPARENCYVALUE, str);	
				iBlendFactor = v;
				if((caAppearance == Transparent)||(caAppearance == TransparentRound))
					BlendWindow(hMainWindow, 0, 100-iBlendFactor);
				break;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_REFRESH:	// refresh the time
				PSYSTEMTIME st;
				st = new SYSTEMTIME;
				GetLocalTime(st);
				acClock->SetTime(st);
				break;
			case IDOK: // einstellungen ¸bernehmen
				bChangeSettings = false;
				/*SaveVars();
				ApplyChanges();*/
				ShowWindow(hMainWindow, (bVisible?SW_SHOW:SW_HIDE));
				EndDialog(hwndDlg, 0);
				break;
			case IDC_AUTOHIDE: // disable or enable autohide
				bAutoHide = !bAutoHide;
				break;
			case IDC_SHOWDATE: // enable or disable displaying of date
				bShowDate = !bShowDate;
				acClock->SetShowDate(bShowDate);
				break;
			case IDC_CLASSIC: // classic quadratic window
				acClock->UnloadSkin();
				caAppearance = Classic;
				SetWindowRgn(hMainWindow, NULL, true);
				BlendWindow(hMainWindow, 0, 100);
				acClock->SetBackground(clBackground);
				break;
			case IDC_ROUND: // round clock face shaped window
				acClock->UnloadSkin();
				caAppearance = Round;
				HRGN rgn;
				rgn = CreateEllipticRgn(0, 0, csSize, csSize);
				SetWindowRgn(hMainWindow, rgn, true);
				BlendWindow(hMainWindow, 0, 100);
				acClock->SetBackground(clBackground);
				break;
			case IDC_TRANS: // quadratic transparent
				if(Win2000()) // check for windows 2000, because uses functions that are only avail in w2k+
				{
					acClock->UnloadSkin();
					if(MessageBox(hwndDlg, "Do you wanna go round?", "Question", MB_YESNO|MB_ICONQUESTION)==IDYES)
					{
						caAppearance = TransparentRound;
						acClock->SetBackground(clBackground);
						HRGN rgn;
						rgn = CreateEllipticRgn(0, 0, csSize, csSize);
						SetWindowRgn(hMainWindow, rgn, true);
						BlendWindow(hMainWindow, 0, 100-iBlendFactor);
					}
					else
					{
						acClock->SetBackground(clBackground);
						caAppearance = Transparent;
						SetWindowRgn(hMainWindow, NULL, true);
						BlendWindow(hMainWindow, 0, 100-iBlendFactor);
					}
				}
				else
				{
					acClock->UnloadSkin();
					MessageBox(hwndDlg, "Not available! Requires at least Windows 2000.", "Error", MB_OK);
					CheckDlgButton(hwndDlg, IDC_CLASSIC, true);
					caAppearance = Classic;
				}
				break;
			case IDC_SHAPE: // clock shaped window, you only see face and hands, no background
				if(Win2000())
				{
					acClock->UnloadSkin();
					SetWindowRgn(hMainWindow, NULL, true);
					if(acClock->GetForeground() != RGB(255,255,255))
					{
						acClock->SetBackground(RGB(255,255,255));
						BlendWindow(hMainWindow, RGB(255,255,255), 100-iBlendFactor, true);
					}
					else
					{
						acClock->SetBackground(RGB(0,0,0));
						BlendWindow(hMainWindow, RGB(0,0,0), 100-iBlendFactor, true);
					}
				}
				else
				{
					MessageBox(hwndDlg, "Not available! Requires at least Windows 2000.", "Error", MB_OK);
					CheckDlgButton(hwndDlg, IDC_CLASSIC, true);
					caAppearance = Classic;
					acClock->UnloadSkin();
				}
				break;
			case IDC_UPPERLEFT: // upper left of screen
				cpPosition = UpperLeft;
				PosClockWindow();
				break;
			case IDC_UPPERRIGHT: // upper right of screen
				cpPosition = UpperRight;
				PosClockWindow();
				break;
			case IDC_LOWERLEFT: // lower left of screen
				cpPosition = LowerLeft;
				PosClockWindow();
				break;
			case IDC_LOWERRIGHT: // lower right of screen
				cpPosition = LowerRight;
				PosClockWindow();
				break;
			case IDC_MIDDLE: // middle of the screen
				cpPosition = Middle;
				PosClockWindow();
				break;
			case IDC_SIZE1: // 100x100
				csSize = Size1;
				ResizeWindow();
				if(caAppearance == Round || caAppearance == TransparentRound)
				{
					SetWindowRgn(hMainWindow, NULL, true);
					HRGN rgn;
					rgn = CreateEllipticRgn(0, 0, csSize, csSize);
					SetWindowRgn(hMainWindow, rgn, true);
				}
				break;
			case IDC_SIZE2: // 200x200
				csSize = Size2;
				ResizeWindow();
				if(caAppearance == Round || caAppearance == TransparentRound)
				{
					SetWindowRgn(hMainWindow, NULL, true);
					HRGN rgn;
					rgn = CreateEllipticRgn(0, 0, csSize, csSize);
					SetWindowRgn(hMainWindow, rgn, true);
				}
				break;
			case IDC_SIZE3: // 300x300
				csSize = Size3;
				ResizeWindow();
				if(caAppearance == Round || caAppearance == TransparentRound)
				{
					SetWindowRgn(hMainWindow, NULL, true);
					HRGN rgn;
					rgn = CreateEllipticRgn(0, 0, csSize, csSize);
					SetWindowRgn(hMainWindow, rgn, true);
				}
				break;
			case IDC_CHOSEBG: // background
				cc.rgbResult = clBackground;
				if(ChooseColor(&cc))
				{
					clBackground = cc.rgbResult;
					ShowWindow(GetDlgItem(hwndDlg, IDC_BKGNDINDSTATIC), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_BKGNDINDSTATIC), SW_SHOW);
				}
				if(caAppearance != Clocked) // wenn die uhr nicht clocked ist, kann der hintergrund ver‰ndert werden
					acClock->SetBackground(clBackground);
				break;
			case IDC_CHOSEFG: // foregorund
				cc.rgbResult = clForeground;
				if(ChooseColor(&cc))
				{
					clForeground = cc.rgbResult;
					ShowWindow(GetDlgItem(hwndDlg, IDC_FGNDINDSTATIC), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_FGNDINDSTATIC), SW_SHOW);
				}
				acClock->SetForeground(clForeground);
				break;
			case IDC_CHOSESP: // special color
				cc.rgbResult = clSpecial;
				if(ChooseColor(&cc))
				{
					clSpecial = cc.rgbResult;
					ShowWindow(GetDlgItem(hwndDlg, IDC_SPINDSTATIC), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_SPINDSTATIC), SW_SHOW);
				}
				acClock->SetSpecial(clSpecial);
				break;
			case IDC_SKIN:
			case IDC_SEARCH: // search for skin file
				ClockAppearance prevca = caAppearance;
				int touncheck = 0;
				switch(prevca)
				{
				case Classic:
					touncheck = IDC_CLASSIC;
					break;
				case Round:
					touncheck = IDC_ROUND;
					break;
				case Transparent:
				case TransparentRound:
					touncheck = IDC_TRANS;
					break;
				case Clocked:
					touncheck = IDC_SHAPE;
					break;
				}
				CheckDlgButton(hwndDlg, IDC_SKIN, true);
				CheckDlgButton(hwndDlg, touncheck, false);
				OPENFILENAME ofn;
				caAppearance = Skin;
				char szFile[260];       // buffer for file name
				// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFile = szFile;
				//
				// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
				// use the contents of szFile to initialize itself.
				//
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = "Tray Clock Skin\0*.tcs\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if((GetOpenFileName(&ofn)==0)||(ofn.lpstrFile[0] == '\0'))
				{
					MessageBox(hwndDlg, "An Error has occured! Setting Appearance to Classic!", "Error", MB_ICONEXCLAMATION);
					caAppearance = Classic;
					SetWindowRgn(hMainWindow, NULL, true);
					BlendWindow(hMainWindow, 0, 100);
					acClock->SetBackground(clBackground);
					CheckDlgButton(hwndDlg, IDC_CLASSIC, true);
					CheckDlgButton(hwndDlg,IDC_SKIN , false);
					break;
				}
				// editboxtext setzen und skinfile zum parsen an acClock ¸bergeben
				SetEditText(GetDlgItem(hwndDlg, IDC_SKINFILE), ofn.lpstrFile);
				/*acClock->ConstructImagePath(szFile, "hallo.bmp");
 				MessageBox(hwndDlg, szFile, "Oink", 0);*/
				if(!acClock->SetSkinFile(szFile))
				{
					MessageBox(hwndDlg, "An Error has occured! Error in Skin File!", "Error", MB_ICONEXCLAMATION);
					caAppearance = Classic;
					SetWindowRgn(hMainWindow, NULL, true);
					BlendWindow(hMainWindow, 0, 100);
					acClock->SetBackground(clBackground);
					CheckDlgButton(hwndDlg, IDC_CLASSIC, true);
					CheckDlgButton(hwndDlg,IDC_SKIN , false);
					break;
				}
				csSize = Size3;
				ResizeWindow();
				CheckDlgButton(hwndDlg, IDC_SIZE3, true);
				SetWindowRgn(hMainWindow, acClock->GetSkinRegion(), true);
				break;

			}
			return true;
			break;
		case WM_INITDIALOG:
			ShowWindow(hMainWindow, SW_SHOW);
			icce.dwICC = ICC_BAR_CLASSES;
			InitCommonControlsEx(&icce);

			switch(cpPosition)
			{
			case UpperLeft:
				CheckDlgButton(hwndDlg, IDC_UPPERLEFT, BST_CHECKED);
				break;
			case UpperRight:
				CheckDlgButton(hwndDlg, IDC_UPPERRIGHT, BST_CHECKED);
				break;
			case LowerLeft:
				CheckDlgButton(hwndDlg, IDC_LOWERLEFT, BST_CHECKED);
				break;
			case LowerRight:
				CheckDlgButton(hwndDlg, IDC_LOWERRIGHT, BST_CHECKED);
				break;
			case Middle:
				CheckDlgButton(hwndDlg, IDC_MIDDLE, BST_CHECKED);
				break;
			}
			switch(csSize)
			{
			case Size1:
				CheckDlgButton(hwndDlg, IDC_SIZE1, BST_CHECKED);
				break;
			case Size2:
				CheckDlgButton(hwndDlg, IDC_SIZE2, BST_CHECKED);
				break;
			case Size3:
				CheckDlgButton(hwndDlg, IDC_SIZE3, BST_CHECKED);
				break;
			}
			switch(caAppearance)
			{
			case Classic:
				CheckDlgButton(hwndDlg, IDC_CLASSIC, BST_CHECKED);
				break;
			case Round:
				CheckDlgButton(hwndDlg, IDC_ROUND, BST_CHECKED);
				break;
			case Transparent:
			case TransparentRound:
				CheckDlgButton(hwndDlg, IDC_TRANS, BST_CHECKED);
				break;
			case Clocked:
				CheckDlgButton(hwndDlg, IDC_SHAPE, BST_CHECKED);
				break;
			case Skin:
				CheckDlgButton(hwndDlg, IDC_SKIN, true);
				break;
			}
			TrackBar_SetMinRange(GetDlgItem(hwndDlg,IDC_VISIBILITYSLIDER), 1000);
			TrackBar_SetMaxRange(GetDlgItem(hwndDlg,IDC_VISIBILITYSLIDER), 10000);
			TrackBar_SetPos(GetDlgItem(hwndDlg,IDC_VISIBILITYSLIDER), uShowTimerTime);
			wsprintf(str, "%d ms", uShowTimerTime);
			SetDlgItemText(hwndDlg, IDC_VISIBILITYTIME, str);
			TrackBar_SetMinRange(GetDlgItem(hwndDlg,IDC_TRANSPARENCYSLIDER), 0);
			TrackBar_SetMaxRange(GetDlgItem(hwndDlg,IDC_TRANSPARENCYSLIDER), 100);
			TrackBar_SetPos(GetDlgItem(hwndDlg,IDC_TRANSPARENCYSLIDER), iBlendFactor);
			wsprintf(str, "%d%%", 100-iBlendFactor);
			SetDlgItemText(hwndDlg, IDC_TRANSPARENCYVALUE, str);
			if(bAutoHide)
				CheckDlgButton(hwndDlg, IDC_AUTOHIDE, BST_CHECKED);
			if(bShowDate)
				CheckDlgButton(hwndDlg, IDC_SHOWDATE, BST_CHECKED);
			return 0;
			break;
			case WM_PAINT:
				HDC hdc = NULL;
				PAINTSTRUCT ps;
				hdc = BeginPaint(hwndDlg, &ps);

				DrawScreen(1, 5, hdc);

				EndPaint(hwndDlg,&ps);
				return 0;  
	}
	return 0;
};

//===== The Main Window Procedure =============================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD prevState;
	switch(uMsg)
	{
		case WM_TRAYICON:
			TrayIconMessage(hwnd, uMsg, wParam, lParam);
			break;
		case WM_CLOSE:
			CleanUp();
			break;
		case WM_TIMER:
			ShowWindow(hwnd, SW_HIDE);
			bVisible = false;
			KillTimer(hwnd, 0);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_COMMAND: // tray menu commands
			switch(LOWORD(wParam))
			{
			case ID_TRAY_EXIT:
				SaveProgramSettings();
				SaveAlarms();
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			case ID_TRAY_ALARMS:
				bChangeSettings = true;
				DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_ALARM), hMainWindow, AlarmsProc, 0);
				bChangeSettings = false;
				break;
			case ID_TRAY_INFO:
				bChangeSettings = true;
				DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_ABOUT), hMainWindow, AboutProc, 0);							
				bChangeSettings = false;
				break;
			case ID_TRAY_SETTINGS:
				bChangeSettings = true;
				DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_CONFIGURATION), hMainWindow, SettingsProc, 0);
				break;
			}
			break;
			case WM_LBUTTONDOWN:
				pOldPos.x = GET_X_LPARAM(lParam);
				pOldPos.y = GET_Y_LPARAM(lParam);
				ClientToScreen(hwnd, &pOldPos); // f¸r absolute differenzen
				SetCapture(hwnd);
				bMouseDown = true;
				break;
			case WM_LBUTTONUP:
				bMouseDown = false;
				ReleaseCapture();
				break;
			case WM_MOUSEMOVE:
				if(bMouseDown)
				{
					cpPosition = UserDefined;
					POINT pos;
					pos.x = GET_X_LPARAM(lParam);
					pos.y = GET_Y_LPARAM(lParam);
					ClientToScreen(hwnd, &pos); // f¸r absolute differenzen
					int xdiff, ydiff;
					xdiff = pos.x-pOldPos.x;
					ydiff = pos.y-pOldPos.y;
					if((xdiff!= 0) || (ydiff !=0))
					{
						RECT r;
						GetWindowRect(hwnd, &r);
						r.left += xdiff;
						r.top += ydiff;
						SetWindowPos(hwnd, HWND_TOP, r.left, r.top, 0, 0, SWP_NOSIZE);
						SetWindowPos(acClock->GetHandle(),HWND_TOP, 0,0,0,0,SWP_NOSIZE);
						cpUserX = r.left;
						cpUserY = r.top;
						pOldPos.x = pos.x;
						pOldPos.y = pos.y;
					}
				}
			return 0;

		default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	prevState = 0;
	return 0;
};


//===== PROGRAM ENTRY POINT =============================================================
// Initialises Application window and loads settings if available
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;

	hAppInstance = hInstance;

	if(!LoadProgramData())
	{
		MessageBox(NULL, szFLoadRes, szError, MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	LoadProgramSettings();
	LoadAlarms();

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLOCK));
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szClassName;
	wc.hIconSm		 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLOCK));

	if(!RegisterClassEx(&wc)) // register window class
	{
		MessageBox(NULL, szFRegisterWc, szError, MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// create program window without title bar, dimensions can be used for skinned window
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,szClassName,szAppTitle,WS_POPUP,cpUserX, cpUserY, csSize, csSize ,NULL, NULL, hInstance, NULL);

	if(hwnd == NULL)
	{
		MessageBox(NULL, szFCreateWnd, szError, MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hMainWindow = hwnd;

	PosClockWindow();
	AddTrayIcon(hwnd, uTrayIconId, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLOCK))); // register tray icon

	acClock = new AnalogueClock;

	if(acClock == NULL)
	{
		MessageBox(hwnd, szFCreateClock, szError, MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	acClock->SetParentWindow(hwnd);
	acClock->SetHourHandSize(40);
	acClock->SetMinuteHandSize(70);
	acClock->SetForeground(clForeground);
	acClock->SetBackground(clBackground);
	acClock->SetSpecial(clSpecial);

	PSYSTEMTIME st;
	st = new SYSTEMTIME;
	GetLocalTime(st);
	acClock->SetTime(st);

	RECT r;
	r.left = 0; r.top = 0;
	r.right =  r.bottom = csSize;
	acClock->SetDimensions(r);

	acClock->SetCallback(AlarmCallback);

	acClock->Create();


	if(bShowDate)
		acClock->SetShowDate(true);

	if(caAppearance == Transparent)
	{
		BlendWindow(hwnd, RGB(255,255,255), 100-iBlendFactor);
	}
	else if (caAppearance == Round)
	{
		HRGN r;
		r = CreateEllipticRgn(0,0,csSize, csSize);
		SetWindowRgn(hMainWindow, r, true);
	}
	else if (caAppearance == TransparentRound)
	{
		HRGN r;
		r = CreateEllipticRgn(0,0,csSize, csSize);
		SetWindowRgn(hMainWindow, r, true);
		BlendWindow(hwnd, 0, 100-iBlendFactor);
	}
	else if(caAppearance == Clocked)
	{
		if(acClock->GetForeground() != RGB(255,255,255))
		{
			acClock->SetBackground(RGB(255,255,255));
			BlendWindow(hMainWindow, RGB(255,255,255), 100-iBlendFactor, true);
		}
		else
		{
			acClock->SetBackground(RGB(0,0,0));
			BlendWindow(hMainWindow, RGB(0,0,0), 100-iBlendFactor, true);
		}
	}

	if(bVisible)
	{
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
};

//-------------------------------------------------------------------