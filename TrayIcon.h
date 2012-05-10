//
// TrayIcon.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

/*
	How to:
	
	1. In MainFrame class, inside BEGIN_MSG_MAP_EX(MainFrame)
		add MESSAGE_HANDLER_EX(WM_TRAYICON, OnTrayIcon).
	
	2. Add also MESSAGE_HANDLER_EX(WM_COMMAND, OnTrayCommand),
		this handles tray context menu commands, if you also process
		WM_COMMAND message then forward unprocessed messages to 
		OnTrayCommand.
*/

#pragma once

#define APP_NAME L"5 Minute Break" // This name used when adding to registry startup.
#define TRAY_TIMER 1
#define DEFAULT_TIMER 60 // in minutes

#include <ShellAPI.h>
#include "RunAtStartup.h"

#include "HelpFrame.h"
#include "AlarmFrame.h"

#include <math.h>
#include <time.h>

#include "WinVersion.h"
#include "Registry.h"

#include "Language.h"

// This enumeration must be global, as TRAYMENU_EXIT is used in WinMain().
enum
{
	WM_FIRST = WM_APP,
	WM_TRAYICON,
	TRAYMENU_EXIT,
	TRAYMENU_STARTUP,
	TRAYMENU_HELP,
	TRAYMENU_TIMER, // "Alarm is off" or "Alarm in 20 minutes"
	TRAYMENU_5_MINUTES,
	TRAYMENU_15_MINUTES,
	TRAYMENU_30_MINUTES,
	TRAYMENU_45_MINUTES,
	TRAYMENU_1_HOUR,
	TRAYMENU_2_HOURS,
	// -- language menu
	LANGMENU_ENGLISH,
	LANGMENU_POLISH,
	// -- subtimer menu
	DEFAULTTIMER_2_HOURS,
	DEFAULTTIMER_1_HOUR,
	DEFAULTTIMER_45_MINUTES,
	DEFAULTTIMER_30_MINUTES,
	DEFAULTTIMER_15_MINUTES
};

template<class T, bool t_bHasSip = true>
class TrayIcon
{
public:
	
	T* self;
	NOTIFYICONDATAW notifyicon; // this must be wide, as we're hardcoding NOTIFYICONDATAW_V2_SIZE, as there is a "guidItem" bug in ShellAPI.h.
	HelpFrame<TrayIcon>* helpframe; // this is cleaned up in HelpFrame::OnFinalMessage.
	AlarmFrame<TrayIcon>* alarmframe; // this is cleaned up in AlarmFrame::OnFinalMessage.
	
	int alarmtime; // in seconds
	unsigned long alarmtime_unix; // unix timestamp of when the alarm was set.

	wchar_t alarmstring[100]; // call GetAlarmString() to get current info.
	bool break5balloon; // whether we already displayed "In 5 minutes there will be a break" balloon.

	int cxscreen;
	int cyscreen;	

	TrayIcon()
	{
		self = static_cast<T*>(this);
		ZeroMemory(&notifyicon, sizeof notifyicon);
		helpframe = 0;
		alarmframe = 0;
		alarmtime = 0;
		alarmtime_unix = 0;
		break5balloon = false;
	}

	int GetMinutes()
	{
		if (alarmtime <= 0) return 0; // to be sure.
		return (int)ceil(alarmtime / 60.0f);
	}

	void GetPolishMinutes(int minutes, wchar_t* string, int stringSize)
	{
		int modulo;
		modulo = minutes % 10;
		
		if (2 == modulo || 3 == modulo || 4 == modulo) {
			wcsncpy_s(string, stringSize, L"minuty", _TRUNCATE); 
		} else {
			wcsncpy_s(string, stringSize, L"minut", _TRUNCATE);
		}
	}

