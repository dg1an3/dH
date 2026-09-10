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

#include <functional>
#include <string>
#include <vector>

class CWebView2Host : public CWnd
{
public:
	CWebView2Host();
	virtual ~CWebView2Host();

	// map a virtual https host name to a local folder so large local assets
	//	(e.g. the vtk.js bundle) can be served with normal http(s) semantics that
	//	NavigateToString / file:// cannot provide. Must be set before Navigate;
	//	applied once the controller is ready. host e.g. L"planar.assets", folder
	//	an absolute path. Then Navigate(L"https://planar.assets/index.html").
	void SetVirtualHostMapping(LPCWSTR host, LPCWSTR folder);

	// navigate to a URL (e.g. a file:/// path or virtual host) -- queued if not ready
	void Navigate(LPCWSTR url);

	// render an inline, self-contained HTML string -- queued if not ready
	void NavigateToString(LPCWSTR html);

	// run JavaScript in the current page -- queued until the page has loaded
	void ExecScript(LPCWSTR js);

	// post a structured message to the page (window.chrome.webview 'message'
	//	event); json must be a valid JSON value. Queued until ready.
	void PostJson(LPCWSTR json);

	// register a handler for messages the page sends back via
	//	window.chrome.webview.postMessage(...). Invoked on the UI thread with the
	//	message serialized as a JSON string.
	void SetMessageHandler(std::function<void(const std::wstring&)> handler)
	{
		m_onMessage = std::move(handler);
	}

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
	void FlushPosts();

	Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller;
	Microsoft::WRL::ComPtr<ICoreWebView2> m_webview;
	EventRegistrationToken m_navCompletedToken;
	EventRegistrationToken m_webMessageToken;

	bool m_ready;		// controller + webview created
	bool m_navigated;	// a page has finished loading (scripts can run)

	// message handler for page -> host messages
	std::function<void(const std::wstring&)> m_onMessage;

	// virtual-host -> local-folder mapping, applied when the controller is ready
	std::wstring m_vhHost;
	std::wstring m_vhFolder;

	// requests queued until ready / navigated
	std::wstring m_pendingUrl;
	std::wstring m_pendingHtml;
	std::vector<std::wstring> m_pendingScripts;
	std::vector<std::wstring> m_pendingPosts;
};
