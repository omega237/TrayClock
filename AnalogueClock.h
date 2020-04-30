/*

AnalogueClock is a simple class to display an analogue clock

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


// für die anglearcfunktion muß ich wissen, welches der kürzest mögliche abstand zwischen randx und mittelpunkt x
// und randy und mittelpunkt  ist
// wenn ich keinen skin verwende, ist ja alles symmetrisch und ich kann einfach die ausmaße durch 2 teilen, funzt bei skins, wo
// die befestigung der zeiger nicht unbedingt in der mitte der uhr sein muß, nicht

// um die skinned version zu programmieren, klasse von ANALOGUECLOCK ableiten und die drawfunktionen überschreiben

#ifndef __ANALOGUECLOCK_H__
#define __ANALOGUECLOCK_H__

#include <windows.h>
#include <mmsystem.h>
#include <math.h>

// Dateiversion 1.0

typedef signed long DayTime;

const double	 M_PI 	=	3.14159265358979323846f;
const double	 to_rad =	0.017453292519943295769236907684886f;

#define SECONDSPERMINUTE 	60
#define MINUTESPERHOUR 		60
#define HOURSPERDAY 		24
#define SECONDSPERDAY 		(SECONDSPERMINUTE*MINUTESPERHOUR*HOURSPERDAY)

#define ALTER_MINUTE 		SECONDSPERMINUTE
#define ALTER_HOUR 			(SECONDSPERMINUTE*MINUTESPERHOUR)
#define ALTER_QUARTERDAY 	(6*ALTER_HOUR)
#define ALTER_HALFDAY 		(12*ALTER_HOUR)

class AnalogueClock
{
	public:
	AnalogueClock(); // initialisierung mit standardwerten
	~AnalogueClock();
        //virtual ~AnalogueClock();
	
	void SetParentWindow(HWND hwndParent);
	HWND GetParentWindow();
	
	HWND GetHandle();

	virtual void SetDimensions(RECT rcDimensions);
	virtual void GetDimensions(RECT* pRect);
	
	void SetTime(DayTime iTime);
	void SetTime(PSYSTEMTIME tTime);
	void AlterTime(DayTime iAlter); // relativen Wert in s angeben, um den die Zeit geändert wird
	DayTime GetTime();
	void IntTimeToSystemTime(DayTime iTime, PSYSTEMTIME tTime);
	DayTime SystemtimeToIntTime(SYSTEMTIME st);

	int GetHour(DayTime iTime);
	int GetMinute(DayTime iTime);
	int GetSecond(DayTime iTime);
	
	// ableiten und redefinieren um den look der uhr zu ändern
	virtual void DrawClockface();
	virtual void DrawHands();	

	virtual void DoSecond();
	
	COLORREF GetBackground();
	COLORREF GetForeground();
	COLORREF GetSpecial();
	void SetBackground(COLORREF clBackground);
	void SetForeground(COLORREF clForeground);
	void SetSpecial(COLORREF clSpecial);
	
	void SetHourHandSize(int iSize); // maximal 100% für jeden Wert
	void SetMinuteHandSize(int iSize);
	void SetSecondHandSize(int iSize);
	int GetHourHandSize();
	int GetMinuteHandSize();
	int GetSecondHandSize();
	
	void SetShowHourLabels(bool bShow);
	void SetShowMinuteLabels(bool bShow);
	void SetShowSecondHand(bool bShow);
	bool GetShowHourLabels();
	bool GetShowMinuteLabels();
	bool GetShowSecondHand();

	bool SetSkinFile(char *szSkinFile); // erst normal erstellen, dann den skin setzen!!!!
	HRGN GetSkinRegion();
	void ConstructImagePath(char *szPath, char *szFile);
	void UnloadSkin();
	
	bool Create(); // aufrufen wenn die dimensions und das parentfenster gesetzt worden sind
	
	void Show();
	void Hide();
	bool IsVisible();
	
	static LRESULT CALLBACK AnalogueClockProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
	virtual LRESULT On_WMPAINT(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT On_WMTIMER(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT On_WMLBUTTONDOWN(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT On_WMLBUTTONUP(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT On_WMMOUSEMOVE(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT On_WMNCHITTEST(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT On_WMNCMOUSEMOVE(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT On_WMERASEBKGND(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void SetCallback(void (*cb) (AnalogueClock*, DayTime));

	void SetShowDate(bool bShow);
	
	private:

	void CalculatePoints();

	bool ShowHourLabels;
	bool ShowMinuteLabels;
	bool ShowSecondHand;
	bool OwnTimer;
	
	COLORREF Foreground;
	COLORREF Background;
	COLORREF Special;
	
	HDC ClockDc, FaceDc;
	HBITMAP ClockBitmap, FaceBitmap;
	HWND ParentWindow;
	HWND ClockWindow;
	RECT Dimensions;
	int Diameter; // durchmesser = seitenlaenge Ursprung immer 0,0
	
	int HourHandSize; // größenangaben in % des radius des ziffernblattes
	int MinuteHandSize;
	int SecondHandSize;

	bool ShowDate;
	bool bTimealtered;;
	
	DayTime Time;
	
	bool Visible;

	MMRESULT Timer;
	int Day, Month;
	

	POINT pMiddle;
	POINT ptClockface[60];

	// skin settings and data
	bool skinned;
	HDC skinDc;
	HBITMAP skinBitmap;
	POINT skinMiddle;
	COLORREF skinTransCol;
	COLORREF skinSpecial;
	COLORREF skinOthers;
	int skinTransPixel;
	char skinFile[MAX_PATH];
	int skinHourHand;
	int skinMinuteHand;
	int skinSecondHand;
	bool skinIrregularShape;
	RECT skinBounds;

	int SkinGetMaxLength();

	// eigene Callbackfunktion speichern, die immer handle und aktuelle zeit übergibt und jedesmal aufruft wenn dosecond
	void (*ClockCallback) (AnalogueClock*, DayTime);
};


/* ermittelt die maximale länge, die ein zeiger haben kann */
int AnalogueClock::SkinGetMaxLength()
{
	int dx, dx1, dy, dy1, sx, sy; // m-r, m-l, t-m, m-b
	dx = skinBounds.right-pMiddle.x;
	dx1 = pMiddle.x-skinBounds.left;
	dy = pMiddle.y-skinBounds.top;
	dy1 = skinBounds.bottom-pMiddle.y;

	if(dx<=dx1)
		sx = dx;
	else sx = dx1;
	if(dy<=dy1)
		sy = dy;
	else sy = dy1;
	if(sx<=sy)
		return sx;
	return sy;
};

