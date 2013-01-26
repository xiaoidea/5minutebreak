// Copyright (c) 2012-2013 Czarek Tomczak. All rights reserved.
// License: New BSD License.
// Website: http://code.google.com/p/5minutebreak/

#include <comutil.h>
#include <atlbase.h>
#include <iostream>
#include <windows.h>
#include <Dshow.h>

// Playing sound: IMediaControl, IGraphBuilder
#pragma comment(lib, "Strmiids.lib")

using namespace std;

void PlaySound() {
    HRESULT hr;
    CComPtr<IGraphBuilder> graphBuilder;
    hr = graphBuilder.CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC);
    ATLASSERT(SUCCEEDED(hr));
    CComQIPtr<IMediaControl> mediaControl(graphBuilder);
    hr = graphBuilder->RenderFile(L"c:\\cpp-test\\Debug\\alarm-default.mp3", 0);
    ATLASSERT(SUCCEEDED(hr));
    hr = mediaControl->Run();
    ATLASSERT(SUCCEEDED(hr));
    MessageBox(NULL, L"Playing sound", L"Playing sound", MB_OK);
}
int main() {
    HRESULT hr = ::CoInitialize(NULL);
    ATLASSERT(SUCCEEDED(hr));
    PlaySound();
    PlaySound();
    ::CoUninitialize();
    system("pause");
    return 0;
}