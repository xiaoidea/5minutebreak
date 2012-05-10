//
// WinVersion.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

enum WinVersion
{
	WINVERSION_XPSP2
};

bool WinVersionMinimum(WinVersion minimum)
{
	OSVERSIONINFOEX osversion;
	ZeroMemory(&osversion, sizeof(osversion));
	osversion.dwOSVersionInfoSize = sizeof osversion;
	
	BOOL b = GetVersionEx((OSVERSIONINFO*)&osversion);
	ASSERT_EXIT(b, "GetVersionEx()");

	if (WINVERSION_XPSP2 == minimum) {
		if (osversion.dwMajorVersion > 5) {
			return true;
		} else if (osversion.dwMajorVersion == 5 && osversion.dwMinorVersion > 1) {
			// XP x64 - dwMinorVersion=2
			// XP x64 released a year after XP SP2.
			return true;
		} else if (osversion.dwMajorVersion == 5 && osversion.dwMinorVersion == 1 && osversion.wServicePackMajor >= 2) {
			return true;
		} else {
			return false;
		}
	}

	ASSERT_EXIT(false, "Invalid WinVersion");
}
