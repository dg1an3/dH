// Observer.h: interface for the CObserver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
#define AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CObservable;

class CObserver  
{
public:
	virtual void OnChange(CObservable *pFromObject) = 0;
};

class CObservable : public CObject
{
public:
	DECLARE_DYNAMIC(CObservable);

	void AddObserver(CObserver *pObserver) const;
	void RemoveObserver(CObserver *pObserver) const;

	void FireChange();

private:
	mutable CArray<CObserver *, CObserver *> m_arrObservers;
};

#endif // !defined(AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
