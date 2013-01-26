//
// RunAtStartup.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

#include "ExecutablePath.h"
#include "Registry.h"

// ----

bool ApplicationUpdated(wchar_t* appname, int currentVersion);
bool IsFirstRun(wchar_t* appname, int currentVersion);
bool IsRunningAtStartup(wchar_t* appname);
bool RunAtStartup(wchar_t* appname, bool dorun);

// ----

bool ApplicationUpdated(wchar_t* appname, int currentVersion)
{
    // You must first call IsFirstRun(). If you call ApplicationUpdated() then it will return true when it's first run.
    // After you call IsFirstRun(), a call to ApplicationUpdated() will return false when it's first run.

    wchar_t location[256];
    int swp = swprintf_s((wchar_t*)&location, _countof(location), L"SOFTWARE\\%s", appname);
    ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");

    // "InternalVersion" because "Version" is already used by Advanced Installer.

    DWORD internalVersion;
    bool result = ReadRegistryDword(HKEY_CURRENT_USER, location, L"InternalVersion", &internalVersion);

    bool ret;
    if (result) {
        if (currentVersion != internalVersion) {
            ret = true;
        } else {
            ret = false;
        }
    } else {
        // 1.00 did not have "InternalVersion" in registry.
        ret = true;
    }

    WriteRegistryDword(HKEY_CURRENT_USER, location, L"InternalVersion", currentVersion);

    return ret;
}

bool IsFirstRun(wchar_t* appname, int currentVersion)
{
    // You should always call this function, as it creates initial settings for the app in the registry.

    // We must know whether this is the first run of application, because we want to display
    // a balloon tooltip so that user knows where to find our program's icon in system tray.

    // Only first call to this function may return true,
    // it automatically creates a registry key so that any subsequent calls return false.

    bool isfirstrun;
    DWORD runatstartup; // not used really.
    wchar_t location[256];

    int swp = swprintf_s((wchar_t*)&location, _countof(location), L"SOFTWARE\\%s", appname);
    ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");

    bool result = ReadRegistryDword(HKEY_CURRENT_USER, location, L"FirstRun", &runatstartup);

    if (result) {
        isfirstrun = false;
    } else {
        isfirstrun = true;

        // This is the first run, let's create a key in registry that will tell
        // us on next program call that this is not the first run, FirstRun=0.
        WriteRegistryDword(HKEY_CURRENT_USER, location, L"FirstRun", 0);

        // Let's add our application to run at startup.
        // OFF: we do not do it by default as Comodo Firewall detects it as suspicious behavior.
        // RunAtStartup(appname, true);
    }

    if (isfirstrun) {
        // We don't do anything with the bool result, on first run there is no need
        // to display an update notification, we already have a "first run" notification.
        ApplicationUpdated(APP_NAME, currentVersion);
    }

    return isfirstrun;
}

bool IsRunningAtStartup(wchar_t* appname)
{
    wchar_t keyvalue[1024];
    bool result = ReadRegistryString(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        appname, keyvalue, _countof(keyvalue));
    if (result) {
        // Case: program's folder has moved to another location, an invalid path would be in registry startup.
        wchar_t executablequoted[1024];
        GetExecutablePathQuoted((wchar_t*)&executablequoted, _countof(executablequoted));
        if (wcscmp(keyvalue, executablequoted) != 0) {
            RunAtStartup(appname, true);
        }
    }
    return result;
}

bool RunAtStartup(wchar_t* appname, bool dorun)
{
    bool b;
    if (dorun) {
        wchar_t executable[1024];
        GetExecutablePathQuoted(executable, _countof(executable));
        b = WriteRegistryString(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            appname, executable);
    } else {
        b = DeleteRegistryValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            appname);
    }
    return b;
}
