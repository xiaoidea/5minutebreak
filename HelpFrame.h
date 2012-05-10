//
// HelpFrame.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

// Multiple windows inheriting from CFrameWindowImpl:
// http://tech.groups.yahoo.com/group/wtl/message/4204
// - delete secondary frames (must be allocated dynamically using "new")
//	in their OnFinalMessage ("delete this", but also set pointer in parent class to 0),
//	see OnFinalMessage: http://msdn.microsoft.com/en-us/library/wecead4y(v=vs.80).aspx
// - override WM_DESTROY so that PostQuitMessage is not called (set bHandled=TRUE)
// - accelerators: keep active frame by trapping WM_ACTIVATE and in PreTranslateMessage()
//	add check if "this" == active frame, if not return false.

#include "WebBrowser/WebBrowserFrame.h"
#include "WebBrowser/FullScreenFrame.h"
#include "ExecutablePath.h"

#include "Language.h"


// ----


class HelpView : public CWindowImpl<HelpView, CAxWindow>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CAxWindow::GetWndClassName())

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		   (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return FALSE;

		return (BOOL)SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
	}

	BEGIN_MSG_MAP(HelpView)
	END_MSG_MAP()
};


// ----


template <class T>
class HelpFrame :
	public CFrameWindowImpl<HelpFrame<T>>,
	public CUpdateUI<HelpFrame<T>>,
	public CMessageFilter,
	public CIdleHandler,
	public WebBrowserFrame<HelpFrame<T>>
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_HELPFRAME)

	HelpView rootview;
	T* parentframe; // as we need to set pointer to this class to 0, in TrayIcon class.

	HelpFrame(T* inparentframe)
	{
		parentframe = inparentframe;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<HelpFrame<T>>::PreTranslateMessage(pMsg))
			return TRUE;

		return rootview.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(HelpFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(HelpFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CUpdateUI<HelpFrame<T>>)
		CHAIN_MSG_MAP(CFrameWindowImpl<HelpFrame<T>>)
	END_MSG_MAP()

	int GetOptimalWidth()
	{
		return 600;
	}

	int GetOptimalHeight()
	{
		if (LANG_POLISH == GetLanguage()) {
			return 500;
		} else {
			return 455;
		}
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		wchar_t respath[1024];
		wchar_t exepath[1024];
		GetExecutablePath(exepath, _countof(exepath));
		wchar_t idhelp[30];
		if (LANG_POLISH == GetLanguage()) {
			wcsncpy_s(idhelp, _countof(idhelp), L"IDHTML_HELP_PL", _TRUNCATE);
		} else {
			wcsncpy_s(idhelp, _countof(idhelp), L"IDHTML_HELP_EN", _TRUNCATE);
		}

		int swp = swprintf_s(respath, _countof(respath), L"res://%s/%s", exepath, idhelp);
		ASSERT_EXIT((-1 != swp), "swprintf_s(res://IDHTML_HELP)");
		CreateBrowser(respath);

		CMessageLoop* pLoop = __Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

		lpMMI->ptMinTrackSize.x = GetOptimalWidth(); // min width
		lpMMI->ptMinTrackSize.y = GetOptimalHeight(); // min height
		lpMMI->ptMaxTrackSize.x = GetOptimalWidth(); // max width, cannot set to 0!
		lpMMI->ptMaxTrackSize.y = GetOptimalHeight(); // max height

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CMessageLoop* pLoop = __Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		bHandled = TRUE; // so that it doesn't call PostQuitMessage(), as our tray icon still runs.
		return 0; // this was also changed along with bHandled, from 1 to 0, from msdn "If an application processes this message, it should return zero.".
	}

	virtual void OnFinalMessage(HWND hwnd)
	{
		parentframe->helpframe = 0;
		delete this;
	}
};