/* constructs the fully qualified image path by adding szFile to szPath where adding begins at the position of the last
backslash.
szPath in/out
*/
void AnalogueClock::ConstructImagePath(char *szPath, char *szFile)
{
	int len = strlen(szPath);
	while (len>0)
	{
		if(szPath[len] == '\\')
			break;
		len--;
	}
	szPath = &szPath[len+1]; // let szPath point to char after last backslash

	// copy bzw. append filename
	while(*szFile != '\0')
	{
		*szPath = *szFile;
		szPath++;
		szFile++;
	};
	// terminate path string
	*(szPath) = '\0';
}

// ===== setzt einen skin ===================================

// dran denken noch eine farbe für die zeiger mit in die datei zu nehmen
// sekundenzeiger und normale zeiger
bool AnalogueClock::SetSkinFile(char *szSkinFile)
{
	UnloadSkin(); 

	// read stuff from file
	char ImageFile[MAX_PATH]; // location of image file
	char CenterX[20];
	char CenterY[20];
	char MinuteHand[20];
	char SecondHand[20];
	char HourHand[20];
	char TransCol[20];
	char IrregularShape[20];
	char top[20], left[20], bottom[20], right[20];
	GetPrivateProfileString("SkinInfo", "ImageFile", "not found", ImageFile, sizeof(ImageFile), szSkinFile);
	GetPrivateProfileString("SkinInfo", "CenterX", "not found", CenterX, sizeof(CenterX), szSkinFile);
	GetPrivateProfileString("SkinInfo", "CenterY", "not found", CenterY, sizeof(CenterY), szSkinFile);
	GetPrivateProfileString("SkinInfo", "MinuteHand", "not found", MinuteHand, sizeof(MinuteHand), szSkinFile);
	GetPrivateProfileString("SkinInfo", "HourHand", "not found", HourHand, sizeof(HourHand), szSkinFile);
	GetPrivateProfileString("SkinInfo", "SecondHand", "not found", SecondHand, sizeof(SecondHand), szSkinFile);
	GetPrivateProfileString("SkinInfo", "TransCol", "not found", TransCol, sizeof(TransCol), szSkinFile);
	GetPrivateProfileString("SkinInfo", "IrregularShape", "not found", IrregularShape, sizeof(IrregularShape), szSkinFile);
	GetPrivateProfileString("SkinInfo", "top", "not found", top, sizeof(top), szSkinFile);
	GetPrivateProfileString("SkinInfo", "left", "not found", left, sizeof(left), szSkinFile);
	GetPrivateProfileString("SkinInfo", "bottom", "not found", bottom, sizeof(bottom), szSkinFile);
	GetPrivateProfileString("SkinInfo", "right", "not found", right, sizeof(right), szSkinFile);
	// check if every key was found. every key is required in a properly made skin file
	bool everykeythere = true;
	everykeythere &= strcmp(ImageFile, "not found")?true:false;
	everykeythere &= strcmp(CenterX, "not found")?true:false;
	everykeythere &= strcmp(CenterY, "not found")?true:false;
	everykeythere &= strcmp(MinuteHand, "not found")?true:false;
	everykeythere &= strcmp(HourHand, "not found")?true:false;
	everykeythere &= strcmp(SecondHand, "not found")?true:false;
	everykeythere &= strcmp(TransCol, "not found")?true:false;
	everykeythere &= strcmp(IrregularShape, "not found")?true:false;
	everykeythere &= strcmp(top, "not found")?true:false;
	everykeythere &= strcmp(left, "not found")?true:false;
	everykeythere &= strcmp(bottom, "not found")?true:false;
	everykeythere &= strcmp(right, "not found")?true:false;

	// everything found, proceed
	skinBounds.top = atoi(top);
	skinBounds.left = atoi(left);
	skinBounds.right = atoi(right);
	skinBounds.bottom = atoi(bottom);
	if(!everykeythere)
		return false;
	strcpy(skinFile, ImageFile);
	skinMiddle.x = atoi(CenterX);
	skinMiddle.y = atoi(CenterY);
	skinHourHand = atoi(HourHand);
	skinMinuteHand = atoi(MinuteHand);
	skinSecondHand = atoi(SecondHand);
	skinTransPixel = atoi(TransCol);
	skinIrregularShape = atoi(IrregularShape)?true:false;

	// try to load bitmap and init transcol value
	// first construct path to img

	ConstructImagePath(szSkinFile, skinFile);

	if((skinBitmap = (HBITMAP)LoadImage(NULL, skinFile, IMAGE_BITMAP, 0,0, LR_LOADFROMFILE))==NULL)
		return false; // failed loading image

	// check if image is bigger than 300x300, if so delete image and return false
	BITMAP bmp;
	ZeroMemory(&bmp, sizeof(BITMAP));
	if(GetObject(skinBitmap, sizeof(BITMAP), &bmp)==0)
		return false;
	if(bmp.bmHeight*bmp.bmWidth != 90000) // bad dimensions
	{
		DeleteObject(skinBitmap);
		return false;
	}
	skinDc = CreateCompatibleDC(NULL);
	SelectObject(skinDc, skinBitmap);
	skinTransCol = GetPixel(skinDc, skinTransPixel%bmp.bmWidth, (skinTransPixel/bmp.bmWidth));

	skinned = true; // set skinned to true so the clock knows from which dc to take the background

	RECT r;
	r.left = r.top = 0;
	r.right = r.bottom= 300;
	SetDimensions(r);
	pMiddle.x = skinMiddle.x;
	pMiddle.y = skinMiddle.y;

	return true;
};

