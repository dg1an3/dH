// PlanarView.cpp : implementation file
//

#include "stdafx.h"
#include "brimstone.h"
#include "PlanarView.h"

#include <Series.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlanarView

CPlanarView::CPlanarView()
: m_bWindowLeveling(FALSE),
	m_pSeries(NULL)
{
	m_pVolume[0] = NULL;
	m_pVolume[1] = NULL;

	m_window[0] = (REAL) 400.0;
	m_window[1] = (REAL) 1.0 / 0.8;

	m_level[0] = (REAL) 30.0;
	m_level[1] = (REAL) 0.4;

	m_alpha = (REAL) 1.0;
}

CPlanarView::~CPlanarView()
{
}


BEGIN_MESSAGE_MAP(CPlanarView, CWnd)
	//{{AFX_MSG_MAP(CPlanarView)
	ON_WM_PAINT()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPlanarView message handlers

void CPlanarView::SetVolume(CVolume<VOXEL_REAL> *pVolume, int nVolumeAt)
{
	if (nVolumeAt == 1 && m_pVolume[1] == NULL)
	{
		m_alpha = 0.75;
	}

	m_pVolume[nVolumeAt] = pVolume;
}

void CPlanarView::SetBasis(const CMatrixD<4>& mBasis)
{
	m_volumeResamp[0].SetBasis(mBasis);
	m_volumeResamp[1].SetBasis(mBasis);
}

void CPlanarView::SetWindowLevel(REAL win, REAL cen, int nVolumeAt)
{
	m_window[nVolumeAt] = win;
	m_level[nVolumeAt] = cen;
}

void CPlanarView::SetAlpha(REAL alpha)
{
	m_alpha = alpha;
}

void CPlanarView::SetLUT(CArray<COLORREF, COLORREF>& arrLUT, int nVolumeAt)
{
	m_arrLUT[nVolumeAt].Copy(arrLUT);
}

void CPlanarView::AddStructure(CStructure *pStruct)
{
	m_arrStructures.Add(pStruct);
}

void CPlanarView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);

	DrawImages(&dc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	// selects image objects in to context
	dcMem.SelectObject(m_dib);

	dcMem.SelectStockObject(HOLLOW_BRUSH);
	dcMem.Rectangle(rect);

	// draw isocurves
	if (m_pVolume[1] != NULL && m_pVolume[1]->GetHeight() > 0)
	{
		VOXEL_REAL c = (VOXEL_REAL) 0.425;
		while (c < 1.0)
		{
			VOXEL_REAL max_value = (VOXEL_REAL) m_arrLUT[1].GetSize();
			VOXEL_REAL pix_value1 = (VOXEL_REAL)
				(max_value * m_window[1] * (c - m_level[1]) + max_value / 2.0);

			// scale to 0..255
			pix_value1 = (VOXEL_REAL) __min(pix_value1, max_value - 1.0);
			pix_value1 = (VOXEL_REAL) __max(pix_value1, 0.0);

			//  colorIndex = (int) pix_value1 // * (VOXEL_REAL) (max_value - 1.0));
			int colorIndex = Round<int>(__min(pix_value1, max_value - 1.0));

			CPen pen(PS_SOLID, 1, m_arrLUT[1][colorIndex]);
			dcMem.SelectObject(&pen);
			DrawIsocurves(m_pVolume[1], c, &dcMem);

			c += (VOXEL_REAL) 0.05;
		}
	}

	// draw contours
	DrawContours(&dcMem);

	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

	// Do not call CWnd::OnPaint() for painting messages
}

