// ModelObject.h: interface for the CModelObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_)
#define AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_

// class CObserver;
#include "Observer.h"
#include "Value.h"
#include "Collection.h"

class CModelObject : public CObservableObject // , public CObserver
{
public:
	CModelObject(const CString& strName = "");
	virtual ~CModelObject();

	DECLARE_SERIAL(CModelObject)

	// the given name for this model object
	CValue< CString > name;

	// collection of the children of this model object
	CCollection< CModelObject > children;

	// member function to add observers to all children (and grand-children, etc.)
	void AddObserverToChildren(CObject *pObserver, ChangeFunction func, int nLevels = -1);
	void RemoveObserverFromChildren(CObject *pObserver, ChangeFunction func, int nLevels = -1);
	void FireChangeChildren(int nLevels = -1);

	// serialization
	virtual void Serialize( CArchive& ar );

	// handles changes from contained objects
	virtual void OnChange(CObservableObject *pSource, void *pOldValue);
};

#endif // !defined(AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_)
