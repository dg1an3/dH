//////////////////////////////////////////////////////////////////////
// TPSTransform.h: interface for the CTPSTransform class.
//
// Copyright (C) 2002
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TPSTRANSFORM_H__AD843B49_3C65_4573_B2D5_9C258B29969B__INCLUDED_)
#define AFX_TPSTRANSFORM_H__AD843B49_3C65_4573_B2D5_9C258B29969B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _FIELDCOM
#include "FieldCOM.h"
#endif

// container vector for the landmarks
#include <vector>
using namespace std;

// vector includes
#include <VectorD.h>
#include <VectorN.h>

// model object base class
#include <ModelObject.h>

// Dib for resampling
#include <Dib.h>

//////////////////////////////////////////////////////////////////////
// class CTPSTransform
// 
// represents a basic TPS transform.  holds a collection of position
//		vector pairs and interpolates the field from those
//////////////////////////////////////////////////////////////////////
class CTPSTransform : public CModelObject
#ifdef _FIELDCOM
		, public IField  
#endif
{
public:
	// construction/destruction
	CTPSTransform();
	virtual ~CTPSTransform();

	// landmark accessors
	int GetLandmarkCount();
	const CVectorD<3>& GetLandmark(int nDataSet, int nIndex);
	void SetLandmark(int nDataSet, int nIndex, const CVectorD<3>& vLandmark);

	// functions to add landmark points
	int AddLandmark(const CVectorD<3>& vLandmark);
	int AddLandmark(const CVectorD<3>& vLandmark1, 
		const CVectorD<3>& vLandmark2);

	// removes all landmarks from the transform
	void RemoveAllLandmarks();

	// evaluates the field at a point
	void Eval(const CVectorD<3>& vPos, CVectorD<3>& vPosTrans);

	// resamples an image based on the field
	void Resample(CDib *pImageSrc, CDib *pImageDst);

protected:
	// recalculates the TPS from the landmarks
	void RecalcWeights();

private:
	// the array of landmarks
	vector< CVectorD<3> > m_arrLandmarks[2];

	// the final weight vectors
	CVectorN<> m_vWx;
	CVectorN<> m_vWy;

	// flag to indicate that recalculation of the TPS is needed
	BOOL m_bRecalc;
};

#endif // !defined(AFX_TPSTRANSFORM_H__AD843B49_3C65_4573_B2D5_9C258B29969B__INCLUDED_)
