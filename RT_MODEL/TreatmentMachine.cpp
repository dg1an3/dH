// TreatmentMachine.cpp: implementation of the CTreatmentMachine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <UtilMacros.h>

#include <Matrix.h>

#include "TreatmentMachine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreatmentMachine::CTreatmentMachine()
	: m_SAD(700.0),
		m_SCD(300.0),
		m_SID(m_SAD * 2.0)
{
	m_projection = CreateProjection(m_SCD, m_SID);
}

CTreatmentMachine::~CTreatmentMachine()
{
}

IMPLEMENT_SERIAL(CTreatmentMachine, CObject, 1)

// machine identification
const CString& CTreatmentMachine::GetManufacturer() const
{
	return m_strManufacturer;
}

const CString& CTreatmentMachine::GetModel() const
{
	return m_strModel;	
}

const CString& CTreatmentMachine::GetSerialNumber() const
{
	return m_strSerialNumber;
}

// machine geometry parameters
double CTreatmentMachine::GetSAD() const
{
	return m_SAD;
}

double CTreatmentMachine::GetSCD() const
{
	return m_SCD;
}

double CTreatmentMachine::GetSID() const
{
	return m_SID;
}

// the projection matrix for the machine
const CMatrix<4>& CTreatmentMachine::GetProjection() const
{
	return m_projection;
}


void CTreatmentMachine::Serialize(CArchive &ar)
{
	SERIALIZE_VALUE(ar, m_strManufacturer);
	SERIALIZE_VALUE(ar, m_strModel);
	SERIALIZE_VALUE(ar, m_strSerialNumber);
	SERIALIZE_VALUE(ar, m_SAD);
	SERIALIZE_VALUE(ar, m_SCD);
	SERIALIZE_VALUE(ar, m_SID);
	if (ar.IsLoading())
	{
		m_projection = CreateProjection(m_SCD, m_SID);
	}
}