// erstellt eine region, damit der skin passend im fenster angezeigt wird
/*
zuerst wird eine leere region erstellt. dann wird der erste transparente pixel herausgesucht und gemerkt, dann der erste nicht transparente
und auch gemerkt. es wird eine region in diesen ausmaßen erstellt und zu der leeren addiert. dies wird durchgeführt, 
bis das ganze bild gescannt wurde. gescannt wird von x skinBounds.left-skinBounds.right y skinBounds.top-skinBounds.bottom.

wenn die region mit der region, die das bounding rect abbildet übereinstimmt, wird die bounding rect region zurückgegeben,
ansonsten die erstellte region.
*/
HRGN AnalogueClock::GetSkinRegion()
{
	HRGN tmp = CreateRectRgn(0,0,0,0);

	int iX = skinBounds.left;
	int iRet = 0;
	for (int iY = skinBounds.top; iY < skinBounds.bottom; iY++)
	{
	  do
	  {
		//skip over transparent pixels at start of lines.
		while (iX < skinBounds.right && GetPixel(skinDc, iX, iY)
				  == skinTransCol) iX++;
		//remember this pixel
		int iLeftX = iX;
		//now find first non-transparent pixel
		while (iX < skinBounds.right && GetPixel(skinDc, iX, iY) != skinTransCol) 
			++iX;
		//create a temp region on this info
		HRGN hRgnTemp = CreateRectRgn(iLeftX, iY, iX, iY+1);
		//combine into main region
		iRet = CombineRgn(tmp, tmp, hRgnTemp, RGN_OR);
		if(iRet == ERROR)
		{
		  return NULL;
		}
		//delete the temp region for next pass
		DeleteObject(hRgnTemp);
	  }while(iX < skinBounds.right);
	  iX = 0;
	}
	return tmp;
};

