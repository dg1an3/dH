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

#include <MatrixD.h>

class CTreatmentMachine : public CModelObject  
{
public:
	// constructor/destructor
	CTreatmentMachine();
	virtual ~CTreatmentMachine();

	// serialization support
	DECLARE_SERIAL(CTreatmentMachine)

	// machine identification
	const CString& GetManufacturer() const;
	const CString& GetModel() const;
	const CString& GetSerialNumber() const;

	// machine geometry parameters
	double GetSAD() const;
	double GetSCD() const;
	double GetSID() const;

	// the projection matrix for the machine
	const CMatrixD<4>& GetProjection() const;

	// serialization
	void Serialize(CArchive &ar);

private:
	// machine identification
	CString m_strManufacturer;
	CString m_strModel;
	CString m_strSerialNumber;

	// machine geometry description
	double m_SAD;	// source-axis distance
	double m_SCD;	// source-collimator distance
	double m_SID;	// source-image distance

	// the machine's projection matrix
	CMatrixD<4> m_projection;

};

#endif // !defined(AFX_TREATMENTMACHINE_H__731877C5_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
