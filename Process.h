//
// Process.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

#include <tlhelp32.h>

BOOL CALLBACK EnumWindowsOfProcess(HWND hwnd, LPARAM lparam);
//int g_sent = 0;

void TerminateProcessByName(wchar_t* processName)
{
    PROCESSENTRY32 entry;
    ZeroMemory(&entry, sizeof(entry));
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, GetCurrentProcessId());

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (wcscmp(entry.szExeFile,  processName)== 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                if (hProcess == 0) {
                    continue;
                }
                // GetCurrentProcessId() - returns real process id > 0 (seen in Task Manager > Processes)
                if (entry.th32ProcessID > 0 && entry.th32ProcessID != GetCurrentProcessId()) {
                    TerminateProcess(hProcess, 0);
                }
                CloseHandle(hProcess);
            }
        }
    }

    CloseHandle(snapshot);
}

struct EnumWindowsStruct {
public:
    UINT msg;
    WPARAM wparam;
    LPARAM lparam;
    DWORD processId;
};

void SendMessageToProcess(wchar_t* processName, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // It ignores current process you're running in.
    // It sends message to all top-level windows of given process.
    // SendMessage() calls the window procedure for the specified window and does not return until the window procedure has processed the message.

    // Use it to send a message to your application when you're making an update,
    // and want to close the other instance of your app running.

    PROCESSENTRY32 entry;
    ZeroMemory(&entry, sizeof(entry));
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, GetCurrentProcessId());

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (wcscmp(entry.szExeFile,  processName)== 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                if (hProcess == 0) {
                    continue;
                }
                // GetCurrentProcessId() - returns real process id > 0 (seen in Task Manager > Processes)
                if (entry.th32ProcessID > 0 && entry.th32ProcessID != GetCurrentProcessId()) {

                    EnumWindowsStruct enumdata;
                    enumdata.msg = msg;
                    enumdata.wparam = wparam;
                    enumdata.lparam = lparam;
                    enumdata.processId = entry.th32ProcessID;

                    EnumWindows(&EnumWindowsOfProcess, reinterpret_cast<LPARAM>(&enumdata));

                    //DEBUG_INT(g_sent);
                }
                CloseHandle(hProcess);
            }
        }
    }

    CloseHandle(snapshot);
}
BOOL CALLBACK EnumWindowsOfProcess(HWND hwnd, LPARAM lparam)
{
    DWORD wndProcessId;
    GetWindowThreadProcessId(hwnd, &wndProcessId);

    EnumWindowsStruct* enumdata = reinterpret_cast<EnumWindowsStruct*>(lparam);

    if (enumdata->processId == wndProcessId) {
        SendMessage(hwnd, enumdata->msg, enumdata->wparam, enumdata->lparam);
        //g_sent++;
    }

    return TRUE;
}