void AnalogueClock::UnloadSkin()
{
	if(skinned)
	{
		RECT r;
		GetDimensions(&r);
		pMiddle.x = r.right/2;
		pMiddle.y = r.bottom/2;
		skinned = false;
		DeleteObject(skinBitmap);
		DeleteObject(skinDc);
		DrawClockface();
	}
};

//-----------------------------------------------------------
AnalogueClock* GetAC(HWND hWnd)
{
	return (AnalogueClock*)GetWindowLong(hWnd,0);
};

void SetAC(HWND hWnd, AnalogueClock *ac)
{
;	SetWindowLong(hWnd, 0, (LONG)ac);
}
//-------------------------------------------------------------
DayTime AnalogueClock::SystemtimeToIntTime(SYSTEMTIME st)
{
	return (st.wHour*3600)+(st.wMinute*60)+st.wSecond;
};


void AnalogueClock::SetCallback(void (*cb) (AnalogueClock*, DayTime))
{ 
	ClockCallback = cb;
};


void AnalogueClock::SetShowDate(bool bShow)
{ 
	ShowDate = bShow;
	DrawClockface();
};

void CALLBACK AnalogueClock::TimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	AnalogueClock *ac = GetAC((HWND)dwUser);

	ac->DoSecond();

	return;
}

int AnalogueClock::GetHour(DayTime iTime)
{
	return (int)(iTime/(SECONDSPERMINUTE*MINUTESPERHOUR));
};

int AnalogueClock::GetMinute(DayTime iTime)
{
	return (int)((iTime/MINUTESPERHOUR)%MINUTESPERHOUR);
};

int AnalogueClock::GetSecond(DayTime iTime)
{
	return (int)(iTime%SECONDSPERMINUTE);
};

void AnalogueClock::DoSecond() // fenster neu zeichnen
{
	Time = ((Time++)%SECONDSPERDAY);

	// datum checken, damit wenn die uhr über nacht an ist, das datum auch aktualisiert wird
	SYSTEMTIME st;
	GetSystemTime(&st);
	Day = st.wDay;
	Month = st.wMonth;

	bTimealtered = true;

	RECT r;
	GetClientRect(ClockWindow, &r);
	InvalidateRect(ClockWindow, &r, true);
	
	(*ClockCallback)(this, Time);
};

void AnalogueClock::SetTime(DayTime iTime)
{
	if(iTime <0)
	{
		Time = 0;
		return;
	}
	Time = iTime%SECONDSPERDAY;
	return;
};

void AnalogueClock::AlterTime(DayTime iAlter)
{
	Time = ((Time+iAlter)%SECONDSPERDAY);
	RECT r; r.left = r.top = 0;
	r.right = r.bottom = Diameter;
	InvalidateRect(ClockWindow, &r, true);
};

void AnalogueClock::SetTime(PSYSTEMTIME tTime)
{
	Time = (tTime->wHour*3600)+(tTime->wMinute*60)+tTime->wSecond;
	Day = tTime->wDay;
	Month = tTime->wMonth;
};

HWND AnalogueClock::GetHandle()
{
	return ClockWindow;
}

