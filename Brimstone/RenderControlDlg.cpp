// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
//
// RenderControlDlg.cpp : implementation of CRenderControlDlg
//
#include "stdafx.h"
#include "Brimstone.h"
#include "BrimstoneDoc.h"
#include "BrimstoneView.h"
#include "RenderControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenderControlDlg

CRenderControlDlg::CRenderControlDlg(CBrimstoneView* pView, CWnd* pParent /*=NULL*/)
	: CDialog(CRenderControlDlg::IDD, pParent)
	, m_pView(pView)
{
}

void CRenderControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRenderControlDlg, CDialog)
	//{{AFX_MSG_MAP(CRenderControlDlg)
	ON_BN_CLICKED(IDC_RC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_RC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_RC_SLICE_PREV, OnSlicePrev)
	ON_BN_CLICKED(IDC_RC_SLICE_NEXT, OnSliceNext)
	ON_BN_CLICKED(IDC_RC_CAPTURE, OnCapture)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenderControlDlg message handlers

BOOL CRenderControlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// default capture prefix lives next to the executable's working dir
	SetDlgItemText(IDC_RC_CAPTURE_PATH, _T("brimstone_capture"));

	// seed the edits with the live render state
	RefreshFromView();

	SetStatus(_T("Ready."));

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// modeless: a click on the close box just hides the dialog so the owning view
// can re-show it. The view owns the object, so we never delete here.
void CRenderControlDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}

