// TreatmentMachine.cpp: implementation of the CTreatmentMachine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <MatrixFunction.h>

#include "TreatmentMachine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMatrix<4> ComputeProj(const double& n, const double& f)
{
	CMatrix<4> proj;

	proj[0][0] = n;

	proj[1][1] = n;

	proj[2][2] = -(f + n)	  / (f - n);
	proj[2][3] = -2.0 * f * n / (f - n);

	proj[3][2] = -1.0;
	proj[3][3] =  0.0;

	return proj;
}

FUNCTION_FACTORY2(ComputeProj, CMatrix<4>)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreatmentMachine::CTreatmentMachine()
	: SAD(400.0),
		SCD(240.0),
		SID(SAD.Get() * 4.0)
{
	projection.SyncTo(&ComputeProj(SCD, SID));
}

IMPLEMENT_SERIAL(CTreatmentMachine, CObject, 1)

CTreatmentMachine::~CTreatmentMachine()
{
}

void CTreatmentMachine::Serialize(CArchive &ar)
{
	manufacturer.Serialize(ar);
	model.Serialize(ar);
	serialNumber.Serialize(ar);
	SAD.Serialize(ar);
	SCD.Serialize(ar);
	SID.Serialize(ar);
}
