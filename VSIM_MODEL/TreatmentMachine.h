// TreatmentMachine.h: interface for the CTreatmentMachine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREATMENTMACHINE_H__731877C5_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
#define AFX_TREATMENTMACHINE_H__731877C5_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Value.h>
#include <ModelObject.h>

#include <Matrix.h>

class CTreatmentMachine : public CModelObject  
{
public:
	CTreatmentMachine();

	DECLARE_SERIAL(CTreatmentMachine)

	virtual ~CTreatmentMachine();

	// machine identification
	CValue< CString > manufacturer;
	CValue< CString > model;
	CValue< CString > serialNumber;

	// machine parameters
	CValue< double > SAD;	// source-axis distance
	CValue< double > SCD;	// source-collimator distance
	CValue< double > SID;	// source-image distance

	// the projection matrix for the machine
	CValue< CMatrix<4> > projection;

	// serialization
	void Serialize(CArchive &ar);
};

#endif // !defined(AFX_TREATMENTMACHINE_H__731877C5_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
