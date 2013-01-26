// Copyright (c) 2012-2013 Czarek Tomczak. All rights reserved.
// License: New BSD License.
// Project website: http://code.google.com/p/5minutebreak/

#include "stdafx.h"

// Playing sound: IMediaControl, IGraphBuilder
#include <Dshow.h>
#pragma comment(lib, "Strmiids.lib")

#include "alarm-sound.h"
#include "debug.h"
#include "defines.h"
#include "ExecutablePath.h"
#include "Registry.h"
#include "TrayMenu.h"

CComPtr<IGraphBuilder> g_graphBuilder;
CComQIPtr<IMediaControl> g_mediaControl;

void PlayAlarmSound() {
  // What sound to play?
  int alarmSound = GetAlarmSound();
  if (ALARM_SOUND_DISABLED == alarmSound)
    return;

  // Get path to mp3 file
  wchar_t cwd[1024];
	wchar_t mp3File[1024];
  GetExecutableDir(cwd, _countof(cwd));
  if (ALARM_SOUND_DEFAULT == alarmSound) {
	  swprintf_s(mp3File, _countof(mp3File), L"%s\\alarm-default.mp3", cwd);
  } else if (ALARM_SOUND_ANNOYING == alarmSound) {
    swprintf_s(mp3File, _countof(mp3File), L"%s\\alarm-annoying.mp3", cwd);
  } else {
    ATLASSERT(false);
    return;
  }

  // Play mp3 sound
  HRESULT hr;
  if (g_mediaControl) {
    g_mediaControl.Release();
  }
  if (g_graphBuilder) {
    g_graphBuilder.Release();
  }  
  hr = g_graphBuilder.CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC);
  ATLASSERT(SUCCEEDED(hr));  
  hr = g_graphBuilder.QueryInterface(&g_mediaControl);
  ATLASSERT(SUCCEEDED(hr));
  hr = g_graphBuilder->RenderFile(mp3File, 0);
  ATLASSERT(SUCCEEDED(hr));
  hr = g_mediaControl->Run();
  ATLASSERT(SUCCEEDED(hr));
}
int GetAlarmSound() {
  // Read from registry.
	wchar_t location[256];
	int swp = swprintf_s(location, _countof(location), L"SOFTWARE\\%s", APP_NAME);
	ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");
	DWORD alarmSound;
	bool b = ReadRegistryDword(HKEY_CURRENT_USER, location, L"AlarmSound", &alarmSound);
  if (b) {
    if (ALARM_SOUND_ANNOYING == alarmSound)
      return ALARM_SOUND_ANNOYING;
    else if (ALARM_SOUND_DISABLED == alarmSound)
      return ALARM_SOUND_DISABLED;
    else
      return ALARM_SOUND_DEFAULT;
  } else {
    return ALARM_SOUND_DEFAULT;
  }
}
void SetAlarmSound(int alarmSound) {
  wchar_t location[256];
	int swp = swprintf_s(location, _countof(location), L"SOFTWARE\\%s", APP_NAME);
	ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");
	WriteRegistryDword(HKEY_CURRENT_USER, location, L"AlarmSound", alarmSound);
}
