// WebView2Host.cpp
//
// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
#include "stdafx.h"
#include "WebView2Host.h"

#include <wrl/event.h>

// The WebView2 loader import lib. vcpkg's user-wide MSBuild integration puts
// C:\vcpkg\installed\x64-windows\lib on the library search path and applocal-
// deploys WebView2Loader.dll to the output dir, so no vcxproj/lib changes are
// needed beyond this auto-link.
#pragma comment(lib, "WebView2Loader.dll.lib")

using namespace Microsoft::WRL;

CWebView2Host::CWebView2Host()
	: m_ready(false)
	, m_navigated(false)
	, m_navCompletedToken()
	, m_webMessageToken()
{
}

CWebView2Host::~CWebView2Host()
{
	if (m_controller)
		m_controller->Close();
}

BEGIN_MESSAGE_MAP(CWebView2Host, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CWebView2Host::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	InitEnvironment();
	return 0;
}

void CWebView2Host::InitEnvironment()
{
	// WebView2 needs a writable user-data folder for the runtime; keep it out
	//	of the exe directory (which may be read-only) by using the temp dir.
	wchar_t tempPath[MAX_PATH] = { 0 };
	GetTempPathW(MAX_PATH, tempPath);
	std::wstring userDataFolder = std::wstring(tempPath) + L"BrimstoneWebView2";

	const HWND hwndHost = m_hWnd;

	CreateCoreWebView2EnvironmentWithOptions(
		nullptr, userDataFolder.c_str(), nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this, hwndHost](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
			{
				if (FAILED(result) || env == nullptr)
					return result;

				env->CreateCoreWebView2Controller(
					hwndHost,
					Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
						[this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
						{
							if (FAILED(result) || controller == nullptr)
								return result;

							OnControllerReady(controller);
							return S_OK;
						}).Get());
				return S_OK;
			}).Get());
}

void CWebView2Host::OnControllerReady(ICoreWebView2Controller* controller)
{
	m_controller = controller;
	m_controller->get_CoreWebView2(&m_webview);
	m_controller->put_IsVisible(TRUE);

	// track navigation completion so queued scripts/messages only reach a live page
	if (m_webview)
	{
		m_webview->add_NavigationCompleted(
			Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[this](ICoreWebView2* /*sender*/, ICoreWebView2NavigationCompletedEventArgs* /*args*/) -> HRESULT
				{
					m_navigated = true;
					FlushScripts();
					FlushPosts();
					return S_OK;
				}).Get(),
			&m_navCompletedToken);

		// page -> host messages (window.chrome.webview.postMessage). Fired on the
		//	UI thread; forwarded to the registered handler as a JSON string.
		m_webview->add_WebMessageReceived(
			Callback<ICoreWebView2WebMessageReceivedEventHandler>(
				[this](ICoreWebView2* /*sender*/, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
				{
					if (m_onMessage && args)
					{
						// the page posts plain strings (postMessage("cmd|a|b"));
						//	fall back to the JSON form for non-string messages.
						LPWSTR msg = nullptr;
						if (SUCCEEDED(args->TryGetWebMessageAsString(&msg)) && msg)
						{
							m_onMessage(std::wstring(msg));
							CoTaskMemFree(msg);
						}
						else if (SUCCEEDED(args->get_WebMessageAsJson(&msg)) && msg)
						{
							m_onMessage(std::wstring(msg));
							CoTaskMemFree(msg);
						}
					}
					return S_OK;
				}).Get(),
			&m_webMessageToken);
	}

	m_ready = true;
	ResizeToClient();
	FlushNavigate();
}

void CWebView2Host::ResizeToClient()
{
	if (!m_controller || m_hWnd == NULL)
		return;

	CRect rc;
	GetClientRect(&rc);
	RECT bounds = { 0, 0, rc.Width(), rc.Height() };
	m_controller->put_Bounds(bounds);
}

void CWebView2Host::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	ResizeToClient();
}

void CWebView2Host::Navigate(LPCWSTR url)
{
	if (m_ready && m_webview)
		m_webview->Navigate(url);
	else
	{
		m_pendingUrl = url;
		m_pendingHtml.clear();
	}
}

void CWebView2Host::NavigateToString(LPCWSTR html)
{
	if (m_ready && m_webview)
		m_webview->NavigateToString(html);
	else
	{
		m_pendingHtml = html;
		m_pendingUrl.clear();
	}
}

void CWebView2Host::ExecScript(LPCWSTR js)
{
	if (m_ready && m_navigated && m_webview)
		m_webview->ExecuteScript(js, nullptr);
	else
		m_pendingScripts.push_back(js);
}

void CWebView2Host::PostJson(LPCWSTR json)
{
	if (m_ready && m_navigated && m_webview)
		m_webview->PostWebMessageAsJson(json);
	else
		m_pendingPosts.push_back(json);
}

void CWebView2Host::FlushNavigate()
{
	if (!m_webview)
		return;

	if (!m_pendingHtml.empty())
	{
		m_webview->NavigateToString(m_pendingHtml.c_str());
		m_pendingHtml.clear();
	}
	else if (!m_pendingUrl.empty())
	{
		m_webview->Navigate(m_pendingUrl.c_str());
		m_pendingUrl.clear();
	}
}

void CWebView2Host::FlushScripts()
{
	if (!m_webview)
		return;

	for (const std::wstring& s : m_pendingScripts)
		m_webview->ExecuteScript(s.c_str(), nullptr);
	m_pendingScripts.clear();
}

void CWebView2Host::FlushPosts()
{
	if (!m_webview)
		return;

	for (const std::wstring& s : m_pendingPosts)
		m_webview->PostWebMessageAsJson(s.c_str());
	m_pendingPosts.clear();
}
