//
// AlarmFrame.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

// Multiple windows inheriting from CFrameWindowImpl:
// http://tech.groups.yahoo.com/group/wtl/message/4204
// - delete secondary frames (must be allocated dynamically using "new")
//  in their OnFinalMessage ("delete this", but also set pointer in parent class to 0),
//  see OnFinalMessage: http://msdn.microsoft.com/en-us/library/wecead4y(v=vs.80).aspx
// - override WM_DESTROY so that PostQuitMessage is not called (set bHandled=TRUE)
// - accelerators: keep active frame by trapping WM_ACTIVATE and in PreTranslateMessage()
//  add check if "this" == active frame, if not return false.

#include "WebBrowser/WebBrowserFrame.h"
#include "WebBrowser/FullScreenFrame.h"

#include "Language.h"


#define EXTERNAL_STARTNEWTIMER      1
#define EXTERNAL_FIVEMOREMINUTES    2
#define EXTERNAL_CLOSEWINDOW        3

// ----


class AlarmView : public CWindowImpl<AlarmView, CAxWindow>
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

    BEGIN_MSG_MAP(AlarmView)
    END_MSG_MAP()
};


// ----


template <class T>
class AlarmFrame :
    public CFrameWindowImpl<AlarmFrame<T>>,
    public CUpdateUI<AlarmFrame<T>>,
    public CMessageFilter,
    public CIdleHandler,
    public WebBrowserFrame<AlarmFrame<T>>,
    public FullScreenFrame<AlarmFrame<T>>
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_ALARMFRAME)

    AlarmView rootview;
    T* parentframe; // as we need to set pointer to this class to 0, in TrayIcon class.

    AlarmFrame(T* inparentframe)
    {
        parentframe = inparentframe;
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if(CFrameWindowImpl<AlarmFrame<T>>::PreTranslateMessage(pMsg))
            return TRUE;

        return rootview.PreTranslateMessage(pMsg);
    }

    virtual BOOL OnIdle()
    {
        return FALSE;
    }

    BEGIN_UPDATE_UI_MAP(AlarmFrame)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(AlarmFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        CHAIN_MSG_MAP(CUpdateUI<AlarmFrame<T>>)
        CHAIN_MSG_MAP(CFrameWindowImpl<AlarmFrame<T>>)
    END_MSG_MAP()


    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        wchar_t cwd[1024];
        wchar_t htmlfile[1024];
        GetExecutableDir(cwd, _countof(cwd));

        if (LANG_POLISH == GetLanguage()) {
            swprintf_s(htmlfile, _countof(htmlfile), L"%s\\Alarm_pl.html", cwd);
        } else {
            swprintf_s(htmlfile, _countof(htmlfile), L"%s\\Alarm_en.html", cwd);
        }

        CreateBrowser(htmlfile);
        SetFullScreen(true);

        CMessageLoop* pLoop = __Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);
        return 0;
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        if (isfullscreen)
            ShowTaskBar(true);

        CMessageLoop* pLoop = __Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->RemoveMessageFilter(this);
        pLoop->RemoveIdleHandler(this);
        bHandled = TRUE; // so that it doesn't call PostQuitMessage(), as our tray icon still runs.
        return 0; // this was also changed along with bHandled, from 1 to 0, from msdn "If an application processes this message, it should return zero.".
    }

    virtual void OnFinalMessage(HWND hwnd)
    {
        parentframe->alarmframe = 0;
        //parentframe->StartNewTimer(parentframe->GetDefaultTimer());
        delete this;
    }

    int ExternalID(wchar_t* funcName)
    {
        if (0 == wcscmp(funcName, L"StartNewTimer")) {
            return EXTERNAL_STARTNEWTIMER;
        } else if (0 == wcscmp(funcName, L"FiveMoreMinutes")) {
            return EXTERNAL_FIVEMOREMINUTES;
        } else if (0 == wcscmp(funcName, L"CloseWindow")) {
            return EXTERNAL_CLOSEWINDOW;
        }
        return 0;
    }

    bool ExternalCall(int funcID)
    {
        switch (funcID)
        {
        case EXTERNAL_STARTNEWTIMER:
            parentframe->StartNewTimer(parentframe->GetDefaultTimer());
            ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
            return true;
        case EXTERNAL_FIVEMOREMINUTES:
            parentframe->StartNewTimer(5);
            ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
            return true;
        case EXTERNAL_CLOSEWINDOW:
            ::PostMessageA(m_hWnd, WM_CLOSE, 0, 0);
            return true;
        }
        return false;
    }
};
