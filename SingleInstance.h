// not used currently.

//
// Code taken from here:
// http://support.microsoft.com/kb/243953 (How to limit 32-bit applications to one instance in Visual C++)
//

#pragma once

#include <windows.h>

class SingleInstance
{

protected:

    DWORD lasterror;
    HANDLE mutex;

public:

    SingleInstance(TCHAR* mutexname)
    {
        mutex = CreateMutex(NULL, FALSE, mutexname);
        lasterror = GetLastError();
    }

    ~SingleInstance()
    {
        if (mutex) {
            CloseHandle(mutex);
            mutex = NULL;
        }
    }

    BOOL IsRunning()
    {
        return (ERROR_ALREADY_EXISTS == lasterror);
    }

};
