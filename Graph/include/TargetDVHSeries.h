#pragma once
#include <DataSeries.h>
#include <KLDivTerm.h>

class CTargetDVHSeries :
	public CDataSeries
{
public:
	CTargetDVHSeries(CKLDivTerm *pKLDT);
	virtual ~CTargetDVHSeries(void);

	virtual void SetDataMatrix(const CMatrixNxM<>& mData);

	// stores pointer to term
	CKLDivTerm *m_pKLDivTerm;
	void OnKLDTChanged(CObservableEvent * pEv, void * pVoid);
};