void CPlanarView::DrawImages(CDC *pDC)
{
	BOOL bDraw = FALSE;

	CRect rect;
	GetClientRect(&rect);

	VOXEL_REAL *pVoxels0 = NULL;
	if (m_pVolume[0] && m_pVolume[0]->GetWidth() != 0)
	{
		if (m_volumeResamp[0].GetWidth() != rect.Width())
		{
			m_volumeResamp[0].SetDimensions(rect.Width(), rect.Height(), 1);
			
			// FIX for actual width
			rect.right = rect.left + m_volumeResamp[0].GetWidth();
			ASSERT(rect.Width() == m_volumeResamp[0].GetWidth());
			ASSERT(rect.Width() % 4 == 0);
		}

		CMatrixD<4> mBasis = m_pVolume[0]->GetBasis();
		mBasis[0][0] = mBasis[1][1] = 0.5 * (REAL) m_pVolume[0]->GetHeight() / (REAL) rect.Height();
		mBasis[3][0] += 0.25 * (REAL) m_pVolume[0]->GetHeight();
		mBasis[3][1] += 0.25 * (REAL) m_pVolume[0]->GetHeight();
		m_volumeResamp[0].SetBasis(mBasis);

		m_volumeResamp[0].ClearVoxels();
		Resample(m_pVolume[0], &m_volumeResamp[0], TRUE);

		pVoxels0 = m_volumeResamp[0].GetVoxels()[0][0];

		bDraw = TRUE;
	}

	VOXEL_REAL *pVoxels1 = NULL; 
	if (m_pVolume[1] && m_pVolume[1]->GetHeight() != 0)
	{
		m_volumeResamp[1].ConformTo(&m_volumeResamp[0]);
/*		if (m_volumeResamp[1].GetWidth() != rect.Width())
		{
			m_volumeResamp[1].SetDimensions(rect.Width(), rect.Height(), 1);
		}
		m_volumeResamp[1].SetBasis(m_volumeResamp[0].GetBasis());  */

		m_volumeResamp[1].ClearVoxels();
		Resample(m_pVolume[1], &m_volumeResamp[1], TRUE);

		pVoxels1 = m_volumeResamp[1].GetVoxels()[0][0];
		bDraw = TRUE;
	}

	if (!bDraw)
		return;
	
	m_arrPixels.SetSize(rect.Width() * rect.Height());

	VOXEL_REAL alpha1 = (VOXEL_REAL) (1.0 - m_alpha);
	for (int nAt = 0; nAt < rect.Width() * rect.Height(); nAt++)
	{
		VOXEL_REAL pix_value0 = (VOXEL_REAL) 
			(128.0 / m_window[0] * (pVoxels0[nAt] - m_level[0]) + 128.0);

		// scale to 0..255
		pix_value0 = (VOXEL_REAL) __min(pix_value0, 255.0);
		pix_value0 = (VOXEL_REAL) __max(pix_value0, 0.0);

		if (!pVoxels1)
		{
			m_arrPixels[nAt] = 
				RGB(pix_value0, pix_value0, pix_value0);
		}
		else
		{
			VOXEL_REAL max_value = (VOXEL_REAL) m_arrLUT[1].GetSize();
			VOXEL_REAL pix_value1 = (VOXEL_REAL) 
				(max_value * m_window[1] * (pVoxels1[nAt] - m_level[1]) + max_value / 2.0);

			// scale to 0..255
			pix_value1 = (VOXEL_REAL) __min(pix_value1, max_value - 1.0);
			pix_value1 = (VOXEL_REAL) __max(pix_value1, 0.0);

			//  colorIndex = (int) pix_value1 // * (VOXEL_REAL) (max_value - 1.0));
			int colorIndex = Round<int>(__min(pix_value1, max_value - 1.0));
			COLORREF color1 = m_arrLUT[1][colorIndex];

			if (colorIndex > 5)
			{
				m_arrPixels[nAt] = 
					RGB(
						m_alpha * pix_value0 + alpha1 * GetBValue(color1), 
						m_alpha * pix_value0 + alpha1 * GetGValue(color1),
						m_alpha * pix_value0 + alpha1 * GetRValue(color1)
						);
			}
			else
			{
				m_arrPixels[nAt] = 
					RGB(pix_value0, pix_value0, pix_value0);
			}
		}
	}

	
	if (m_dib.GetSize().cx != rect.Width())
	{
		m_dib.DeleteObject();
		HBITMAP bm = ::CreateCompatibleBitmap(*pDC, rect.Width(), rect.Height());
		m_dib.Attach(bm);
	}
	DWORD dwCount = m_dib.SetBitmapBits(rect.Width() * rect.Height() * sizeof(COLORREF),
		(void *) m_arrPixels.GetData());

	// PLDrawBitmap(*pDC, &m_dib, NULL, NULL, SRCCOPY);
}



void CPlanarView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_bWindowLeveling = TRUE;
	m_ptWinLevStart = point;
	m_windowStart = m_window[0];
	m_levelStart = m_level[0];
	
	SetCapture();

	CWnd::OnMButtonDown(nFlags, point);
}

