////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

#if !defined(OBJECTIVEFUNCTION_H)
#define OBJECTIVEFUNCTION_H

#include <Vector.h>
#include <Value.h>

template<int DIM, class TYPE>
class CObjectiveFunction
{
public:
	virtual TYPE operator()(const CVector<DIM, TYPE>& vInput) = 0;
};

template<int DIM, class TYPE>
class CLineFunction : public CObjectiveFunction<1, TYPE>
{
public:
	CLineFunction(CObjectiveFunction<DIM, TYPE> *pProjFunc)
		: m_pProjFunc(pProjFunc)
	{
	}

	CValue< CVector<DIM, TYPE> > point;
	CValue< CVector<DIM, TYPE> > direction;

	virtual TYPE operator()(const CVector<1, TYPE>& vInput)
	{
		return (*m_pProjFunc)(point.Get() + vInput[0] * direction.Get());
	}

private:
	CObjectiveFunction<DIM, TYPE> *m_pProjFunc;
};

template<int DIM, class TYPE>
class CGradObjectiveFunction : public CObjectiveFunction<DIM, TYPE>
{
public:
	virtual CVector<DIM, TYPE> Grad(const CVector<DIM, TYPE>& vInput) = 0;
};

template<int DIM, class TYPE>
class CGradLineFunction : public CGradObjectiveFunction<1, TYPE>
{
public:
	CGradLineFunction(CGradObjectiveFunction<DIM, TYPE> *pProjFunc)
		: m_pProjFunc(pProjFunc)
	{
	}

	CValue< CVector<DIM, TYPE> > point;
	CValue< CVector<DIM, TYPE> > direction;

	virtual TYPE operator()(const CVector<1, TYPE>& vInput)
	{
		return (*m_pProjFunc)(point.Get() + vInput[0] * direction.Get());
	}

	virtual CVector<1, TYPE> Grad(const CVector<1, TYPE>& vInput)
	{
		return m_pProjFunc->Grad(point.Get() + vInput[0] * direction.Get())
			* direction.Get();
	}

private:
	CGradObjectiveFunction<DIM, TYPE> *m_pProjFunc;
};

#endif
