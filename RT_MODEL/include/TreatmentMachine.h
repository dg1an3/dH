// TreatmentMachine.h: interface for the CTreatmentMachine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREATMENTMACHINE_H__731877C5_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
#define AFX_TREATMENTMACHINE_H__731877C5_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include <Value.h>
#include <ModelObject.h>

#include <Matrix.h>

class CTreatmentMachine : public CModelObject  
{
public:
	CTreatmentMachine();
	virtual ~CTreatmentMachine();

	DECLARE_SERIAL(CTreatmentMachine)

	// machine identification
	CString m_strManufacturer;
	CString m_strModel;
	CString m_strSerialNumber;

	// machine parameters
	double m_SAD;	// source-axis distance
	double m_SCD;	// source-collimator distance
	double m_SID;	// source-image distance

	// the projection matrix for the machine
	CMatrix<4> m_projection;

	// serialization
	void Serialize(CArchive &ar);
};

#endif // !defined(AFX_TREATMENTMACHINE_H__731877C5_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
