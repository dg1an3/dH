// Function.h: interface for the CFunction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FUNCTION_H__9ADD8ED4_FC5D_11D4_9E43_00B0D0609AB0__INCLUDED_)
#define AFX_FUNCTION_H__9ADD8ED4_FC5D_11D4_9E43_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Value.h"

template<class TYPE>
class CMemberFunction : public CValue<TYPE>  
{
public:
	CMemberFunction()
	{
	}

	virtual ~CMemberFunction()
	{
	}

	const TYPE& operator()() const
	{
		return Get();
	}

protected:
	TYPE& Get() 
	{
		return CValue<TYPE>::Get();
	}

	const TYPE& Get() const
	{
		return CValue<TYPE>::Get();
	}


	void Set(const TYPE& fromValue)
	{
		CValue<TYPE>::Set(fromValue);
	}
};

template<class TYPE, class ARG_TYPE>
class CFunction1 : public CValue<TYPE>  
{
public:
	CFunction1(TYPE (*pFunc)(const ARG_TYPE&), const CValue<ARG_TYPE> *pArg)
		: m_pFunc(pFunc),
			m_pArg(pArg)
	{
		ASSERT(m_pArg != NULL);
		m_pArg->AddObserver(this);
	}

	virtual ~CFunction1()
	{
		m_pArg->RemoveObserver(this);
	}

	virtual void OnChange(CObservable *pFromObject)
	{
		Set((*m_pFunc)(m_pArg->Get()));
	}

protected:
	void Set(const TYPE& fromValue)
	{
		CValue<TYPE>::Set(fromValue);
	}

private:
	TYPE (*m_pFunc)(const ARG_TYPE&);

	const CValue<ARG_TYPE> *m_pArg;
};

template<class TYPE, class ARG1_TYPE, class ARG2_TYPE>
class CFunction2 : public CValue<TYPE>  
{
public:
	CFunction2(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&), 
				const CValue<ARG1_TYPE> *pArg1, 
				const CValue<ARG2_TYPE> *pArg2)
		: m_pFunc(pFunc),
			m_pArg1(pArg1),
			m_pArg2(pArg2)
	{
		ASSERT(m_pArg1 != NULL);
		m_pArg1->AddObserver(this);

		ASSERT(m_pArg2 != NULL);
		m_pArg2->AddObserver(this);
	}

	CFunction2(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&), 
				const CValue<ARG1_TYPE> *pArg1, 
				const ARG2_TYPE& argConst2)
		: m_pFunc(pFunc),
			m_pArg1(pArg1),
			m_argConst2(argConst2),
			m_pArg2(&m_argConst2)
	{
		ASSERT(m_pArg1 != NULL);
		m_pArg1->AddObserver(this);
	}

	CFunction2(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&), 
				const ARG1_TYPE& argConst1, 
				const CValue<ARG2_TYPE> *pArg2)
		: m_pFunc(pFunc),
			m_argConst1(argConst1),
			m_pArg1(&m_argConst1),
			m_pArg2(pArg2)
	{
		ASSERT(m_pArg2 != NULL);
		m_pArg2->AddObserver(this);
	}

	virtual ~CFunction2()
	{
		m_pArg1->RemoveObserver(this);
		m_pArg2->RemoveObserver(this);
	}

	virtual void OnChange(CObservable *pFromObject)
	{
		Set((*m_pFunc)(m_pArg1->Get(), m_pArg2->Get()));
	}

protected:
	void Set(const TYPE& fromValue)
	{
		CValue<TYPE>::Set(fromValue);
	}

private:
	TYPE (*m_pFunc)(const ARG1_TYPE&, const ARG2_TYPE&);

	CValue<ARG1_TYPE> m_argConst1;
	const CValue<ARG1_TYPE> *m_pArg1;

	CValue<ARG2_TYPE> m_argConst2;
	const CValue<ARG2_TYPE> *m_pArg2;
};

template<class TYPE, class ARG1_TYPE, class ARG2_TYPE, class ARG3_TYPE>
class CFunction3 : public CValue<TYPE>  
{
public:
	CFunction3(TYPE (*pFunc)(const ARG1_TYPE&, const ARG2_TYPE&, const ARG3_TYPE&), 
				const CValue<ARG1_TYPE> *pArg1, 
				const CValue<ARG2_TYPE> *pArg2,
				const CValue<ARG3_TYPE> *pArg3)
		: m_pFunc(pFunc),
			m_pArg1(pArg1),
			m_pArg2(pArg2),
			m_pArg3(pArg3)
	{
		ASSERT(m_pArg1 != NULL);
		m_pArg1->AddObserver(this);

		ASSERT(m_pArg2 != NULL);
		m_pArg2->AddObserver(this);

		ASSERT(m_pArg3 != NULL);
		m_pArg3->AddObserver(this);
	}

	virtual ~CFunction3()
	{
		m_pArg1->RemoveObserver(this);
		m_pArg2->RemoveObserver(this);
		m_pArg3->RemoveObserver(this);
	}

	virtual void OnChange(CObservable *pFromObject)
	{
		Set((*m_pFunc)(m_pArg1->Get(), m_pArg2->Get(), m_pArg3->Get()));
	}

protected:
	void Set(const TYPE& fromValue)
	{
		CValue<TYPE>::Set(fromValue);
	}

private:
	TYPE (*m_pFunc)(const ARG1_TYPE&, const ARG2_TYPE&, const ARG3_TYPE&);

	const CValue<ARG1_TYPE> *m_pArg1;
	const CValue<ARG2_TYPE> *m_pArg2;
	const CValue<ARG3_TYPE> *m_pArg3;
};

#endif // !defined(AFX_FUNCTION_H__9ADD8ED4_FC5D_11D4_9E43_00B0D0609AB0__INCLUDED_)
