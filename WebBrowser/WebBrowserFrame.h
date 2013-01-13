//
// WebBrowserFrame.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

/*
    Author: Czarek Tomczak <czarek.tomczak@@gmail.com>

    How to use WebBrowser:

    1. You should include "debug.h" which defines macro ASSERT_EXIT(bool, msb).
    2. Before creating window include "InternetFeatures.h" and call SetInternetFeatures(),
        this gets rid of any restrictions in IE webbrowser.
    3. MainFrame should inherit:
        public WebBrowserFrame<MainFrame>,
        public FullScreenFrame<MainFrame>
    4. MainView member should be named "rootview".
    5. If you call SetFullScreen() you must call ShowTaskBar() in WM_CLOSE message,
        check isfullscreen: if (isfullscreen) ShowTaskBar(true).
*/

#include "WebBrowserFrameInterface.h"
#include "OleClientSite.h"
#include "DocHostUIHandlerDispatch.h"
#include "ClickEvents.h"
#include <comutil.h>
#include <string>

template <class RootFrame, bool t_bHasSip = true>
class WebBrowserFrame
    :
    public WebBrowserFrameInterface<RootFrame>
{
public:

    OleClientSite<RootFrame> oleClientSite;
    DocHostUIHandlerDispatch<RootFrame> docHostUIHandlerDispatch;
    ClickEvents<RootFrame> clickEvents;
    _variant_t clickDispatch;
    RootFrame* self;
    wchar_t allowedURL[2084]; // Default: "nohttp" - if url starts with http:// it will be opened in default browser, not in webbrowser control.

    WebBrowserFrame()
        :
        oleClientSite((WebBrowserFrameInterface<RootFrame>*)this),
        docHostUIHandlerDispatch(DocHostUIHandlerDispatch<RootFrame>((IOleClientSite*)&oleClientSite, (WebBrowserFrameInterface<RootFrame>*)this)),
        clickEvents(ClickEvents<RootFrame>(static_cast<RootFrame*>(this)))
    {
        self = static_cast<RootFrame*>(this);
        oleClientSite.dispatch = &docHostUIHandlerDispatch;

         // _variant_t declared in <comutil.h>
        clickDispatch.vt = VT_DISPATCH;
        clickDispatch.pdispVal = &clickEvents;
        wcsncpy_s(allowedURL, _countof(allowedURL), L"nohttp", _TRUNCATE);
    }

    void CreateBrowser(wchar_t* navigateurl)
    {
        self->m_hWndClient = self->rootview.Create(self->m_hWnd, self->rcDefault, (LPCTSTR) navigateurl,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
        ASSERT_EXIT(self->m_hWndClient, "self->m_hWndClient");

        // IOleClientSite
        HRESULT hr;

        CComQIPtr<IOleObject> oleObject;
        int ctrlid = self->rootview.GetDlgCtrlID();
        CComQIPtr<IWebBrowser2> webBrowser2;

        hr = self->GetDlgControl(ctrlid, IID_IWebBrowser2, (void**) &webBrowser2);
        ASSERT_EXIT(SUCCEEDED(hr), "self->GetDlgControl(IID_IWebBrowser2) failed");

        hr = webBrowser2->QueryInterface(IID_IOleObject, (void**) &oleObject);
        ASSERT_EXIT(SUCCEEDED(hr), "webBrowser2->QueryInterface(IID_IOleObject)");

        oleObject->SetClientSite(&oleClientSite);
        self->rootview.SetExternalUIHandler(&docHostUIHandlerDispatch);

        // Do not allow displaying files dragged into the window.
        webBrowser2->put_RegisterAsDropTarget(VARIANT_FALSE);

        // Attach OnClick event - to catch clicking any external links.

        CComQIPtr<IDispatch> dispatch;
        hr = webBrowser2->get_Document(&dispatch);
        ASSERT_EXIT(SUCCEEDED(hr), "webBrowser->get_Document(&dispatch)");

        CComQIPtr<IHTMLDocument2> htmlDocument2;
        hr = dispatch->QueryInterface(IID_IHTMLDocument2, (void**) &htmlDocument2);
        ASSERT_EXIT(SUCCEEDED(hr), "dispatch->QueryInterface(&htmlDocument2)");

        htmlDocument2->put_onclick(clickDispatch);

        /*
        CComQIPtr<IHTMLWindow2> htmlWindow2;
        hr = htmlDocument2->get_parentWindow((IHTMLWindow2**) &htmlWindow2);
        ASSERT_EXIT(SUCCEEDED(hr), "htmlDocument2->get_parentWindow(&htmlWindow2)");

        CComQIPtr<IHTMLWindow3> htmlWindow3;
        hr = htmlWindow2->QueryInterface(IID_IHTMLWindow3, (void**) &htmlWindow3);
        ASSERT_EXIT(SUCCEEDED(hr), "htmlWindow2->QueryInterface(IID_IHTMLWindow3)");

        // onbeforeunload (before the connection is made)
        // onunload (after initial connection is made, but before redirecting).
        CComBSTR onClick(TEXT( "onbeforeunload"));
        VARIANT_BOOL result = VARIANT_TRUE;
        hr = htmlWindow3->attachEvent(onClick, &clickEvents, &result);
        ASSERT_EXIT((result == VARIANT_TRUE), "window3->attachEvent(onClick)");
        */
    }

    virtual HWND GetHWND()
    {
        return self->m_hWnd;
    }

    virtual HWND View_GetHWND()
    {
        return self->rootview.m_hWnd;
    }

    virtual IOleClientSite* GetOleClientSite()
    {
        return &oleClientSite;
    }

    virtual int View_GetDlgCtrlID()
    {
        return self->rootview.GetDlgCtrlID();
    }

    virtual HRESULT Self_GetDlgControl(int nID, REFIID iid, void** ppCtrl)
    {
        return self->GetDlgControl(nID, iid, ppCtrl);
    }

    virtual bool GetActiveElement(wchar_t* outTag, wchar_t* outTypeAttr)
    {
        CComQIPtr<IWebBrowser2> webBrowser;
        HRESULT hr = self->GetDlgControl(self->rootview.GetDlgCtrlID(), IID_IWebBrowser2, (void**) &webBrowser);
        ASSERT_EXIT(SUCCEEDED(hr), "self->GetDlgControl(IID_IWebBrowser2) failed");

        CComQIPtr<IDispatch> dispatch;
        hr = webBrowser->get_Document(&dispatch);
        ASSERT_EXIT(SUCCEEDED(hr), "webBrowser->getDocument()");
        if (dispatch == NULL) {
            outTag = 0;
            return false;
        }

        CComQIPtr<IHTMLDocument2> htmlDocument2;
        hr = dispatch->QueryInterface(IID_IHTMLDocument2, (void**) &htmlDocument2);
        ASSERT_EXIT(SUCCEEDED(hr), "dispatch->QueryInterface(IID_IHTMLDocument2)");
        if (htmlDocument2 == NULL) {
            // Is it possible to return NULL for document?
            outTag = 0;
            return false;
        }

        CComQIPtr<IHTMLElement> htmlElement;
        hr = htmlDocument2->get_activeElement(&htmlElement);
        ASSERT_EXIT(SUCCEEDED(hr), "htmlDocument2->get_ActiveElement()");
        if (htmlElement == NULL) {
            // htmlElement might be NULL when document is not yet loaded.
            outTag = 0;
            return false;
        }

        CComBSTR tag;
        hr = htmlElement->get_tagName(&tag);
        ASSERT_EXIT(SUCCEEDED(hr), "htmlElement->get_tagName()");
        swprintf_s(outTag, 50, L"%s", tag.m_str);

        CComBSTR type(L"type");
        VARIANT attrvalue;
        VariantInit(&attrvalue);
        hr = htmlElement->getAttribute(type, 0 | 2, &attrvalue);
        ASSERT_EXIT(SUCCEEDED(hr), "htmlElement->getAttribute()");

        // Other way of doing this (found example in google):
        // hr = VariantChangeType(&attrvalue, &attrvalue, VARIANT_LOCALBOOL, VT_BSTR);
        // ASSERT_EXIT(SUCCEEDED(hr), "VariantChangeType()");
        // DEBUG_INT(attrvalue.vt); // (body)1==VT_NULL, (input)8==VT_BSTR
        // ASSERT_EXIT((attrvalue.vt == VT_BSTR), "attrvalue.vt != VT_BSTR");

        if (attrvalue.vt == VT_BSTR) {
            swprintf_s(outTypeAttr, 50, L"%s", attrvalue.bstrVal);
        } else {
            swprintf_s(outTypeAttr, 50, L"%s", L"");
        }

        return true;
    }

    int ExternalID(wchar_t* funcName)
    {
        return 0;
    }

    bool ExternalCall(int funcID)
    {
        return false;
    }

    void SetAllowedURL(wchar_t* inURL)
    {
        // Call this function in OnCreate().

        // Links that do not start with outURL will not be allowed to open in webbrowser control,
        // instead they will be opened in default browser by calling "system(start http://...)".

        // To disallow opening any urls in webbrowser control return here "nohttp" (this is the default).

        // To allow all urls

        // If your application is a local http server set it for example to "http://127.0.0.1:12345/".

        // If you need more control over allowed urls overwrite IsURLAllowed().

        wcsncpy_s(allowedURL, _countof(allowedURL), inURL, _TRUNCATE);
    }

    bool IsURLAllowed(wchar_t* inURL, int sizeInWords)
    {
        wchar_t* URL_lower = new wchar_t[sizeInWords];
        wcsncpy_s(URL_lower, sizeInWords, inURL, _TRUNCATE);
        _wcslwr_s(URL_lower, sizeInWords);

        bool ret = false;
        std::wstring URL = URL_lower;

        if (0 == wcscmp(allowedURL, L"nohttp")) {
            // Disallow: http://, https:// - case insensitive.
            if (0 == URL.compare(0, wcslen(L"http://"), L"http://")
                || 0 == URL.compare(0, wcslen(L"https://"), L"https://"))
            {
                ret = false;
            } else {
                ret = true;
            }
        } else {
            if (0 == URL.compare(0, wcslen(allowedURL), allowedURL)) {
                ret = true;
            } else {
                ret = false;
            }
        }

        delete[] URL_lower;

        return ret;
    }
};
