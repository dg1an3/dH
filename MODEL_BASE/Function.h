//////////////////////////////////////////////////////////////////////
// Function.h: interface for the CFunction class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FUNCTION_H__9ADD8ED4_FC5D_11D4_9E43_00B0D0609AB0__INCLUDED_)
#define AFX_FUNCTION_H__9ADD8ED4_FC5D_11D4_9E43_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Value.h"


//////////////////////////////////////////////////////////////////////
// class CFunction1<TYPE, ARG_TYPE>
// 
// a function object that accepts an ARG_TYPE argument and is a 
// CValue<TYPE>.
//
// the argument can be synced to another CValue<ARG_TYPE> to implement
// functional pipelines.
//////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
class CFunction1 : public CValue<TYPE>  
{
public:
	CFunction1(TYPE (*pFunc)(const ARG_TYPE&), 
			const CValue<ARG_TYPE> *pArg = NULL)
		: m_pFunc(pFunc)
	{
		// listen for changes on the argument
		::AddObserver<CFunction1>(&argument, this, OnChange);

		// sync the argument to the passed CValue
		argument.SyncTo((CValue<ARG_TYPE> *)pArg);

		// now set the initial value
		Set((*this)(argument.Get()));
	}

	// function operator overload
	TYPE operator()(const ARG_TYPE& arg)
	{
		return (*m_pFunc)(arg);
	}

	// the only argument to the function
	CValue<ARG_TYPE> argument;

	// change to the argument
	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue)
	{
		// set the new calue
		Set((*this)(argument.Get()));
	}

protected:

	// don't let outsiders change the value of this function
	void Set(const TYPE& fromValue)
	{
		CValue<TYPE>::Set(fromValue);
	}

private:
	// function pointer
	TYPE (*m_pFunc)(const ARG_TYPE&);
};


//////////////////////////////////////////////////////////////////////
// class CFunction2<TYPE, ARG1_TYPE, ARG2_TYPE>
// 
// a function object that accepts ARG1_TYPE and ARG2_TYPE arguments 
// and is a CValue<TYPE>.
//
// the arguments can be synced to another CValue to implement
// functional pipelines.
//////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG1_TYPE, class ARG2_TYPE>
class CFunction2 : public CValue<TYPE>  
{
public:
	CFunction2(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&), 
				const CValue<ARG1_TYPE> *pArg1 = NULL, 
				const CValue<ARG2_TYPE> *pArg2 = NULL)
		: m_pFunc(pFunc)
	{
		// listen for changes on the argument1
		::AddObserver<CFunction2>(&argument1, this, OnChange);

		// sync the argument1 to the passed CValue
		argument1.SyncTo((CValue<ARG1_TYPE> *)pArg1);

		// listen for changes on the argument2
		::AddObserver<CFunction2>(&argument2, this, OnChange);

		// sync the argument1 to the passed CValue
		argument2.SyncTo((CValue<ARG2_TYPE> *)pArg2);

		// now set the initial value
		Set((*this)(argument1.Get(), argument2.Get()));
	}

	CFunction2(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&), 
				const CValue<ARG1_TYPE> *pArg1, 
				const ARG2_TYPE& argConst2)
		: m_pFunc(pFunc),
			argument2(argConst2)
	{
		// listen for changes on the argument1
		::AddObserver<CFunction2>(&argument1, this, OnChange);

		// sync the argument1 to the passed CValue
		argument1.SyncTo((CValue<ARG1_TYPE> *)pArg1);

		// listen for changes on the argument2
		::AddObserver<CFunction2>(&argument2, this, OnChange);

		// now set the initial value
		Set((*this)(argument1.Get(), argument2.Get()));
	}

	CFunction2(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&), 
				const ARG1_TYPE& argConst1, 
				const CValue<ARG2_TYPE> *pArg2)
		: m_pFunc(pFunc),
			argument1(argConst1)
	{
		// listen for changes on the argument1
		::AddObserver<CFunction2>(&argument1, this, OnChange);

		// listen for changes on the argument2
		::AddObserver<CFunction2>(&argument2, this, OnChange);

		// sync the argument1 to the passed CValue
		argument2.SyncTo((CValue<ARG1_TYPE> *)pArg2);

		// now set the initial value
		Set((*this)(argument1.Get(), argument2.Get()));
	}

	// function operator overload
	TYPE operator()(const ARG1_TYPE& arg1, const ARG2_TYPE& arg2)
	{
		return (*m_pFunc)(arg1, arg2);
	}

	// The arguments for the function
	CValue<ARG1_TYPE> argument1;
	CValue<ARG2_TYPE> argument2;

	// change to the arguments
	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue)
	{
		Set((*this)(argument1.Get(), argument2.Get()));
	}

protected:

	// don't let outsiders change the value of this function
	void Set(const TYPE& fromValue)
	{
		CValue<TYPE>::Set(fromValue);
	}

private:
	// function pointer
	TYPE (*m_pFunc)(const ARG1_TYPE&, const ARG2_TYPE&);
};


//////////////////////////////////////////////////////////////////////
// class CFunction3<TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE>
// 
// a function object that accepts ARG1_TYPE, ARG2_TYPE and ARG3_TYPE 
// arguments and is a CValue<TYPE>.
//
// the arguments can be synced to another CValue to implement
// functional pipelines.
//////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG1_TYPE, class ARG2_TYPE, class ARG3_TYPE>
class CFunction3 : public CValue<TYPE>  
{
public:
	CFunction3(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&, const ARG3_TYPE&), 
				const CValue<ARG1_TYPE> *pArg1 = NULL, 
				const CValue<ARG2_TYPE> *pArg2 = NULL,
				const CValue<ARG3_TYPE> *pArg3 = NULL)
		: m_pFunc(pFunc)
	{
		// listen for changes on the argument1
		::AddObserver<CFunction3>(&argument1, this, OnChange);

		// sync the argument1 to the passed CValue
		argument1.SyncTo((CValue<ARG1_TYPE> *)pArg1);

		// listen for changes on the argument2
		::AddObserver<CFunction3>(&argument2, this, OnChange);

		// sync the argument1 to the passed CValue
		argument2.SyncTo((CValue<ARG2_TYPE> *)pArg2);

		// listen for changes on the argument2
		::AddObserver<CFunction3>(&argument3, this, OnChange);

		// sync the argument1 to the passed CValue
		argument3.SyncTo((CValue<ARG2_TYPE> *)pArg3);

		// now set the initial value
		Set((*this)(argument1.Get(), argument2.Get(), argument3.Get()));
	}

	// function operator overload
	TYPE operator()(const ARG1_TYPE& arg1, const ARG2_TYPE& arg2,
			const ARG3_TYPE& arg3)
	{
		return (*m_pFunc)(arg1, arg2, arg3);
	}

	// The arguments for the function
	CValue<ARG1_TYPE> argument1;
	CValue<ARG2_TYPE> argument2;
	CValue<ARG3_TYPE> argument3;

	// change to the arguments
	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue)
	{
		Set((*this)(argument1.Get(), argument2.Get(), argument3.Get()));
	}

protected:

	// don't let outsiders change the value of this function
	void Set(const TYPE& fromValue)
	{
		CValue<TYPE>::Set(fromValue);
	}

private:
	// function pointer
	TYPE (*m_pFunc)(const ARG1_TYPE&, const ARG2_TYPE&, const ARG3_TYPE&);
};

#endif // !defined(AFX_FUNCTION_H__9ADD8ED4_FC5D_11D4_9E43_00B0D0609AB0__INCLUDED_)
