#pragma once

bool ReadRegistryString(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, wchar_t* keyvalue, int keyvalue_length);
bool ReadRegistryDword(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, DWORD* keyvalue);
bool WriteRegistryString(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, wchar_t* keyvalue);
bool WriteRegistryDword(HKEY hkeyroot, wchar_t* location, wchar_t* keyname, DWORD keyvalue);
bool DeleteRegistryValue(HKEY hkeyroot, wchar_t* location, wchar_t* keyname);
bool DeleteRegistryKey();
