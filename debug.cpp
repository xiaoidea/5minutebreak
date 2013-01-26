//
// debug.cpp
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#include "stdafx.h"

char* WideToAscii(wchar_t* wide)
{
	int asciisize = WideCharToMultiByte(CP_UTF8, 0, wide, -1, 0, 0, 0, 0);
	char* ascii = (char*)malloc(asciisize * sizeof(char));
	WideCharToMultiByte(CP_UTF8, 0, wide, -1, ascii, asciisize, 0, 0);
	return ascii;
}