bool AnalogueClock::Create()
{
	WNDCLASSEX wc;
	
	wc.cbSize 			= sizeof(wc);
	wc.lpszClassName 	= "AnalogueClock";
	wc.lpfnWndProc		= AnalogueClockProc;
	wc.hInstance 		= GetModuleHandle(NULL) ; // Handle der .EXE
	wc.hCursor 			= NULL; // wir setzen den Cursor selbst
	wc.hbrBackground 	= NULL; // wir zeichnen alles selbst
	wc.style 			= CS_DBLCLKS | CS_OWNDC; // sendet Doppelklicknachrichten
	wc.cbClsExtra 		= 0;
	wc.cbWndExtra 		= sizeof(AnalogueClock*);
	wc.hIcon 			= 0;
	wc.lpszMenuName 	= 0;
	wc.hIconSm 			= 0;
	
	if(!RegisterClassEx(&wc)) return false;

	
	HWND hWnd;
	
	hWnd =  CreateWindowEx(0,  "AnalogueClock",
			      "", // kein Fenstertitel weil keine Caption
			      WS_VISIBLE | WS_CHILD, // sichtbar, untergeordnet
			      Dimensions.left, // x
			      Dimensions.top, // y
			      Dimensions.right, // Breite
			      Dimensions.bottom, // Höhe
			      ParentWindow,
			      0, // kein Menü
			      GetModuleHandle(NULL),
			      NULL);
	if(hWnd == NULL) return false;
	ClockWindow = hWnd;

	SetAC(ClockWindow, this);
	
	MMRESULT res;
	res = timeSetEvent(1000, 250, TimerProc, (DWORD)hWnd, TIME_CALLBACK_FUNCTION | TIME_PERIODIC);
	if(res == NULL)
	{
		DestroyWindow(hWnd);
		return false;
	}
	Timer = res;
		 
	
	Diameter = Dimensions.right-Dimensions.left;
	
	ClockDc = /*GetDC(m_hWnd);*/  CreateCompatibleDC(NULL);
	ClockBitmap = CreateCompatibleBitmap(GetDC(ClockWindow),Diameter,Diameter);
	FaceDc = CreateCompatibleDC(NULL);
	FaceBitmap = CreateCompatibleBitmap(GetDC(ClockWindow), Diameter, Diameter);
	SelectObject(ClockDc,ClockBitmap);
	SelectObject(FaceDc, FaceBitmap);

	if(!skinned)
		CalculatePoints();

	DrawClockface();

	return true;
	
};

void AnalogueClock::SetParentWindow(HWND hwndParent)
{
	if(hwndParent == NULL) return;
	ParentWindow = hwndParent;
};

HWND AnalogueClock::GetParentWindow()
{
	return ParentWindow;
};

	
void AnalogueClock::SetDimensions(RECT rcDimensions)
{
	// kreisrunde form sicherstellen
	if(((rcDimensions.right-rcDimensions.left) != (rcDimensions.bottom-rcDimensions.top)))
		rcDimensions.bottom = rcDimensions.top+(rcDimensions.right-rcDimensions.left);
	if((rcDimensions.right%2)!=0) rcDimensions.right = rcDimensions.bottom = (rcDimensions.right+1); // ordentlichen mittelpunkt
	
	Dimensions = rcDimensions;

	Diameter = Dimensions.right-Dimensions.left;

	if(!skinned)
		CalculatePoints();

	if(ClockWindow!=NULL)
	{
		SetWindowPos(ClockWindow, HWND_TOP , 0,0, Diameter, Diameter, SWP_NOMOVE);
		DeleteObject(ClockBitmap);
		ClockBitmap = CreateCompatibleBitmap(GetDC(ClockWindow),Diameter,Diameter);
		SelectObject(ClockDc,ClockBitmap);
		DeleteObject(FaceBitmap);
		FaceBitmap = CreateCompatibleBitmap(GetDC(ClockWindow), Diameter, Diameter);
		SelectObject(FaceDc, FaceBitmap);
		DrawClockface();
		RECT r;
		GetWindowRect(ClockWindow, &r);
		InvalidateRect(ClockWindow, &r, true);
	}
};

void AnalogueClock::GetDimensions(RECT *pRect)
{
	if(pRect != NULL)
	{
		*pRect = Dimensions;
	}
};

AnalogueClock::AnalogueClock()
{
	RECT rdim;
	rdim.left = 0;
	rdim.top = 0;
	rdim.right = 100;
	rdim.bottom = 100;
	
	Dimensions = rdim;
	
	Background = RGB(95,95,95); // dunkles Grau
	Foreground = RGB(215,215,215); // helles Grau
	Special = RGB(255,128,64); // recht dunkles orange
	
	Time = 0;
	Day = 0; 
	Month = 0;
	
	OwnTimer = true;
	Visible = true;
	
	ShowHourLabels = true;
	ShowMinuteLabels = true;
	ShowSecondHand = true;
	ShowDate = false;

	HourHandSize = 40;
	MinuteHandSize = 70;
	SecondHandSize = 100;

	ClockWindow = NULL;

	ClockCallback = NULL;
	bTimealtered =  false;

	skinned = false;
};

void AnalogueClock::IntTimeToSystemTime(DayTime iTime, PSYSTEMTIME tTime)
{
	if(tTime == NULL) return;
	tTime->wHour = (unsigned short)GetHour(iTime);
	tTime->wMinute = (unsigned short)GetMinute(iTime);
	tTime->wSecond = (unsigned short)GetSecond(iTime);
};

