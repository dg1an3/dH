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
typedef void (CObject::*ChangeFunction)(CObservableObject *, void *);


//////////////////////////////////////////////////////////////////////
// a CObservableObject fires change events that can be processed by a CObserver

class CObservableObject : public CObject
{
public:
	// includes dynamic type information
	DECLARE_DYNAMIC(CObservableObject)

	// accessors for the observer list
	void AddObserver(CObject *pObserver, ChangeFunction func) const;
	void RemoveObserver(CObject *pObserver, ChangeFunction func) const;

	// called to fire a change
	void FireChange(void *pOldValue = NULL);

private:
	// the array of observers
	mutable CArray<CObject *, CObject *> m_arrObservers;
	mutable CArray<ChangeFunction, ChangeFunction> m_arrFunctions;
};

template<class OBSERVER_TYPE>
void AddObserver(CObservableObject *pObservable, 
				 OBSERVER_TYPE *pObserver, ChangeFunction func);

template<class OBSERVER_TYPE>
void RemoveObserver(CObservableObject *pObservable, 
					OBSERVER_TYPE *pObserver, ChangeFunction func);

#endif // !defined(AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
