//
// winmain.cpp
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#include "stdafx.h"
#include "resource.h"

// Saved in registry "InternalVersion", used to display notifications about updated application.
#define VERSION	107

#include "debug.h"

//#include "SingleInstance.h"
#include "Process.h"
#include "ExecutablePath.h"

#include "MainFrame.h"
#include "WebBrowser/InternetFeatures.h"

CAppModule __Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	__Module.AddMessageLoop(&theLoop);

	SetInternetFeatures();

	MainFrame mainFrame;
	if(mainFrame.CreateEx() == NULL) {
		ASSERT(0, "mainFrame.CreateEx() failed");
		return 0;
	}

	mainFrame.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	__Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	// When application is updated we need to close the other instance, if it's running.
	// Instead of terminating process (which would leave an orphaned icon in system tray),
	// we send a message so that it closes gracefully. This will also close any help window or alarm window in that process.
	wchar_t executable[100];
	GetExecutableFilename(executable, _countof(executable));
	SendMessageToProcess(executable, WM_COMMAND, MAKEWPARAM(TRAYMENU_EXIT, 0), 0);

	// Just to make sure let's terminate, in case SendMessageToProcess() failed.
	TerminateProcessByName(executable);

	/*
	OFF: it still displays message that another instance is running during update, when it's not.
	Let's get rid of this, we already are sending message to another process, and even if it failed
	we terminate the process.

	// In MS example this is a global variable, but vars in winmain also exist till the end of the program.
	// This also works even if the exe has been copied and renamed.
	SingleInstance __singleInstance(_T("{4633D898-8DB2-4840-93CD-24EF99634A5C}")); // generated using guid tool.
	if (__singleInstance.IsRunning()) {
		MessageBox(NULL, L"5 Minute Break is already running, search for an alarm clock icon in notification area near the clock.\n"\
			L"If program hanged up then try to terminate process in Task Manager (ctrl+alt+delete) in Processes tab.", L"5 Minute Break", 0);
		return FALSE;
	}
	*/

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

	hRes = __Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	AtlAxWinInit();

	int nRet = Run(lpstrCmdLine, SW_HIDE);

	__Module.Term();
	::CoUninitialize();

	return nRet;
}