LRESULT CALLBACK AnalogueClock::AnalogueClockProc(HWND hWnd, UINT uMsg,
					      WPARAM wParam, LPARAM lParam)
{
	AnalogueClock *ac = GetAC(hWnd);
	
	switch(uMsg)
	{
		case WM_PAINT: // neu zeichnen
		return ac->On_WMPAINT(hWnd, uMsg, wParam, lParam);
		case WM_TIMER:
		return ac->On_WMTIMER(hWnd, uMsg, wParam, lParam);
		case WM_LBUTTONDOWN:
		return ac->On_WMLBUTTONDOWN(hWnd, uMsg, wParam, lParam);
		case WM_LBUTTONUP:
		return ac->On_WMLBUTTONUP(hWnd, uMsg, wParam, lParam);
		case WM_MOUSEMOVE:
		return ac->On_WMMOUSEMOVE(hWnd, uMsg, wParam, lParam);
		case WM_NCHITTEST:
		return ac->On_WMNCHITTEST(hWnd, uMsg, wParam, lParam);
		case WM_NCMOUSEMOVE:
		return ac->On_WMNCMOUSEMOVE(hWnd, uMsg, wParam, lParam);
		case WM_ERASEBKGND:
		return ac->On_WMERASEBKGND(hWnd, uMsg, wParam, lParam);
		default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
};

LRESULT AnalogueClock::On_WMPAINT(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// hintergrund auf hintergrundfarbe setzen
	// alles zeichnen
	if(GetUpdateRect(hWnd, NULL, false))
	{

		HDC hdc;
		PAINTSTRUCT ps;

		RECT r;
		GetUpdateRect(hWnd, &r, false);

		/*POINT p;
		p.x = r.left;
		p.y = r.top;

		ScreenToClient(hWnd, &p);

		int dx = r.right-r.left;
		int dy = r.bottom-r.top;

		r.left = p.x;
		r.top = p.y;
		r.right = p.x + dx;
		r.bottom = p.y + dy;*/

		
		/*RECT r;
		r.left = r.top = 0;
		r.right = r.bottom = Diameter;*/
		
		
		/*DrawClockface();
		DrawHands();
		*/
		hdc = BeginPaint(hWnd, &ps);

		/*HBRUSH bg = CreateSolidBrush(Background);

		FillRect(ClockDc, &r, bg);

		DeleteObject(bg);*/
		
		//DrawClockface();
		if(bTimealtered) 
		{ 
			BitBlt(ClockDc, 0,0, Diameter, Diameter, FaceDc, 0, 0, SRCCOPY);
			DrawHands(); 
		}
			BitBlt(hdc, r.left, r.top, r.right, r.bottom, ClockDc, r.left, r.top, SRCCOPY);

		EndPaint(hWnd, &ps);
	}

	bTimealtered = false;
	return 0;
};

// nachrichten an das parentfenster weiterleiten
LRESULT AnalogueClock::On_WMNCMOUSEMOVE(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
};

LRESULT AnalogueClock::On_WMTIMER(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
};

LRESULT AnalogueClock::On_WMLBUTTONDOWN(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PostMessage(ParentWindow, WM_NCHITTEST, wParam, lParam);
	PostMessage(ParentWindow, uMsg, wParam, lParam);
	return 0;
};

LRESULT AnalogueClock::On_WMLBUTTONUP(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PostMessage(ParentWindow, WM_NCHITTEST, wParam, lParam);
	PostMessage(ParentWindow, uMsg, wParam, lParam);
	return 0;
};

LRESULT AnalogueClock::On_WMMOUSEMOVE(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PostMessage(ParentWindow, WM_NCHITTEST, wParam, lParam);
	PostMessage(ParentWindow, uMsg, wParam, lParam);
	return 0;
};

LRESULT AnalogueClock::On_WMNCHITTEST(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PostMessage(ParentWindow, uMsg, wParam, lParam);
	return HTCLIENT;
};

LRESULT AnalogueClock::On_WMERASEBKGND(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
};

AnalogueClock::~AnalogueClock()
{
	if(ClockWindow != NULL)
		DestroyWindow(ClockWindow);
	timeKillEvent(Timer);
};

void AnalogueClock::SetHourHandSize(int iSize)
{
	HourHandSize = ((iSize<0)?0:((iSize>99)?99:iSize));
};


void AnalogueClock::SetMinuteHandSize(int iSize)
{
	MinuteHandSize = ((iSize<0)?0:((iSize>100)?100:iSize));
};

void AnalogueClock::SetSecondHandSize(int iSize)
{
	SecondHandSize = ((iSize<0)?0:((iSize>100)?100:iSize));
};

bool AnalogueClock::IsVisible()
{
	return Visible;
};

void AnalogueClock::Show()
{
	if(ClockWindow!=NULL)
	ShowWindow(ClockWindow, SW_SHOW);
	Visible = true;
};

void AnalogueClock::Hide()
{
	if(ClockWindow!=NULL)
	ShowWindow(ClockWindow, SW_HIDE);
	Visible = false;
};

void AnalogueClock::SetBackground(COLORREF clBackground)
{
	Background = clBackground;
	DrawClockface();
};

void AnalogueClock::SetForeground(COLORREF clForeground)
{
	Foreground = clForeground;
};

void AnalogueClock::SetSpecial(COLORREF clSpecial)
{
	Special = clSpecial;
};

COLORREF AnalogueClock::GetBackground()
{
	return Background;
};

COLORREF AnalogueClock::GetForeground()
{
	return Foreground;
};

COLORREF AnalogueClock::GetSpecial()
{
	return Special;
};

void AnalogueClock::SetShowHourLabels(bool bShow)
{
	ShowHourLabels = bShow;
	DrawClockface();
};

void AnalogueClock::SetShowMinuteLabels(bool bShow)
{
	ShowMinuteLabels = bShow;
	DrawClockface();
};

void AnalogueClock::SetShowSecondHand(bool bShow)
{
	ShowSecondHand = bShow;
	DrawClockface();
};

bool AnalogueClock::GetShowHourLabels()
{
	return ShowHourLabels;
};

bool AnalogueClock::GetShowMinuteLabels()
{
	return ShowMinuteLabels;
};

bool AnalogueClock::GetShowSecondHand()
{
	return ShowSecondHand;
};

void AnalogueClock::DrawClockface() // das ziffernblatt
{
	HPEN pMin, pHour, pBkgnd;

	HBRUSH hBkgnd = CreateSolidBrush(Background);
	HFONT hDateFont;

	LOGFONT tf;
	tf.lfHeight = -8;
	tf.lfWidth = 0;
	tf.lfEscapement = 0;
	tf.lfOrientation = 0;
	tf.lfWeight = 400;
	tf.lfItalic = 0;
	tf.lfUnderline = 0;
	tf.lfStrikeOut = 0;
	tf.lfCharSet = 255;
	tf.lfOutPrecision = 1;
	tf.lfClipPrecision = 2;
	tf.lfQuality = 1;
	tf.lfPitchAndFamily = 49;
	strcpy(tf.lfFaceName,"Terminal");

	hDateFont = CreateFontIndirect(&tf);


	LOGBRUSH lb;
	lb.lbColor = Foreground;
	lb.lbHatch = 0;
	lb.lbStyle = BS_SOLID;

	pMin = ExtCreatePen(PS_GEOMETRIC| PS_SOLID | PS_ENDCAP_FLAT, 1,&lb, 0, NULL);
	pHour = ExtCreatePen(PS_GEOMETRIC| PS_SOLID | PS_ENDCAP_FLAT, (unsigned long)(Diameter/40.0f),&lb, 0, NULL);

	lb.lbColor = Background;
	
	pBkgnd = ExtCreatePen(PS_COSMETIC| PS_SOLID, 1,&lb, 0, NULL);
	if(!skinned)
	{
		SaveDC(FaceDc);

		SelectObject(FaceDc, hBkgnd);

		//Ellipse(FaceDc, -1,-1, Diameter, Diameter);
		Rectangle(FaceDc, -1, -1, Diameter+1, Diameter+1);

		SelectObject(FaceDc, pMin);

		// auf AngleArc umstellen
		if(ShowHourLabels)
		{
			for(int i=0; i<60; i+=1)
			{
				MoveToEx(FaceDc, pMiddle.x, pMiddle.y, NULL);
				LineTo(FaceDc, ptClockface[i].x, ptClockface[i].y);
			}
		}

		Ellipse(FaceDc, pMiddle.x-2, pMiddle.y-2, pMiddle.x+2, pMiddle.y+2);

		SelectObject(FaceDc, pBkgnd);
		Ellipse(FaceDc, 5, 5, Diameter-5, Diameter-5);
		SelectObject(FaceDc, pHour);

		// auf angleArc umstellen
		if(ShowMinuteLabels)
		{
			for(int j=0; j<60; j+=5)
			{
				MoveToEx(FaceDc, pMiddle.x, pMiddle.y, NULL);
				LineTo(FaceDc, ptClockface[j].x, ptClockface[j].y);
			}
		}

		SelectObject(FaceDc, pBkgnd);
		Ellipse(FaceDc, 10, 10, Diameter-10, Diameter-10);

		// date
		// rechteck in vordergrundfarbe, text in schöner schrift
		if(ShowDate)
		{
			SelectObject(FaceDc, hDateFont);
			char date[100];
			wsprintf(date, "%d.%d.", Day, Month);

			SIZE size;
			GetTextExtentPoint32(FaceDc, date, strlen(date), &size);

			SetTextColor(FaceDc, Foreground);
			//DrawText(ClockDc, date, -1, &textrect, DT_CALCRECT | DT_NOCLIP | DT_RIGHT);
			SetBkMode(FaceDc, TRANSPARENT);
			ExtTextOut(FaceDc, pMiddle.x-(int)(size.cx/2.0), pMiddle.y+(int)(Diameter/4.0), 0, NULL, date, strlen(date), NULL);
		}

	} // not skinned
	else
	{
		BitBlt(FaceDc, 0, 0, 300, 300, skinDc, 0, 0, SRCCOPY);
	} // skinned
	RestoreDC(FaceDc, -1);
	DeleteObject(pMin);
	DeleteObject(pHour);
	DeleteObject(hBkgnd);
	DeleteObject(pBkgnd);
	DeleteObject(hDateFont);
};

void AnalogueClock::DrawHands()
{
	LOGBRUSH lb;
	if(skinned)
		lb.lbColor = skinOthers;
	else
		lb.lbColor = Foreground;

	lb.lbHatch = 0;
	lb.lbStyle = BS_SOLID;

	HPEN hHandPen = ExtCreatePen(PS_GEOMETRIC| PS_SOLID | PS_ENDCAP_FLAT, (unsigned long)(Diameter/40.0f),&lb, 0, NULL);
	HPEN hSecPen = NULL;

	int min = GetMinute(Time);
	int hour = GetHour(Time);
	int sec = GetSecond(Time);

	SaveDC(ClockDc);
	SelectObject(ClockDc, hHandPen);


	MoveToEx(ClockDc, pMiddle.x, pMiddle.y, NULL);
	AngleArc(ClockDc, pMiddle.x, pMiddle.y, (unsigned long)((Diameter/2.0f)*MinuteHandSize/100.0f), (float)(450-(min*6)), 0);

	MoveToEx(ClockDc, pMiddle.x, pMiddle.y, NULL);
	AngleArc(ClockDc, pMiddle.x, pMiddle.y, (unsigned long)((Diameter/2.0f)*HourHandSize/100.0f), (float)(450-((hour%12)*30)-(min*0.5f)), 0);


	if(ShowSecondHand)
	{
		if(skinned)
			lb.lbColor = skinSpecial;
		else
			lb.lbColor = Special;
		hSecPen = ExtCreatePen(PS_GEOMETRIC| PS_SOLID | PS_ENDCAP_FLAT, 1,&lb, 0, NULL);
		SelectObject(ClockDc, hSecPen);
		MoveToEx(ClockDc, pMiddle.x, pMiddle.y, NULL);
		//AngleArc(ClockDc, pMiddle.x, pMiddle.y, ,,0)
		LineTo(ClockDc, ptClockface[sec].x, ptClockface[sec].y);
	}


	RestoreDC(ClockDc, -1);
	DeleteObject(hSecPen);
	DeleteObject(hHandPen);
};

DayTime AnalogueClock::GetTime()
{
	return Time;
};

int AnalogueClock::GetSecondHandSize()
{
	return SecondHandSize;
};

int AnalogueClock::GetMinuteHandSize()
{
	return MinuteHandSize;
};

int AnalogueClock::GetHourHandSize()
{
	return HourHandSize;
};

// calculates label points
// wird von create und setdimensions aufgerufen, würde entfallen, wenn die labels mit anglearc gezeichnet würden
void AnalogueClock::CalculatePoints()
{
	double r = Diameter/2.0f;

	pMiddle.x = pMiddle.y = (long) r;

	double angle = 0.0f, off = 0.0f;

	for(int i=0; i<60; i++)
	{
		angle = ((270.0f+(i*6.0f))*to_rad);
		off = (r + (r*cos(angle)));
		ptClockface[i].x =(long) ((i>29)?floor(off):ceil(off));
		off = (r + (r*sin(angle)));
		ptClockface[i].y =(long) ((i>29)?floor(off):ceil(off));
	}
};


#endif //__ANALOGUECLOCK_H__