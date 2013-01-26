//
// ExecutablePaths.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

void GetExecutablePath(wchar_t* out, int length);
void GetExecutablePathQuoted(wchar_t* outpath, int outpath_length);
void GetExecutableDir(wchar_t* out, int length);
void GetExecutableFilename(wchar_t* out, int length);
void GetExecutableName(wchar_t* out, int length);
