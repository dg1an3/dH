//////////////////////////////////////////////////////////////////////
// ModelObject.cpp: implementation of the CModelObject class.
//
// Copyright (C) 1999-2001 DGLane
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the main include for the class
#include "ModelObject.h"

// debug new statement
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CModelObject::CModelObject
// 
// constructs a model object with the given name
//////////////////////////////////////////////////////////////////////
CModelObject::CModelObject(const CString& strName)
	: name(strName)
{
	::AddObserver<CModelObject>(&name, this, OnChange);
}

//////////////////////////////////////////////////////////////////////
// CModelObject::~CModelObject
// 
// destroys the model objects
//////////////////////////////////////////////////////////////////////
CModelObject::~CModelObject()
{

}

//////////////////////////////////////////////////////////////////////
// declares CModelObject as a serializable class
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CModelObject, CObservableObject, 1)

//////////////////////////////////////////////////////////////////////
// CModelObject::AddObserverToChildren
// 
// adds an observer to all of the children, grandchildren etc. to the
// number of specified lelvels (or until the bottom is reached, if
// the number of levels is -1)
//////////////////////////////////////////////////////////////////////
void CModelObject::AddObserverToChildren(CObject *pObserver, 
		ChangeFunction func, int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->AddObserver(pObserver, func);
		if (nLevels != 0)
		{
			children.Get(nAt)->AddObserverToChildren(pObserver, func, nLevels-1);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CModelObject::RemoveObserverFromChildren
// 
// removes an observer from all of the children, grandchildren etc. to 
// the number of specified lelvels (or until the bottom is reached, if
// the number of levels is -1)
//////////////////////////////////////////////////////////////////////
void CModelObject::RemoveObserverFromChildren(CObject *pObserver, 
		ChangeFunction func, int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->RemoveObserver(pObserver, func);
		if (nLevels != 0)
		{
			children.Get(nAt)->RemoveObserverFromChildren(pObserver, func, nLevels-1);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CModelObject::FireChangeChildren
// 
// fires a change on all of the children
//////////////////////////////////////////////////////////////////////
void CModelObject::FireChangeChildren(void *pOldValue, int nLevels)
{
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		children.Get(nAt)->FireChange(pOldValue);
		if (nLevels != 0)
		{
			children.Get(nAt)->FireChangeChildren(pOldValue, nLevels-1);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CModelObject::Serialize
// 
// serialization
//////////////////////////////////////////////////////////////////////
void CModelObject::Serialize( CArchive& ar )
{
	// serialize the base class
	CObject::Serialize(ar);

	// serialize the object's name
	name.Serialize(ar);

	// serialize the children
	children.Serialize(ar);
}

//////////////////////////////////////////////////////////////////////
// CModelObject::OnChange
// 
// handles changes by propagating to listeners
//////////////////////////////////////////////////////////////////////
void CModelObject::OnChange(CObservableObject *pSource, void *pOldValue)
{
	FireChange();
}
