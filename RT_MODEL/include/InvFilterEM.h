// InvFilterEM.h: interface for the CInvFilterEM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INVFILTEREM_H__4134644D_4E19_4D12_8B53_7C3EBE0159D4__INCLUDED_)
#define AFX_INVFILTEREM_H__4134644D_4E19_4D12_8B53_7C3EBE0159D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ObjectiveFunction.h>
#include <MatrixNxM.h>

#include <vector>
using namespace std;

class CInvFilterEM : public CObjectiveFunction  
{
public:
	int GetSubOutputDim();
	CInvFilterEM();
	virtual ~CInvFilterEM();

	// sets up the desired subsampled output filter
	void SetFilterOutput(const CVectorN<>& vFiltOut);

	// gets the filter input for a given sub-output
	const CVectorN<>& GetFilterInput(const CVectorN<>& vSubOut) const;

	// gets the subsampled filter output from an input parameter
	void ParamToSubOutput(const CVectorN<>& vParam, CVectorN<>& vSubOutput) const;

	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const;

protected:
	void CalcPlanesAndCentroid();
	void CalcPlanes(const CMatrixNxM<>& mDecInvFilter, const CVectorN<>& vHG);
	void CalcVerts(const CMatrixNxM<>& mDecInvFilter, const CVectorN<>& vHG);
	void CalcCentroid(const CMatrixNxM<>& mDecInvFilter, const CVectorN<>& vHG);

public:
	CVectorN<> m_vFilterKernel;
	mutable CVectorN<> m_vFilterOut;
	mutable CVectorN<> m_vFilterIn;

	CMatrixNxM<> m_mFilter;
	CMatrixNxM<> m_mInvFilter;

	vector< CVectorN<> > m_arrPlaneNormals;
	vector< CVectorN<> > m_arrPlanePoints;

	vector< CVectorN<> > m_arrVerts;
	CVectorN<> m_vCentroid;
};

#endif // !defined(AFX_INVFILTEREM_H__4134644D_4E19_4D12_8B53_7C3EBE0159D4__INCLUDED_)
