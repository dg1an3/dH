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

void CTreatmentMachine::Serialize(CArchive &ar)
{
	SERIALIZE_VALUE(ar, m_strManufacturer);
	SERIALIZE_VALUE(ar, m_strModel);
	SERIALIZE_VALUE(ar, m_strSerialNumber);
	SERIALIZE_VALUE(ar, m_SAD);
	SERIALIZE_VALUE(ar, m_SCD);
	SERIALIZE_VALUE(ar, m_SID);
}
