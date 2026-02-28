#pragma once
#include <Optimizer.h>

class CDFPOptimizer :
	public COptimizer
{
public:
	CDFPOptimizer(CObjectiveFunction *pFunc);
	~CDFPOptimizer(void);

	// function to actually perform the optimization
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

protected:
	void lnsrch(const CVectorN<>& xold, 
			const REAL fold, 
			const CVectorN<> &g, 
			CVectorN<>& p,
			CVectorN<>& x, 
			REAL &f, 
			const REAL stpmax, 
			bool &check);

	CMatrixNxM<> m_mHess_inv; 		
	CVectorN<> m_vG;
	CVectorN<> m_vXi;

	CVectorN<> m_vP_next;
	CVectorN<> m_vG_next;

	CVectorN<> m_vDeltaG;
	CVectorN<> m_vHDG;

	CMatrixNxM<> m_vDG_out_vDG;			
	CMatrixNxM<> m_vXi_out_vXi;			
	CMatrixNxM<> m_vHDG_out_vHDG;		

};
