//////////////////////////////////////////////////////////////////////
// ModelObject.h: interface for the CModelObject class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_)
#define AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"
#include "Value.h"
#include "Collection.h"

//////////////////////////////////////////////////////////////////////
// class CModelObject
// 
// a model object:
//		1) is observable
//		2) has a name
//		3) possibly has children
//		4) can be serialized
//////////////////////////////////////////////////////////////////////
class CModelObject : public CObservableObject
{
public:
	// constructors/destructors
	CModelObject(const CString& strName = "");
	virtual ~CModelObject();

	// makes the model object serializable
	DECLARE_SERIAL(CModelObject)

	// the given name for this model object
	CValue< CString > name;

	// collection of the children of this model object
	CCollection< CModelObject > children;

	// member function to add observers to all children (and grand-children, etc.)
	//	as well as firing changes on the children
	void AddObserverToChildren(CObject *pObserver, ChangeFunction func, 
		int nLevels = -1);
	void RemoveObserverFromChildren(CObject *pObserver, ChangeFunction func, 
		int nLevels = -1);
	void FireChangeChildren(void *pOldValue = NULL, 
		int nLevels = -1);

	// serialization
	virtual void Serialize( CArchive& ar );

	// handles changes from contained objects
	virtual void OnChange(CObservableObject *pSource, void *pOldValue);
};

#endif // !defined(AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_)
