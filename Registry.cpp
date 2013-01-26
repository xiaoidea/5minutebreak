//
// Registry.cpp
// Copyright (c) Czarek Tomczak. All rights reserved.
//

// In windows 7 to be able to write to HKEY_LOCAL_MACHINE you
// need administrator privileges, you better write to HKEY_CURRENT_USER.

/*
	Examples:

	Write:
		
		WriteRegistryString(HKEY_CURRENT_USER, L"SOFTWARE", L"test", L"456");

		WriteRegistryDword(HKEY_CURRENT_USER, L"SOFTWARE", L"test3", 123);
	
	Read:
		
		wchar_t value[100];
		ReadRegistryString(HKEY_CURRENT_USER, L"SOFTWARE", L"test", (wchar_t*) &value, 100);	
		DEBUGW(value);

		DWORD test3;
		ReadRegistryDword(HKEY_CURRENT_USER, L"SOFTWARE", L"test3", &test3);
		DEBUG_INT(test3);

	Delete:
		
		DeleteRegistryValue(HKEY_CURRENT_USER, L"SOFTWARE", L"test");
*/

#include "stdafx.h"
#include "debug.h"

#include <Winreg.h>
#pragma comment(lib, "Advapi32")

bool ReadRegistryString(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, wchar_t* keyvalue, int keyvalue_length)
{
	int result;
	HKEY hkey;
	LONG regopen = RegOpenKeyEx(hkeyroot, location, 0, KEY_READ, &hkey);
	if (ERROR_SUCCESS != regopen) {
		swprintf_s(keyvalue, keyvalue_length, L"");
		return false;
	}
	wchar_t* buffer = new wchar_t[keyvalue_length];
	DWORD buffersize = keyvalue_length * sizeof(wchar_t);
	DWORD type;
	LONG regquery = RegQueryValueEx(hkey, keyname, 0, &type, (LPBYTE) buffer, &buffersize);
	if (ERROR_SUCCESS != regquery) {
		RegCloseKey(hkey);
		swprintf_s(keyvalue, keyvalue_length, L"");
		return false;
	}
	if (type != REG_SZ) {
		RegCloseKey(hkey);
		swprintf_s(keyvalue, keyvalue_length, L"");
		return false;
	}
	result = swprintf_s(keyvalue, keyvalue_length, L"%s", buffer);
	ASSERT_EXIT((-1 != result), "swprintf_s(keyvalue)");
	delete[] buffer;
	buffer = 0;
	RegCloseKey(hkey);
	return true;
}

bool ReadRegistryDword(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, DWORD* keyvalue)
{
	HKEY hkey;
	LONG regopen = RegOpenKeyEx(hkeyroot, location, 0, KEY_READ, &hkey);
	if (ERROR_SUCCESS != regopen) {
		keyvalue = 0;
		return false;
	}
	DWORD buffer;
	DWORD buffersize = sizeof(DWORD);
	DWORD type;
	LONG regquery = RegQueryValueEx(hkey, keyname, 0, &type, (LPBYTE) &buffer, &buffersize);
	if (ERROR_SUCCESS != regquery) {
		RegCloseKey(hkey);
		keyvalue = 0;
		return false;
	}
	if (type != REG_DWORD) {
		RegCloseKey(hkey);
		keyvalue = 0;
		return false;
	}
	*keyvalue = buffer;
	RegCloseKey(hkey);
	return true;
}

bool WriteRegistryString(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, wchar_t* keyvalue)
{
	HKEY hkey;
	// The RegCreateKeyEx function creates all missing keys in the specified path.
	LONG regcreate = RegCreateKeyEx(hkeyroot, location, 0, 0, 0, KEY_SET_VALUE, 0, &hkey, 0);
	if (ERROR_SUCCESS != regcreate) {
		return false;
	}
	DWORD buffersize = (wcslen(keyvalue) + 1) * sizeof(wchar_t);
	LONG regset = RegSetValueEx(hkey, keyname, 0, REG_SZ, (LPBYTE) keyvalue, buffersize); 
	if (ERROR_SUCCESS != regset) {
		RegCloseKey(hkey);
		return false;
	}
	RegCloseKey(hkey);
	return true;
}

bool WriteRegistryDword(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, DWORD keyvalue)
{
	HKEY hkey;
	// The RegCreateKeyEx function creates all missing keys in the specified path.
	LONG regcreate = RegCreateKeyEx(hkeyroot, location, 0, 0, 0, KEY_SET_VALUE, 0, &hkey, 0);
	if (ERROR_SUCCESS != regcreate) {
		return false;
	}
	DWORD buffersize = sizeof(DWORD);
	LONG regset = RegSetValueEx(hkey, keyname, 0, REG_DWORD, (LPBYTE) &keyvalue, buffersize); 
	if (ERROR_SUCCESS != regset) {
		RegCloseKey(hkey);
		return false;
	}
	RegCloseKey(hkey);
	return true;
}

bool DeleteRegistryValue(HKEY hkeyroot, wchar_t* location, wchar_t* keyname)
{
	HKEY hkey;
	LONG regopen = RegOpenKeyEx(hkeyroot, location, 0, KEY_SET_VALUE, &hkey);
	if (ERROR_SUCCESS != regopen) {
		return false;
	}
	LONG regdelete = RegDeleteValue(hkey, keyname);
	if (ERROR_SUCCESS != regdelete) {
		return false;
	}
	return true;
}

bool DeleteRegistryKey()
{
	// TODO: RegDeleteKey(HKEY_CURRENT_USER, L"SOFTWARE\\test");
	return false;
}
