// Line.h: interface for the CLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINE_H__95792EB2_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
#define AFX_LINE_H__95792EB2_1715_11D5_9E54_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Vector.h"

class CLine : public CObject  
{
public:
	CLine(const CVector<2>&, const CVector<2>&);
	virtual ~CLine();

	double Eval(const CVector<2>&) const;
	int Classify(const CVector<2>&) const;

private:
	double m_a, m_b, m_c;
};

inline CLine::CLine(const CVector<2>& p, const CVector<2>& q)
// Computes the normalized line equation through the
// points p and q.
{
	CVector<2> t = q - p;
	double len =  t.GetLength();
	m_a = t[1] / len; 
	m_b = - t[0] / len; 
	m_c = - (m_a * p[0] + m_b * p[1]);
}

inline double CLine::Eval(const CVector<2>& p) const
// Plugs point p into the line equation.
{
	return (m_a * p[0] + m_b * p[1] + m_c);
}

inline int CLine::Classify(const CVector<2>& p) const
// Returns -1, 0, or 1, if p is to the left of, on,
// or right of the line, respectively.
{
	double d = Eval(p);
	return (d < -EPS) ? -1 : (d > EPS ? 1 : 0);
}


#endif // !defined(AFX_LINE_H__95792EB2_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
