// Observer.h: interface for the CObserver interface and CObservable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
#define AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// forward declaration of the CObservableObject class
class CObservableObject;


//////////////////////////////////////////////////////////////////////
// the CObserver interface must be implemented by any object that wants to observe
//		a CObservableObject

class CObserver  
{
public:
	// the call-back for changes on a CObservableObject
	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue) = 0;
};


//////////////////////////////////////////////////////////////////////
// a CObservableObject fires change events that can be processed by a CObserver

class CObservableObject : public CObject
{
public:
	// includes dynamic type information
	DECLARE_DYNAMIC(CObservableObject)

	// accessors for the observer list
	void AddObserver(CObserver *pObserver) const;
	void RemoveObserver(CObserver *pObserver) const;

	// called to fire a change
	void FireChange(void *pOldValue = NULL);

private:
	// the array of observers
	mutable CArray<CObserver *, CObserver *> m_arrObservers;
};

#endif // !defined(AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