void CPlanarView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	m_bWindowLeveling = FALSE;
	::ReleaseCapture();

	CWnd::OnMButtonUp(nFlags, point);
}

void CPlanarView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bWindowLeveling)
	{
		CRect rectClient;
		GetClientRect(&rectClient);

		VOXEL_REAL voxelMax = m_pVolume[0]->GetMax();

		CPoint ptDelta = point - m_ptWinLevStart;

		m_window[0] = m_windowStart - voxelMax * (VOXEL_REAL) ptDelta.y / (VOXEL_REAL) rectClient.Height();
		m_window[0] = __max(m_window[0], 0.001);

		m_level[0] = m_levelStart - voxelMax * (VOXEL_REAL) ptDelta.x / (VOXEL_REAL) rectClient.Width();

		// Invalidate(FALSE);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

CPoint ToDC(const CVectorD<2>& vVert, const CMatrixD<4>& mBasis)
{
	CPoint pt;
		// (
		// vVert[0] / (VOXEL_REAL) pDens->GetWidth() * (VOXEL_REAL) rect.Width(),
		// vVert[1] / (VOXEL_REAL) pDens->GetHeight() * (VOXEL_REAL) rect.Height());
	// pt += rect.TopLeft();
	// TODO: use round function
	pt.x = Round<LONG>((vVert[0] - mBasis[3][0]) / mBasis[0][0]);
	pt.y = Round<LONG>((vVert[1] - mBasis[3][1]) / mBasis[1][1]);

	return pt;
		
}

void CPlanarView::DrawContours(CDC *pDC)
{
	if (!m_pSeries)
		return;

	const CMatrixD<4>& mBasis = m_volumeResamp[0].GetBasis();
	REAL slicePos = mBasis[3][2]; // (mBasis * ToHG(CVectorD<3>(0.0, 0.0, 0.0)))[2];

	for (int nAtStruct = 0; nAtStruct < m_pSeries->GetStructureCount(); nAtStruct++)
	{
		CStructure *pStruct = m_pSeries->GetStructureAt(nAtStruct);

		if (!pStruct->IsVisible())
		{
			continue;
		}

		CPen pen(PS_SOLID, 2, pStruct->GetColor());
		CPen *pOldPen = pDC->SelectObject(&pen);
		pDC->SelectStockObject(HOLLOW_BRUSH);

		for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
		{
			REAL zPos = pStruct->GetContourRefDist(nAtContour);
			if (IsApproxEqual(zPos, slicePos))
			{
				CPolygon *pPoly = pStruct->GetContour(nAtContour);

				pDC->MoveTo(ToDC(pPoly->GetVertexAt(0), mBasis));
				for (int nAtVert = 1; nAtVert < pPoly->GetVertexCount(); nAtVert++)
				{
					pDC->LineTo(ToDC(pPoly->GetVertexAt(nAtVert), mBasis));
				}
				pDC->LineTo(ToDC(pPoly->GetVertexAt(0), mBasis));
			}
		}

		pDC->SelectObject(pOldPen);
	}

}



double calcPoint(double bottom, double top, double c, double tempstep)
{             
	return ((c-bottom)/(top-bottom))*tempstep; 
}


void CPlanarView::DrawIsocurves(CVolume<VOXEL_REAL> *pVolume, REAL c, CDC *pDC)
{
	const CMatrixD<4>& mBasis = m_volumeResamp[0].GetBasis();

	double topleft = 0;
	double topright = 0;
	double bottomleft = 0;
	double bottomright = 0;
	
	VOXEL_REAL **ppPixels = pVolume->GetVoxels()[0];

	CVectorD<3> vPixSpacing = pVolume->GetPixelSpacing();
	CVectorD<3> vOrigin = FromHG<3, REAL>(pVolume->GetBasis()[3]);

	for(int nY = 0; nY < pVolume->GetHeight()-1; nY++)
	{          
		for(int nX = 0; nX < pVolume->GetWidth()-1; nX++)
		{           
			if(nX == 0)
			{
				topleft = ppPixels[nY+1][nX];
				topright = ppPixels[nY+1][nX+1];
				bottomleft = ppPixels[nY][nX];
				bottomright = ppPixels[nY][nX+1];
			}
			else
			{
				topleft = topright;
				topright = ppPixels[nY+1][nX+1];
				bottomleft = bottomright;
				bottomright = ppPixels[nY][nX+1]; 
			}         
			
			//case 1.1
			if((topleft<c && topright<c && bottomleft>c && bottomright>c) 
				|| (topleft>c && topright>c && bottomleft<c && bottomright<c))
			{
				// points are divided down some line in the middle
				double left = calcPoint(bottomleft, topleft, c, vPixSpacing[1]);
				double right = calcPoint(bottomright, topright, c, vPixSpacing[1]);
					
				pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX, 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY + left), mBasis));
				pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) (nX+1), 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY + right), mBasis));
			} 

			//case 1.2 
			else if((topleft<c && topright>c && bottomleft<c && bottomright>c)
				|| (topleft>c && topright<c && bottomleft>c && bottomright<c))
			{
				// points are divided down some line in the middle
				double bottom = calcPoint(bottomleft, bottomright, c, vPixSpacing[0]);    
				double top = calcPoint(topleft, topright, c, vPixSpacing[0]);
				
				pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + bottom, 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY), mBasis));
				pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + top, 
					vOrigin[1] + vPixSpacing[1] * (REAL) (nY + 1)), mBasis));
			} 
			
			//case 2              
			else if((topleft<c && topright<c && bottomleft<c && bottomright<c)
				|| (topleft>c && topright>c && bottomleft>c && bottomright>c)) 
			{
				// all points below or above the line, so dont do anything
			}  
			//case 3.1              
			else if((topleft>c && topright>c && bottomleft>c && bottomright<c)
				||(topleft<c && topright<c && bottomleft<c && bottomright>c))
			{
				// three points above or below the line and the other the opposite
				double right = calcPoint(bottomright, topright, c, vPixSpacing[1]);
				double bottom = calcPoint(bottomleft, bottomright, c, vPixSpacing[0]);
				
				pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + bottom, 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY), mBasis));
				pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) (nX+1), 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY + right), mBasis));

				// linevector.add(new Point3f((float)(i+bottom),(float)j,(float)(k)));
				// linevector.add(new Point3f((float)(i+vPixSpacing[0]),(float)(j+right), (float)(k)));
			}  
			//case 3.2              
			else if((topleft>c && topright>c && bottomleft<c && bottomright>c)
				||(topleft<c && topright<c && bottomleft>c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double left = calcPoint(bottomleft, topleft, c, vPixSpacing[1]);
				double bottom = calcPoint(bottomleft, bottomright, c, vPixSpacing[0]);
				
				pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + bottom, 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY), mBasis));
				pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX, 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY + left), mBasis));

				// linevector.add(new Point3f((float)(i+bottom),(float)j,(float)(k)));
				// linevector.add(new Point3f((float)(i),(float)(j+left), (float)(k)));
			}  
			//case 3.3             
			else if((topleft<c && topright>c && bottomleft>c && bottomright>c)
				||(topleft>c && topright<c && bottomleft<c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double left = calcPoint(bottomleft, topleft, c, vPixSpacing[1]);
				double top = calcPoint(topleft, topright, c, vPixSpacing[0]);
				
				pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX, 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY + left), mBasis));
				pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + top, 
					vOrigin[1] + vPixSpacing[1] * (REAL) (nY+1)), mBasis));

				// linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
				// linevector.add(new Point3f((float)(i+top),(float)(j+vPixSpacing[1]), (float)(k)));
			}  
			//case 3.4              
			else if((topleft>c && topright<c && bottomleft>c && bottomright>c)
				||(topleft<c && topright>c && bottomleft<c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double top = calcPoint(topleft, topright, c, vPixSpacing[0]);
				double right = calcPoint(bottomright, topright, c, vPixSpacing[1]);
				
				pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + top, 
					vOrigin[1] + vPixSpacing[1] * (REAL) (nY+1) ), mBasis));
				pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) (nX+1), 
					vOrigin[1] + vPixSpacing[1] * (REAL) nY + right), mBasis));

				// linevector.add(new Point3f((float)(i+top),(float)(j+vPixSpacing[1]),(float)(k)));
				// linevector.add(new Point3f((float)(i+vPixSpacing[0]),(float)(j+right), (float)(k)));
			}  
			
			//case 4.1              
			else if(topleft>c && topright<c && bottomleft<c && bottomright>c)
			{
				//Ambigous case where the corners are diametrically opposed
				//Test in the middle and decide which way the lines go..
				double middle = 0.0; // SFS.val(new Vector3d((double)(i+(vPixSpacing[0]/2)), (double)(j+(stepj/2)), 0.0));
				
				double top = calcPoint(topleft, topright, c, vPixSpacing[0]);
				double bottom = calcPoint(bottomleft, bottomright, c, vPixSpacing[0]);
				double left = calcPoint(bottomleft, topleft, c, vPixSpacing[1]);
				double right = calcPoint(bottomright, topright, c, vPixSpacing[1]);
				
				if(middle < c)
				{
					pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX, 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY + left), mBasis));
					pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + top, 
						vOrigin[1] + vPixSpacing[1] * (REAL) (nY+1)), mBasis));

					// linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					// linevector.add(new Point3f((float)(i+top),(float)(j+stepj), (float)(k)));
					
					pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + bottom, 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY), mBasis));
					pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) (nX+1), 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY + right), mBasis));

					// linevector.add(new Point3f((float)(i+bottom),(float)(j),(float)(k)));
					// linevector.add(new Point3f((float)(i+vPixSpacing[0]),(float)(j+right), (float)(k)));  
				}
				else if(middle > c)
				{
					pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX, 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY + left), mBasis));
					pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + bottom, 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY), mBasis));

					// linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					// linevector.add(new Point3f((float)(i+bottom),(float)(j),(float)(k)));
					
					pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + top, 
						vOrigin[1] + vPixSpacing[1] * (REAL) (nY+1)), mBasis));
					pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) (nX+1), 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY + right), mBasis));

					// linevector.add(new Point3f((float)(i+top),(float)(j+stepj), (float)(k)));
					// linevector.add(new Point3f((float)(i+vPixSpacing[0]),(float)(j+right), (float)(k)));
				}
				
			}
			//case 4.2
			else if(topleft<c && topright>c && bottomleft>c && bottomright<c)
			{
				//Ambigous case where the corners are diametrically opposed
				//Test in the middle and decide which way the lines go..
				double middle = 0.0; // SFS.val(new Vector3d((double)(i+(vPixSpacing[0]/2)), (double)(j+(stepj/2)), 0.0));
				double top = calcPoint(topleft, topright, c, vPixSpacing[0]);
				double bottom = calcPoint(bottomleft, bottomright, c, vPixSpacing[0]);
				double left = calcPoint(bottomleft, topleft, c, vPixSpacing[1]);
				double right = calcPoint(bottomright, topright, c, vPixSpacing[1]);
				
				if(middle<c)
				{
					pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX, 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY + left), mBasis));
					pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + bottom, 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY), mBasis));

					// linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					// linevector.add(new Point3f((float)(i+bottom),(float)(j), (float)(k)));
					
					pDC->MoveTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) nX + top, 
						vOrigin[1] + vPixSpacing[1] * (REAL) (nY+1)), mBasis));
					pDC->LineTo(ToDC(CVectorD<2>(vOrigin[0] + vPixSpacing[0] * (REAL) (nX+1), 
						vOrigin[1] + vPixSpacing[1] * (REAL) nY + right), mBasis));

					// linevector.add(new Point3f((float)(i+top),(float)(j+stepj),(float)(k)));
					// linevector.add(new Point3f((float)(i+vPixSpacing[0]),(float)(j+right), (float)(k)));   
				}
				else if(middle>c)
				{
				/*	linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					linevector.add(new Point3f((float)(i+top),(float)(j+stepj),(float)(k)));
					
					linevector.add(new Point3f((float)(i+bottom),(float)(j), (float)(k)));
					linevector.add(new Point3f((float)(i+vPixSpacing[0]),(float)(j+right), (float)(k)));   */
				} 
				
			}
			
			// nX++; 
    }//close i-loop
	
	// i = imin;
	// nY++;
	
 }//close j-loop
 
 // i = imin;
 // j = jmin;
 // myzcounter++;
 
} // close k-loop