	wchar_t* GetAlarmString()
	{
		if (alarmtime > 0) {
			int minutes = GetMinutes();
			if (minutes == 1) {
				if (LANG_POLISH == GetLanguage()) {
					swprintf_s(alarmstring, _countof(alarmstring), L"Alarm za %i minutê", minutes);
				} else {
					swprintf_s(alarmstring, _countof(alarmstring), L"Alarm in %i minute", minutes);
				}
			} else {
				if (LANG_POLISH == GetLanguage()) {
					wchar_t polishMinutes[10];
					GetPolishMinutes(minutes, polishMinutes, _countof(polishMinutes));
					swprintf_s(alarmstring, _countof(alarmstring), L"Alarm za %i %s", minutes, polishMinutes);
				} else {
					swprintf_s(alarmstring, _countof(alarmstring), L"Alarm in %i minutes", minutes);
				}
			}			
		} else {
			if (LANG_POLISH == GetLanguage()) {
				swprintf_s(alarmstring, _countof(alarmstring), L"%s", L"Alarm jest wy³¹czony");
			} else {
				swprintf_s(alarmstring, _countof(alarmstring), L"%s", L"Alarm is off");
			}
		}
		return alarmstring;
	}

	// Need it public, so AlarmFrame::OnFinalMessage can call it, when closing alarm window.
	void StartNewTimer(int minutes)
	{
		// Timer may already be running - in this case reset the timer.		

		alarmtime = minutes * 60;
		alarmtime_unix = GetUnixTimestamp();

		if (notifyicon.hIcon) {
			BOOL b = DestroyIcon(notifyicon.hIcon);
			ASSERT_EXIT(b, "DestroyIcon()");
		}
		notifyicon.hIcon = AtlLoadIconImage(IDI_RED, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
		UpdateAlarmTip(NIF_ICON); // updating tip sends NIM_MODIFY

		break5balloon = false;
	}

	void StopTimer()
	{
		alarmtime = 0;
		alarmtime_unix = 0;		

		if (notifyicon.hIcon) {
			BOOL b = DestroyIcon(notifyicon.hIcon);
			ASSERT_EXIT(b, "DestroyIcon()");
		}
		notifyicon.hIcon = AtlLoadIconImage(IDI_BLUE, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
		UpdateAlarmTip(NIF_ICON); // updating tip sends NIM_MODIFY
	}

	void UpdateAlarmTip(int moreflags=0)
	{
		notifyicon.uFlags = NIF_TIP | moreflags;
		GetAlarmString(); // acts as UpdateAlarmString(), now use "alarmstring" to access it.
		if (wcscmp(notifyicon.szTip, alarmstring) != 0) {
			wcsncpy_s(notifyicon.szTip, _countof(notifyicon.szTip), alarmstring, _TRUNCATE);
			BOOL b = Shell_NotifyIcon(NIM_MODIFY, &notifyicon);
			ASSERT_EXIT(b, "Shell_NotifyIcon(NIM_MODIFY)");
		} else {
			if (moreflags) {
				// icon has been modified
				BOOL b = Shell_NotifyIcon(NIM_MODIFY, &notifyicon);
				ASSERT_EXIT(b, "Shell_NotifyIcon(NIM_MODIFY)");
			}
		}
	}

	unsigned long GetUnixTimestamp()
	{
		return (unsigned long) time(NULL);
	}

	void UpdateAlarmTime()
	{
		if (alarmtime <= 0) {
			alarmtime = 0;
			alarmtime_unix = 0;
			return;
		}
		
		unsigned long currentunix = GetUnixTimestamp();
		int diffunix = currentunix - alarmtime_unix;
		alarmtime = alarmtime - diffunix;
		alarmtime_unix = currentunix;
		
		if (alarmtime < 0) {
			alarmtime = 0;
			alarmtime_unix = 0;
		}
	}

	void OneSecondTimer()
	{
		// Created with ::SetTimer().

		if (alarmframe) {
			CheckAlarmWindowSize();
		}

		if (alarmtime <= 0) { // to be sure.
			// there is no alarm set.
			return;
		}
		
		UpdateAlarmTime();
		UpdateAlarmTip();

		if (alarmtime <= 0) { // to be sure.
			break5balloon = true; // balloon is no more needed, to be sure.
			StopTimer();
			AlarmWindow();			
			return;
		}

		int minutes = GetMinutes();
		if (!break5balloon && minutes <= 5) {
			break5balloon = true;
			BreakIn5MinutesBalloon();
		}
	}

	void HelpWindow()
	{
		// IsWindow() doesn't have to be checked, as HelpFrame implemented OnFinalMessage,
		// and sets our pointer to 0 before "delete this".

		if (helpframe) {
			// check if minimized, bring to top, focus
			//
			if (helpframe->IsIconic()) {
				helpframe->ShowWindow(SW_RESTORE);
			} else {
				helpframe->SetActiveWindow();
			}
		} else {
			helpframe = new HelpFrame<TrayIcon>(this);
			
			RECT rc = {0, 0, helpframe->GetOptimalWidth(), helpframe->GetOptimalHeight()}; // Also change HelpFrame::OnGetMinMaxInfo()
			HWND helphwnd = helpframe->CreateEx(NULL, rc);
			ASSERT(helphwnd, "Failed to create help window.");
			
			if (helphwnd) {
				helpframe->SetWindowText(L"5 Minute Break");
				helpframe->CenterWindow();
				helpframe->SetWindowLong(GWL_STYLE, helpframe->GetWindowLong(GWL_STYLE) & ~WS_MAXIMIZEBOX); // disable maximize
				helpframe->ShowWindow(SW_SHOWNORMAL);				
			} else {
				helphwnd = 0;
			}
		}
	}

	void AlarmWindow()
	{
		if (alarmframe) {
			return;
		}

		alarmframe = new AlarmFrame<TrayIcon>(this);
			
		HWND hwnd = alarmframe->CreateEx();
		ASSERT(hwnd, "Failed to create alarm window.");
		
		if (hwnd) {
			alarmframe->SetWindowText(L"5 Minute Break");

			cxscreen = GetSystemMetrics(SM_CXSCREEN);
			cyscreen = GetSystemMetrics(SM_CYSCREEN);
			alarmframe->MoveWindow(0, 0, cxscreen, cyscreen, FALSE);

			if (ForceForegroundWindow1()) {
				alarmframe->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			} else {
				// If minimize<>restore trick didn't work, try attachiing keyboard input to the active window.
				if (ForceForegroundWindow2()) {
					alarmframe->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				}
			}
			
		} else {
			delete alarmframe;
			alarmframe = 0;
		}
	}

	bool ForceForegroundWindow1()
	{
		// The easiest way to force foreground window, let's try it first.
		alarmframe->ShowWindow(SW_MINIMIZE);
		alarmframe->ShowWindow(SW_RESTORE);
		return (GetForegroundWindow() == alarmframe->m_hWnd && GetActiveWindow() == alarmframe->m_hWnd);
	}

	bool ForceForegroundWindow2()
	{
		// Attaching keyboard input to current foreground window, only
		// window that has keyboard focus can be bring to foreground.

		DWORD thread1;
		DWORD thread2;
		HWND foregroundWnd;

		foregroundWnd = GetForegroundWindow();
		if (foregroundWnd == alarmframe->m_hWnd) {
			return true;
		}

		thread1 = GetWindowThreadProcessId(foregroundWnd, 0);
		thread2 = GetWindowThreadProcessId(alarmframe->m_hWnd, 0);

		if (thread1 != thread2) {
			
			AttachThreadInput(thread1, thread2, TRUE);
			
			SetForegroundWindow(alarmframe->m_hWnd); // this is not enough alone.
			alarmframe->BringWindowToTop(); // this is important.
			alarmframe->SetActiveWindow();
			alarmframe->SetFocus();

			if (GetForegroundWindow() != alarmframe->m_hWnd) {

				// Active window has locked SetForegroundWindow() by calling LockSetForegroundWindow().
				// We unlock it for a moment and set foreground window to ours.

				DWORD lockTimeout;		
				SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockTimeout, 0);
				SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, SPIF_SENDCHANGE);

				SetForegroundWindow(alarmframe->m_hWnd); // this is not enough alone.
				alarmframe->BringWindowToTop(); // this is important.
				alarmframe->SetActiveWindow();
				alarmframe->SetFocus();

				SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &lockTimeout, SPIF_SENDCHANGE);
			}

			if (IsIconic(alarmframe->m_hWnd)) {
				alarmframe->ShowWindow(SW_RESTORE);
			} else {
				alarmframe->ShowWindow(SW_SHOW);
			}

			AttachThreadInput(thread1, thread2, FALSE);

		} else {
			
			SetForegroundWindow(alarmframe->m_hWnd);
			alarmframe->BringWindowToTop();
			alarmframe->SetActiveWindow();
			alarmframe->SetFocus();
			
			if (IsIconic(alarmframe->m_hWnd)) {
				alarmframe->ShowWindow(SW_RESTORE);
			} else {
				alarmframe->ShowWindow(SW_SHOW);
			}
		}		

