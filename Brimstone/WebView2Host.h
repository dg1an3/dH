// WebView2Host.h
//
// A CWnd that hosts a Microsoft Edge WebView2 control. WebView2 initialization
// is asynchronous (COM completion callbacks pumped on the UI thread), so any
// Navigate / NavigateToString / ExecScript request made before the control is
// ready is queued and flushed once it is.
//
// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
#pragma once

#include <wrl.h>
#include <WebView2.h>

#include <string>
#include <vector>

class CWebView2Host : public CWnd
{
public:
	CWebView2Host();
	virtual ~CWebView2Host();

	// navigate to a URL (e.g. a file:/// path or virtual host) -- queued if not ready
	void Navigate(LPCWSTR url);

	// render an inline, self-contained HTML string -- queued if not ready
	void NavigateToString(LPCWSTR html);

	// run JavaScript in the current page -- queued until the page has loaded
	void ExecScript(LPCWSTR js);

	// true once the WebView2 controller is created and a page has finished loading
	bool IsReady() const { return m_ready; }
	bool IsNavigated() const { return m_navigated; }

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	void InitEnvironment();
	void OnControllerReady(ICoreWebView2Controller* controller);
	void ResizeToClient();
	void FlushNavigate();
	void FlushScripts();

	Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller;
	Microsoft::WRL::ComPtr<ICoreWebView2> m_webview;
	EventRegistrationToken m_navCompletedToken;

	bool m_ready;		// controller + webview created
	bool m_navigated;	// a page has finished loading (scripts can run)

	// requests queued until ready / navigated
	std::wstring m_pendingUrl;
	std::wstring m_pendingHtml;
	std::vector<std::wstring> m_pendingScripts;
};
