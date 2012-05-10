//
// ServiceProvider.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//

#pragma once

#include <ServProv.h>
#include "InternetSecurityManager.h"
#include "HTMLOMWindowServices.h"
#include "WebBrowserFrameInterface.h"

template <class RootFrame>
class ServiceProvider : public IServiceProvider
{
public:
	InternetSecurityManager internetSecurityManager;
	HTMLOMWindowServices<RootFrame> htmlOMWindowServices;

	ServiceProvider(WebBrowserFrameInterface<RootFrame>* inwebframe)
		:
		htmlOMWindowServices(HTMLOMWindowServices<RootFrame>(inwebframe))
	{
	}

	// IServiceProvider

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		return E_NOTIMPL;
	}
	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 1;
	}
	ULONG STDMETHODCALLTYPE Release(void)
	{
		return 1;
	}
	HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
	{
		if (riid == IID_IInternetSecurityManager) {
			*ppvObject = &internetSecurityManager;
		} else if(riid == IID_IHTMLOMWindowServices) {
			*ppvObject = &htmlOMWindowServices;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
		return S_OK;
	}
};
