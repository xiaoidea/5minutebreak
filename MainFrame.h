//
// MainFrame.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

#include "TrayIcon.h"

class MainFrame :
    public CFrameWindowImpl<MainFrame>,
    public CUpdateUI<MainFrame>,
    public CMessageFilter,
    public CIdleHandler,
    public TrayIcon<MainFrame>
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

    MainFrame() {}

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        return CFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg);
    }

    virtual BOOL OnIdle()
    {
        return FALSE;
    }


    BEGIN_UPDATE_UI_MAP(MainFrame)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP_EX(MainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER_EX(WM_TRAYICON, OnTrayIcon)
        MESSAGE_HANDLER_EX(WM_COMMAND, OnTrayCommand);
        MESSAGE_HANDLER_EX(WM_TIMER, OnTrayTimer);
        CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<MainFrame>)
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CreateTrayIcon();
        CMessageLoop* pLoop = __Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);
        return 0;
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        DestroyTrayIcon();
        CMessageLoop* pLoop = __Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->RemoveMessageFilter(this);
        pLoop->RemoveIdleHandler(this);
        bHandled = FALSE;
        return 1;
    }
};
