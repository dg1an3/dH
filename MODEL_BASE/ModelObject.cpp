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
	name.AddObserver(this);
}

CModelObject::~CModelObject()
{

}

IMPLEMENT_SERIAL(CModelObject, CObservableObject, 1)

void CModelObject::AddObserverToChildren(CObserver *pObserver, int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->AddObserver(pObserver);
		if (nLevels != 0)
			children.Get(nAt)->AddObserverToChildren(pObserver, nLevels-1);
	}
}

void CModelObject::RemoveObserverFromChildren(CObserver *pObserver, int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->RemoveObserver(pObserver);
		if (nLevels != 0)
			children.Get(nAt)->RemoveObserverFromChildren(pObserver, nLevels-1);
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
