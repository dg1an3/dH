// ModelObject.cpp: implementation of the CModelObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ModelObject.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModelObject::CModelObject(const CString& strName)
	: name(strName)
{
	name.AddObserver(this, (ChangeFunction)&CModelObject::OnChange);
}

CModelObject::~CModelObject()
{

}

IMPLEMENT_SERIAL(CModelObject, CObservableObject, 1)

void CModelObject::AddObserverToChildren(CObject *pObserver, ChangeFunction func, int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->AddObserver((CObject *)pObserver, func);
		if (nLevels != 0)
			children.Get(nAt)->AddObserverToChildren(pObserver, func, nLevels-1);
	}
}

void CModelObject::RemoveObserverFromChildren(CObject *pObserver, ChangeFunction func, int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->RemoveObserver((CObject *)pObserver, func);
		if (nLevels != 0)
			children.Get(nAt)->RemoveObserverFromChildren(pObserver, func, nLevels-1);
	}
}

void CModelObject::FireChangeChildren(int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->FireChange();
		if (nLevels != 0)
			children.Get(nAt)->FireChangeChildren(nLevels-1);
	}
}

// serialization
void CModelObject::Serialize( CArchive& ar )
{
	CObject::Serialize(ar);

	name.Serialize(ar);

	children.Serialize(ar);
}

void CModelObject::OnChange(CObservableObject *pSource, void *pOldValue)
{
	FireChange();
}
