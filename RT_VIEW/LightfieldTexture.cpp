// LightfieldTexture.cpp: implementation of the CLightfieldTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "LightfieldTexture.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// texture resolution (in pixels)
#define TEX_RESOLUTION 512

// maximum texture size (in mm)
#define TEX_SIZE 40.0

#define PEN_THICKNESS (TEX_RESOLUTION / 100)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLightfieldTexture::CLightfieldTexture()
: m_pBeam(NULL)
{

}

CLightfieldTexture::~CLightfieldTexture()
{

}

CBeam *CLightfieldTexture::GetBeam()
{
	return m_pBeam;
}

void CLightfieldTexture::SetBeam(CBeam *pBeam)
{
	if (NULL != GetBeam())
	{
		// remove this as an observer of surface changes
		::RemoveObserver<CLightfieldTexture>(&GetBeam()->GetChangeEvent(),
			this, OnBeamChanged);
	}

	// assign the pointer
	m_pBeam = pBeam;

	if (NULL != GetBeam())
	{
		// add this as an observer of surface changes
		::AddObserver<CLightfieldTexture>(&GetBeam()->GetChangeEvent(),
			this, OnBeamChanged);
	}

	// re-render
	OnBeamChanged(&GetBeam()->GetChangeEvent(), NULL);
}

// event handler for beam changes
void CLightfieldTexture::OnBeamChanged(CObservableEvent *pEvent, void *pOldValue)
{
	if (NULL == GetBeam())
	{
		return;
	}

	SetWidthHeight(TEX_RESOLUTION, TEX_RESOLUTION);

	CDC *pDC = GetDC();

	// default pen == thick green
	CPen pen(PS_SOLID, PEN_THICKNESS, RGB(0, 255, 0));
	CPen *pOldPen = pDC->SelectObject(&pen);

	// hollow brush
	CBrush *pOldBrush = (CBrush *)pDC->SelectStockObject(HOLLOW_BRUSH);

	CVectorD<2> vMin = GetBeam()->GetCollimMin();
	CVectorD<2> vMax = GetBeam()->GetCollimMax();

	vMin *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
	vMin += CVectorD<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

	vMax *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
	vMax += CVectorD<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

	pDC->Rectangle((int)vMin[0], (int)vMin[1], (int)vMax[0], (int)vMax[1]);

	CBrush brush(RGB(128, 128, 128));
	pDC->SelectObject(&brush);
	pDC->SetPolyFillMode(WINDING);
	for (int nAt = 0; nAt < GetBeam()->GetBlockCount(); nAt++)
	{
		CPolygon *pPolygon = GetBeam()->GetBlockAt(nAt);
		CArray<CPoint, CPoint&> arrBlock;
		arrBlock.SetSize(pPolygon->GetVertexCount());
		for (int nAtVert = 0; nAtVert < pPolygon->GetVertexCount(); nAtVert++)
		{
			CVectorD<2> v = pPolygon->GetVertex(nAtVert);
			v *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
			v += CVectorD<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

			arrBlock[nAtVert] = v;
		}

		pDC->Polygon(arrBlock.GetData(), pPolygon->GetVertexCount());
	}

	// draw the central axis
	CVectorD<2> vCenter(0.0, 0.0);
	vCenter *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
	vCenter += CVectorD<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

	pDC->MoveTo(vCenter - CVectorD<2>(8.0, 0.0));
	pDC->LineTo(vCenter + CVectorD<2>(8.0, 0.0));
	pDC->MoveTo(vCenter - CVectorD<2>(0.0, 8.0));
	pDC->LineTo(vCenter + CVectorD<2>(0.0, 8.0));

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	DrawBorder();

	ReleaseDC();

	// compute the texture projection
	CMatrixD<4> mTex;

	// scaling component for texture size
	mTex[0][0] = -1.0 / (TEX_SIZE + 1);
	mTex[1][1] = -1.0 / (TEX_SIZE + 1);
	mTex[3][0] = 0.5;
	mTex[3][1] = 0.5;

	// beam projection 
	mTex *= GetBeam()->GetTreatmentMachine()->GetProjection();

	// beam projection
	// glMultMatrix(m_pBeam->GetTreatmentMachine()->m_projection);

	// transform from patient -> beam coordinates
	CMatrixD<4> mB2PXform = GetBeam()->GetBeamToPatientXform();
	mB2PXform.Invert();
	mTex *= mB2PXform;

	// glMultMatrix(mXform);

	SetProjection(mTex);
}