		return (GetForegroundWindow() == alarmframe->m_hWnd && GetActiveWindow() == alarmframe->m_hWnd);
	}

	void CheckAlarmWindowSize()
	{
		// When playing starcraft the resolution is 640x480 and alarm window size
		// uses that resolution, we should check in OneSecondTimer whether resolution
		// changed and resize window.

		if (alarmframe && alarmframe->IsWindow()) {
			int newcxscreen = GetSystemMetrics(SM_CXSCREEN);
			int newcyscreen = GetSystemMetrics(SM_CYSCREEN);
			if (newcxscreen != cxscreen || newcyscreen != cyscreen) {
				cxscreen = newcxscreen;
				cyscreen = newcyscreen;
				alarmframe->MoveWindow(0, 0, cxscreen, cyscreen, TRUE);				
			}
		}
	}

	void DisplayBalloon(wchar_t* title, wchar_t* text)
	{
		notifyicon.uFlags = NIF_INFO;
		if (WinVersionMinimum(WINVERSION_XPSP2)) {
			notifyicon.dwInfoFlags = NIIF_USER; // will use notifyicon.hIcon
		} else {
			notifyicon.dwInfoFlags = NIIF_INFO; // an information icon
		}		
		notifyicon.uTimeout = 0;
		
		wcsncpy_s(notifyicon.szInfo, _countof(notifyicon.szInfo), text, _TRUNCATE);
		wcsncpy_s(notifyicon.szInfoTitle, _countof(notifyicon.szInfoTitle), title, _TRUNCATE);

		BOOL b = Shell_NotifyIcon(NIM_MODIFY, &notifyicon);
		ASSERT_EXIT(b, "Shell_NotifyIcon(NIM_MODIFY)");
	}
	
	void HideBalloon()
	{
		notifyicon.uFlags = NIF_INFO;
		wcsncpy_s(notifyicon.szInfo, _countof(notifyicon.szInfo), L"", _TRUNCATE);
		BOOL b = Shell_NotifyIcon(NIM_MODIFY, &notifyicon);
		ASSERT_EXIT(b, "Shell_NotifyIcon(NIM_MODIFY)");
	}

	void FirstRunBalloon()
	{
		if (LANG_POLISH == GetLanguage()) {
			DisplayBalloon(L"5 Minute Break", 
				L"Kliknij lewym przyciskiem by ustawiæ timer na 1 godzinê.\n"\
				L"Kliknij ponownie by go wy³¹czyæ.\n"\
				L"Pod prawym przyciskiem jest menu z opcjami."
			);
		} else {
			DisplayBalloon(L"5 Minute Break", 
				L"Left click to set timer for 1 hour.\n"\
				L"Click again to set it off.\n"\
				L"Right click to see context menu."
			);
		}
	}

	void ApplicationUpdatedBalloon(int newVersion)
	{
		wchar_t message[100];
		int major;
		int minor;
		major = (int) floor(newVersion / 100.0f);
		minor = newVersion % 100;
		if (LANG_POLISH == GetLanguage()) {
			swprintf_s(message, _countof(message), L"Uaktualniono aplikacjê do wersji %d.%02d", major, minor);
		} else {
			swprintf_s(message, _countof(message), L"Application was updated to version %d.%02d", major, minor);
		}
		DisplayBalloon(L"5 Minute Break", message);
	}

	void BreakIn5MinutesBalloon()
	{
		// Five minutes before break display a notification, so user can prepare for the break.
		
		if (LANG_POLISH == GetLanguage()) {
			DisplayBalloon(L"5 Minute Break", 
				L"Za 5 minut bêdzie przerwa.\n"\
				L"Przygotuj siê..."
			);
		} else {
			DisplayBalloon(L"5 Minute Break", 
				L"In 5 minutes there will be a break.\n"\
				L"Be prepared..."
			);
		}
	}

	int GetDefaultTimer()
	{
		wchar_t location[256];
		int swp = swprintf_s((wchar_t*)&location, _countof(location), L"SOFTWARE\\%s", APP_NAME);
		ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");

		DWORD defaultTimer;
		bool result = ReadRegistryDword(HKEY_CURRENT_USER, location, L"DefaultTimer", &defaultTimer);
		// not doing anything with result

		if (result && defaultTimer > 0) {
			return defaultTimer;
		} else {
			return DEFAULT_TIMER;
		}
	}

	void SetDefaultTimer(int timer)
	{
		wchar_t location[256];
		int swp = swprintf_s((wchar_t*)&location, _countof(location), L"SOFTWARE\\%s", APP_NAME);
		ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");

		int result = WriteRegistryDword(HKEY_CURRENT_USER, location, L"DefaultTimer", timer);
		// not doing anything with result		
	}

	void CreateTrayContextMenu()
	{
		BOOL b;
		int menupos = 0;

		// -- subtimer MENU

		HMENU subtimer;
		subtimer = CreatePopupMenu();
		ASSERT_EXIT(subtimer, "CreatePopupMenu() - subtimer");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_2_HOURS, L"2 godziny");
		} else {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_2_HOURS, L"2 hours");
		}
		ASSERT_EXIT(b, "InsertMenu(DEFAULTTIMER_1_HOUR)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_1_HOUR, L"1 godzina");
		} else {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_1_HOUR, L"1 hour");
		}
		ASSERT_EXIT(b, "InsertMenu(DEFAULTTIMER_1_HOUR)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_45_MINUTES, L"45 minut");
		} else {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_45_MINUTES, L"45 minutes");
		}
		ASSERT_EXIT(b, "InsertMenu(DEFAULTTIMER_45_MINUTES)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_30_MINUTES, L"30 minut");
		} else {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_30_MINUTES, L"30 minutes");
		}
		ASSERT_EXIT(b, "InsertMenu(DEFAULTTIMER_30_MINUTES)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_15_MINUTES, L"15 minut");
		} else {
			b = InsertMenu(subtimer, menupos++, MF_BYPOSITION | MF_STRING, DEFAULTTIMER_15_MINUTES, L"15 minutes");
		}
		ASSERT_EXIT(b, "InsertMenu(DEFAULTTIMER_15_MINUTES)");

		int defaultTimer = GetDefaultTimer();

		if (120 == defaultTimer) {
			CheckMenuItem(subtimer, DEFAULTTIMER_2_HOURS, MF_CHECKED);
		} else if (60 == defaultTimer) {
			CheckMenuItem(subtimer, DEFAULTTIMER_1_HOUR, MF_CHECKED);
		} else if (45 == defaultTimer) {
			CheckMenuItem(subtimer, DEFAULTTIMER_45_MINUTES, MF_CHECKED);
		} else if (30 == defaultTimer) {
			CheckMenuItem(subtimer, DEFAULTTIMER_30_MINUTES, MF_CHECKED);
		} else if (15 == defaultTimer) {
			CheckMenuItem(subtimer, DEFAULTTIMER_15_MINUTES, MF_CHECKED);
		}

		// -- sublang MENU

		HMENU sublang;
		sublang = CreatePopupMenu();
		ASSERT_EXIT(sublang, "CreatePopupMenu() sublang")

		b = InsertMenu(sublang, menupos++, MF_BYPOSITION | MF_STRING, LANGMENU_ENGLISH, L"English");
		ASSERT_EXIT(b, "InsertMenu(LANGMENU_ENGLISH)");

		b = InsertMenu(sublang, menupos++, MF_BYPOSITION | MF_STRING, LANGMENU_POLISH, L"Polski");
		ASSERT_EXIT(b, "InsertMenu(LANGMENU_POLISH)");

		int language = GetLanguage();

		if (LANG_ENGLISH == language) {
			CheckMenuItem(sublang, LANGMENU_ENGLISH, MF_CHECKED);
		} else if (LANG_POLISH == language) {
			CheckMenuItem(sublang, LANGMENU_POLISH, MF_CHECKED);
		}

		// -- main MENU
		
		HMENU menu;		
		menu = CreatePopupMenu();
		ASSERT_EXIT(menu, "CreatePopupMenu() - menu");		
		
		menupos = 0;
		
		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_EXIT, L"Zakoñcz");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_EXIT, L"Exit");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_EXIT)");

		b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_SEPARATOR, (UINT_PTR) 0, (LPCTSTR) 0);
		ASSERT_EXIT(b, "InsertMenu(MF_SEPARATOR1)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_STARTUP, L"Uruchom przy starcie");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_STARTUP, L"Run at startup");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_STARTUP)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR) sublang, L"Jêzyk");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR) sublang, L"Language");
		}
		ASSERT_EXIT(b, "InsertMenu(Default timer)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR) subtimer, L"Domyœlny timer");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR) subtimer, L"Default timer");
		}
		ASSERT_EXIT(b, "InsertMenu(Default timer)");		

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_HELP, L"Pomoc");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_HELP, L"Help");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_STARTUP)");

		b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_SEPARATOR, (UINT_PTR) 0, (LPCTSTR) 0);
		ASSERT_EXIT(b, "InsertMenu(MF_SEPARATOR2)");

		b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_TIMER, GetAlarmString());
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_TIMER)");
		
		b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_SEPARATOR, (UINT_PTR) 0, (LPCTSTR) 0);
		ASSERT_EXIT(b, "InsertMenu(MF_SEPARATOR3)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_2_HOURS, L"2 godziny");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_2_HOURS, L"2 hours");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_2_HOURS)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_1_HOUR, L"1 godzina");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_1_HOUR, L"1 hour");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_1_HOUR)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_45_MINUTES, L"45 minut");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_45_MINUTES, L"45 minutes");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_45_MINUTES)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_30_MINUTES, L"30 minut");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_30_MINUTES, L"30 minutes");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_30_MINUTES)");

		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_15_MINUTES, L"15 minut");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_15_MINUTES, L"15 minutes");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_15_MINUTES)");
		
		if (LANG_POLISH == GetLanguage()) {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_5_MINUTES, L"5 minut");
		} else {
			b = InsertMenu(menu, menupos++, MF_BYPOSITION | MF_STRING, TRAYMENU_5_MINUTES, L"5 minutes");
		}
		ASSERT_EXIT(b, "InsertMenu(TRAYMENU_5_MINUTES)");

		PrepareTrayContextMenu(menu);

		b = SetForegroundWindow(self->m_hWnd);
		//ASSERT(b, "SetForegroundWindow(self->m_hWnd)");
		if (!b) {
			// If you drag with right mouse click SetForegroundWindow() will fail,
			// and assert errors are displayed, and the context menu is displayed
			// in the place where you dragged it - in this case we do not display context menu.
			DestroyMenu(menu);
			return;
		}

		CPoint pos;
		GetCursorPos(&pos);

		b = TrackPopupMenu(menu, TPM_BOTTOMALIGN, pos.x, pos.y, 0, self->m_hWnd, 0);
		// if (!b) DEBUG_INT(GetLastError());
		ASSERT_EXIT(b, "TrackPopupMenu()");
		
		self->PostMessage(WM_NULL); // google "PRB: Menus for Notification Icons Don't Work Correctly" > http://support.microsoft.com/kb/135788
		DestroyMenu(menu);
	}

	void PrepareTrayContextMenu(HMENU menu)
	{
		if (IsRunningAtStartup(APP_NAME)) {
			CheckMenuItem(menu, TRAYMENU_STARTUP, MF_CHECKED);
		}
		EnableMenuItem(menu, TRAYMENU_TIMER, MF_GRAYED);

		int minutes = GetMinutes();
		if (5 == minutes) {
			CheckMenuItem(menu, TRAYMENU_5_MINUTES, MF_CHECKED);
		} else if (15 == minutes) {
			CheckMenuItem(menu, TRAYMENU_15_MINUTES, MF_CHECKED);
		} else if (30 == minutes) {
			CheckMenuItem(menu, TRAYMENU_30_MINUTES, MF_CHECKED);
		} else if (45 == minutes) {
			CheckMenuItem(menu, TRAYMENU_30_MINUTES, MF_CHECKED);
		} else if (60 == minutes) {
			CheckMenuItem(menu, TRAYMENU_1_HOUR, MF_CHECKED);
		} else if (120 == minutes) {
			CheckMenuItem(menu, TRAYMENU_2_HOURS, MF_CHECKED);
		}
	}

	void CreateTrayIcon()
	{
		ASSERT_EXIT(!notifyicon.cbSize, "notifyicon has already been created");

		/*
			Bug in windows SDK headers (both 7.0 and 7.1).
			Error: "error C2039: 'guidItem' : is not a member of '_NOTIFYICONDATAW'"
			
			When _WIN32_WINNT is 0x0500 (win2k) then the last member of NOTIFYICONDATA
			is "dwInfoFlags", not "guidItem" which is available for XP and later.

			Replacing "FIELD_OFFSET(NOTIFYICONDATAA, guidItem)" with 
			"FIELD_OFFSET(NOTIFYICONDATAA, dwInfoFlags)" is invalid!

		*/

		// Hardcoding v2_size=936
		// Where did I get that 936? Go edit stdafx.h and change _WIN32_WINNT to 0x0501,
		// also NTDDI_VERSION to NTDDI_WINXP, then you can use NOTIFYICONDATAW_V2_SIZE
		// to check the size, call DEBUG_INT(NOTIFYICONDATAW_V2_SIZE).

		notifyicon.cbSize = 936; // NOTIFYICONDATAW_V2_SIZE == 936, win2k or later
		notifyicon.uVersion = NOTIFYICON_VERSION; // win2k or later, defines what wParam/lParam data comes in WM_TRAYICON message, 
										// used along with NIM_SETVERSION
		ASSERT_EXIT(self->m_hWnd, "self->m_hWnd");
		notifyicon.hWnd = self->m_hWnd;
		notifyicon.uID = IDI_BLUE;
		notifyicon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		notifyicon.uCallbackMessage = WM_TRAYICON;
		notifyicon.hIcon = AtlLoadIconImage(IDI_BLUE, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));		
		wcsncpy_s(notifyicon.szTip, _countof(notifyicon.szTip), GetAlarmString(), _TRUNCATE);

		BOOL b;		

		b = Shell_NotifyIcon(NIM_ADD, &notifyicon);
		if (!b) {
			
			MSGBOXPARAMS msg;
			ZeroMemory(&msg, sizeof(MSGBOXPARAMS));
			msg.cbSize = sizeof(MSGBOXPARAMS);
			msg.hInstance = __Module.m_hInst;			
			
			if (LANG_POLISH == GetLanguage()) {
				msg.lpszText = L"Nie uda³o siê utworzyæ ikony programu w zasobniku systemowym (obszar powiadomieñ).\n\n"\
					L"Funkcje programu zosta³y ograniczone, prawdopodobnie przez program antywirusowy\n"\
					L"(u¿ywasz Avasta?). Ustaw odpowiednie uprawnienia dla 5 Minute Break (dodaj wyj¹tek\n"
					L"w AutoSandbox jeœli u¿ywasz Avasta) i uruchom program ponownie.";
			} else {
				msg.lpszText = L"Could not create program icon in system tray (notification area).\n\n"\
					L"Program features have been limited, probably by an antivirus program (using Avast?).\n"\
					L"Set appropriate permissions for 5 Minute Break (add an exception in AutoSandbox if\n"\
					L"using Avast) and launch program again.";
			}
			
			msg.lpszCaption = L"5 Minute Break";			
			msg.dwStyle = MB_USERICON;
			msg.lpszIcon = MAKEINTRESOURCE(IDR_HELPFRAME);
			
			MessageBoxIndirect(&msg);
			exit(-1);
		}
		

		b = Shell_NotifyIcon(NIM_SETVERSION, &notifyicon);
		ASSERT_EXIT(b, "Shell_NotifyIcon(NIM_SETVERSION)");

		// Now let's display a notification balloon that says "I'm the new icon here!".
		if (IsFirstRun(APP_NAME, VERSION)) {
			FirstRunBalloon();
		} else if (ApplicationUpdated(APP_NAME, VERSION)) {
			ApplicationUpdatedBalloon(VERSION);
		}

		SetTimer(self->m_hWnd, TRAY_TIMER, 1000, 0);
		
		// Calling this func automatically fixes if there is a wrong path to application in registy startup.
		// For example user moved application's folder to another location, and run again app,
		// but didn't click context menu, if he resarted computer our app would not load.
		IsRunningAtStartup(APP_NAME);

		GetLanguage();
	}
	
	void DestroyTrayIcon()
	{
		if (notifyicon.cbSize) {
			Shell_NotifyIcon(NIM_DELETE, &notifyicon);
			ZeroMemory(&notifyicon, sizeof notifyicon);
		}
	}

	LRESULT OnTrayIcon(UINT, WPARAM wParam, LPARAM lParam)
	{
		// identifier set in "notifyicon.uID".
		if (wParam != IDI_BLUE && wParam != IDI_RED) {
			return 0;
		}
		switch (lParam)
		{
		case WM_LBUTTONUP:
			{
				if (alarmtime) {
					StopTimer();
				} else {
					StartNewTimer(GetDefaultTimer());
				}
			}
			break;
		case WM_CONTEXTMENU:
			{
				CreateTrayContextMenu();
			}
			break;
		case WM_MOUSEMOVE:
			{
				// After changing language, icon tip did not update.
				UpdateAlarmTip();
			}
			break;
		}
		return 0;
	}

	LRESULT OnTrayCommand(UINT, WPARAM wParam, LPARAM lParam)
	{
		int cmdid = LOWORD(wParam);
		int wmevent = HIWORD(wParam);

		switch (cmdid)
		{
		
		case TRAYMENU_5_MINUTES:
			StartNewTimer(5);
			break;
		
		case TRAYMENU_15_MINUTES:
			StartNewTimer(15);
			break;
		
		case TRAYMENU_30_MINUTES:
			StartNewTimer(30);
			break;

		case TRAYMENU_45_MINUTES:
			StartNewTimer(45);
			break;
		
		case TRAYMENU_1_HOUR:
			StartNewTimer(60);
			break;

		case TRAYMENU_2_HOURS:
			StartNewTimer(120);
			break;
		
		case TRAYMENU_HELP:
			HelpWindow();
			break;
		
		case TRAYMENU_STARTUP:
			if (IsRunningAtStartup(APP_NAME)) {
				RunAtStartup(APP_NAME, false);
			} else {
				RunAtStartup(APP_NAME, true);
			}
			break;
		
		case TRAYMENU_EXIT:
			// Using SendMessage() and not PostMessage() as this command
			// might sent from another process, a new app opening and closing the other running instance.
			
			// We are sending WM_DESTROY as another instance of program requires
			// us to do this immediately. It is also used in context menu > Exit, who
			// cares about WM_CLOSE, dig it.

			// Earlier we used this code:
			// SendMessage(self->m_hWnd, WM_DESTROY, 0, 0);
			// But it throws an assertion error in debug mode in atlwin.h, because m_hWnd is not null,
			// so it looks like that instead of sending message we should call DestroyWindow().

			self->DestroyWindow();

			break;

		// -- language SUBMENU

		case LANGMENU_ENGLISH:
			SetLanguage(LANG_ENGLISH);
			break;

		case LANGMENU_POLISH:
			SetLanguage(LANG_POLISH);
			break;

		// -- default timer SUBMENU

		case DEFAULTTIMER_2_HOURS:
			SetDefaultTimer(120);
			break;

		case DEFAULTTIMER_1_HOUR:
			SetDefaultTimer(60);
			break;

		case DEFAULTTIMER_45_MINUTES:
			SetDefaultTimer(45);
			break;

		case DEFAULTTIMER_30_MINUTES:
			SetDefaultTimer(30);
			break;

		case DEFAULTTIMER_15_MINUTES:
			SetDefaultTimer(15);
			break;

		}

		return 1;
	}

	LRESULT OnTrayTimer(UINT, WPARAM wParam, LPARAM lParam)
	{
		if (wParam != TRAY_TIMER) {
			return 0;
		}
		OneSecondTimer();
		return 1;
	}

};