/////////////////////////////////////////////////////////////////////////////
bool CRenderControlDlg::GetDlgItemDouble(int nID, double& value)
{
	CString str;
	GetDlgItemText(nID, str);
	str.Trim();
	if (str.IsEmpty())
		return false;

	LPTSTR lpszEnd = NULL;
	double parsed = _tcstod(str, &lpszEnd);
	if (lpszEnd == (LPCTSTR) str)	// nothing consumed
		return false;

	value = parsed;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::SetStatus(LPCTSTR lpszText)
{
	SetDlgItemText(IDC_RC_STATUS, lpszText);
}

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::RefreshFromView()
{
	if (m_pView == NULL || !::IsWindow(m_hWnd))
		return;

	CPlanarView& view = m_pView->m_wndPlanarView;

	CString str;
	str.Format(_T("%.3f"), (double) view.m_window[0]);
	SetDlgItemText(IDC_RC_WINDOW, str);
	str.Format(_T("%.3f"), (double) view.m_level[0]);
	SetDlgItemText(IDC_RC_LEVEL, str);
	str.Format(_T("%.3f"), (double) view.m_alpha);
	SetDlgItemText(IDC_RC_ALPHA, str);
	str.Format(_T("%.4f"), (double) view.m_zoom);
	SetDlgItemText(IDC_RC_ZOOM, str);
	str.Format(_T("%.3f"), (double) view.m_vCenter[0]);
	SetDlgItemText(IDC_RC_CENTERX, str);
	str.Format(_T("%.3f"), (double) view.m_vCenter[1]);
	SetDlgItemText(IDC_RC_CENTERY, str);
	str.Format(_T("%.3f"), (double) view.m_vCenter[2]);
	SetDlgItemText(IDC_RC_SLICEZ, str);
}

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::OnRefresh()
{
	RefreshFromView();
	SetStatus(_T("Refreshed from view."));
}

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::OnApply()
{
	ApplyToView();
}

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::ApplyToView()
{
	if (m_pView == NULL)
		return;

	CPlanarView& view = m_pView->m_wndPlanarView;

	// window / level / alpha are simple field sets -- always safe
	double win = view.m_window[0], lev = view.m_level[0];
	GetDlgItemDouble(IDC_RC_WINDOW, win);
	GetDlgItemDouble(IDC_RC_LEVEL, lev);
	view.SetWindowLevel((REAL) win, (REAL) lev, 0);

	double alpha = view.m_alpha;
	if (GetDlgItemDouble(IDC_RC_ALPHA, alpha))
	{
		alpha = __max(0.0, __min(1.0, alpha));
		view.SetAlpha((REAL) alpha);
	}

	// zoom / pan need a loaded volume (the setters dereference m_pVolume[0])
	if (view.m_pVolume[0] != NULL)
	{
		Vector<REAL> vCenter = view.m_vCenter;
		double cx = vCenter[0], cy = vCenter[1], cz = vCenter[2];
		GetDlgItemDouble(IDC_RC_CENTERX, cx);
		GetDlgItemDouble(IDC_RC_CENTERY, cy);
		GetDlgItemDouble(IDC_RC_SLICEZ, cz);
		vCenter[0] = (REAL) cx;
		vCenter[1] = (REAL) cy;
		vCenter[2] = (REAL) cz;

		double zoom = view.m_zoom;
		if (GetDlgItemDouble(IDC_RC_ZOOM, zoom) && zoom > 0.0)
			view.SetZoom((REAL) zoom);

		// SetCenter re-derives the basis from the (possibly new) zoom
		view.SetCenter(vCenter);
	}

	// repaint the image and the graphs
	view.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	m_pView->m_graphDVH.Invalidate(TRUE);
	m_pView->m_graphIterations.Invalidate(TRUE);

	SetStatus(_T("Applied render parameters."));
}

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::StepSlice(int nSlices)
{
	if (m_pView == NULL)
		return;

	CPlanarView& view = m_pView->m_wndPlanarView;
	if (view.m_pVolume[0] == NULL)
	{
		SetStatus(_T("No volume loaded."));
		return;
	}

	REAL spacingZ = view.m_pVolume[0]->GetSpacing()[2];
	Vector<REAL> vCenter = view.m_vCenter;
	vCenter[2] += (REAL) nSlices * spacingZ;
	view.SetCenter(vCenter);
	view.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

	// reflect the new Z back into the edit so a reader sees the result
	CString str;
	str.Format(_T("%.3f"), (double) vCenter[2]);
	SetDlgItemText(IDC_RC_SLICEZ, str);

	SetStatus(_T("Stepped slice."));
}

void CRenderControlDlg::OnSlicePrev() { StepSlice(-1); }
void CRenderControlDlg::OnSliceNext() { StepSlice(+1); }

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::OnCapture()
{
	CaptureWindows();
}

/////////////////////////////////////////////////////////////////////////////
void CRenderControlDlg::CaptureWindows()
{
	if (m_pView == NULL)
		return;

	CString strPrefix;
	GetDlgItemText(IDC_RC_CAPTURE_PATH, strPrefix);
	strPrefix.Trim();
	if (strPrefix.IsEmpty())
		strPrefix = _T("brimstone_capture");

	struct { CWnd* pWnd; LPCTSTR lpszSuffix; } targets[] =
	{
		{ &m_pView->m_wndPlanarView,   _T("_planar.bmp") },
		{ &m_pView->m_graphDVH,        _T("_dvh.bmp")    },
		{ &m_pView->m_graphIterations, _T("_iter.bmp")   },
	};

	int nSaved = 0;
	CString strFirst;
	for (int nAt = 0; nAt < _countof(targets); nAt++)
	{
		if (targets[nAt].pWnd == NULL || !::IsWindow(targets[nAt].pWnd->GetSafeHwnd()))
			continue;

		CString strPath = strPrefix + targets[nAt].lpszSuffix;
		if (SaveWndToBmp(targets[nAt].pWnd, strPath))
		{
			if (nSaved == 0)
			{
				// resolve to a full path so the status line is unambiguous
				TCHAR szFull[MAX_PATH] = { 0 };
				if (::GetFullPathName(strPath, MAX_PATH, szFull, NULL))
					strFirst = szFull;
				else
					strFirst = strPath;
			}
			nSaved++;
		}
	}

	CString strStatus;
	if (nSaved > 0)
		strStatus.Format(_T("Saved %d window(s): %s ..."), nSaved, (LPCTSTR) strFirst);
	else
		strStatus = _T("Capture failed.");
	SetStatus(strStatus);
}

/////////////////////////////////////////////////////////////////////////////
// SaveWndToBmp - BitBlt a window's client area into a 24-bit DIB section and
// write it out as a standalone .bmp file (no MFC/CDib dependency so it works
// for any CWnd-derived graphical window).
bool CRenderControlDlg::SaveWndToBmp(CWnd* pWnd, LPCTSTR lpszPath)
{
	if (pWnd == NULL || !::IsWindow(pWnd->GetSafeHwnd()))
		return false;

	CRect rect;
	pWnd->GetClientRect(&rect);
	const int cx = rect.Width();
	const int cy = rect.Height();
	if (cx <= 0 || cy <= 0)
		return false;

	CDC* pWndDC = pWnd->GetDC();
	if (pWndDC == NULL)
		return false;

	CDC memDC;
	bool bOk = false;
	if (memDC.CreateCompatibleDC(pWndDC))
	{
		BITMAPINFO bmi;
		::ZeroMemory(&bmi, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = cx;
		bmi.bmiHeader.biHeight = cy;			// bottom-up DIB
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biCompression = BI_RGB;

		void* pBits = NULL;
		HBITMAP hDib = ::CreateDIBSection(pWndDC->GetSafeHdc(), &bmi,
			DIB_RGB_COLORS, &pBits, NULL, 0);
		if (hDib != NULL && pBits != NULL)
		{
			HGDIOBJ hOld = ::SelectObject(memDC.GetSafeHdc(), hDib);
			memDC.BitBlt(0, 0, cx, cy, pWndDC, 0, 0, SRCCOPY);
			::SelectObject(memDC.GetSafeHdc(), hOld);

			// each scanline of a DIB is DWORD-aligned
			const int nRowBytes = ((cx * 24 + 31) / 32) * 4;
			const DWORD dwImageSize = (DWORD) nRowBytes * cy;

			BITMAPFILEHEADER bfh;
			::ZeroMemory(&bfh, sizeof(bfh));
			bfh.bfType = 0x4D42;	// 'BM'
			bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
			bfh.bfSize = bfh.bfOffBits + dwImageSize;

			CFile file;
			if (file.Open(lpszPath, CFile::modeCreate | CFile::modeWrite))
			{
				try
				{
					file.Write(&bfh, sizeof(bfh));
					file.Write(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
					file.Write(pBits, dwImageSize);
					file.Close();
					bOk = true;
				}
				catch (CFileException* pEx)
				{
					pEx->Delete();
				}
			}
		}
		if (hDib != NULL)
			::DeleteObject(hDib);
	}

	pWnd->ReleaseDC(pWndDC);
	return bOk;
}
