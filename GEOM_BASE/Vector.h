#if !defined(VECTOR_H)
#define VECTOR_H

#include <math.h>

#define IS_APPROX_EQUAL(v1, v2) \
	(fabs(v1- v2) < (1e-6))

template<int DIM = 4, class TYPE = double>
class CVector
{
public:
	CVector() 
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	CVector(TYPE x) 
	{
		m_arrElements[0] = x;
		for (int nAt = 1; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	CVector(TYPE x, TYPE y) 
	{
		m_arrElements[0] = x;
		m_arrElements[1] = y;
		for (int nAt = 2; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	CVector(TYPE x, TYPE y, TYPE z) 
	{
		m_arrElements[0] = x;
		m_arrElements[1] = y;
		m_arrElements[2] = z;
		for (int nAt = 3; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	CVector(TYPE x, TYPE y, TYPE z, TYPE w) 
	{ 
		m_arrElements[0] = x;
		m_arrElements[1] = y;
		m_arrElements[2] = z;
		m_arrElements[3] = w;
		for (int nAt = 4; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

#ifdef _WINDOWS
	CVector(const CPoint& pt)
	{
		m_arrElements[0] = pt.x;
		m_arrElements[1] = pt.y;
	}
#endif

	TYPE& operator[](int nAtRow)
	{
		ASSERT(nAtRow >= 0 && nAtRow < DIM);
		return m_arrElements[nAtRow];
	}

	const TYPE& operator[](int nAtRow) const
	{
		ASSERT(nAtRow >= 0 && nAtRow < DIM);
		return m_arrElements[nAtRow];
	}

	operator TYPE *()
	{
		return &m_arrElements[0];
	}

	operator const TYPE *() const
	{
		return &m_arrElements[0];
	}

#ifdef _WINDOWS
	operator CPoint()
	{
		return CPoint((int)(*this)[0], (int)(*this)[1]);
	}
#endif

	CVector& operator+=(const CVector& vRight)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			(*this)[nAt] += vRight[nAt];

		return (*this);
	}

	CVector& operator-=(const CVector& vRight)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			(*this)[nAt] -= vRight[nAt];

		return (*this);
	}

	CVector& operator*=(TYPE scalar)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			(*this)[nAt] *= scalar;

		return (*this);
	}

//	TYPE operator*=(const CVector& vRight)
//	{
//		TYPE result = 0.0;
//
//		for (int nAt = 0; nAt < DIM; nAt++)
//			result += (*this)[nAt] * vRight[nAt];
//
//		return result;
//	}

	TYPE GetLength() const
	{
		TYPE len = 0.0;

		for (int nAt = 0; nAt < DIM; nAt++)
			len += (*this)[nAt] * (*this)[nAt];
		
		return static_cast<TYPE>(sqrt(len));
	}

	// normalize the vector length
	void Normalize()
	{
		TYPE len = GetLength();

		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] /= len;
	}

private:
	TYPE m_arrElements[DIM];
};

template<int DIM, class TYPE>
inline bool operator==(const CVector<DIM, TYPE>& vLeft, 
					   const CVector<DIM, TYPE>& vRight)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		if (vLeft[nAt] != vRight[nAt])
			return false;

	return true;
}

template<int DIM, class TYPE>
inline bool operator!=(const CVector<DIM, TYPE>& vLeft, 
					   const CVector<DIM, TYPE>& vRight)
{
	return !(vLeft == vRight);
}

template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator+(const CVector<DIM, TYPE>& vLeft, 
							  const CVector<DIM, TYPE>& vRight)
{
	CVector<DIM, TYPE> vSum = vLeft;
	vSum += vRight;
	return vSum;
}

template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator-(const CVector<DIM, TYPE>& vLeft, 
							  const CVector<DIM, TYPE>& vRight)
{
	CVector<DIM, TYPE> vSum = vLeft;
	vSum -= vRight;
	return vSum;
}

template<int DIM, class TYPE>
inline TYPE operator*(const CVector<DIM, TYPE>& vLeft, 
							  const CVector<DIM, TYPE>& vRight)
{
	TYPE prod = 0.0;
	for (int nAt = 0; nAt < DIM; nAt++)
		prod += vLeft[nAt] * vRight[nAt];

	return prod;
}

template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator*(TYPE scalar, 
							  const CVector<DIM, TYPE>& vRight)
{
	CVector<DIM, TYPE> vProd = vRight;
	vProd *= scalar;
	return vProd;
}

template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator*(const CVector<DIM, TYPE>& vLeft, 
							  TYPE scalar)
{
	CVector<DIM, TYPE> vProd = vLeft;
	vProd *= scalar;
	return vProd;
}

template<class TYPE>
inline CVector<3, TYPE> Cross(const CVector<3, TYPE>& vLeft, 
						      const CVector<3, TYPE>& vRight)
{
	CVector<3, TYPE> vProd;

	vProd[0] =   vLeft[1] * vRight[2] - vLeft[2] * vRight[1];
	vProd[1] = -(vLeft[0] * vRight[2] - vLeft[2] * vRight[0]);
	vProd[2] =   vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

	return vProd;
}

template<class TYPE>
inline double Cross(const CVector<2, TYPE>& vLeft, 
					const CVector<2, TYPE>& vRight)
{
	return vLeft[0] * vRight[1] - vLeft[1] * vRight[0];
}

template<int DIM, class TYPE>
inline CVector<DIM + 1, TYPE> ToHomogeneous(const CVector<DIM, TYPE>& v) 
{
	CVector<DIM + 1, TYPE> vh;
	for (int nAt = 0; nAt < DIM; nAt++)
		vh[nAt] = v[nAt];

	vh[DIM] = (TYPE) 1.0;

	return vh;
}

template<int DIM, class TYPE>
inline CVector<DIM, TYPE> FromHomogeneous(const CVector<DIM + 1, TYPE>& vh) 
{
	CVector<DIM, TYPE> v;
	for (int nAt = 0; nAt < DIM; nAt++)
		v[nAt] = vh[nAt] / vh[DIM];

	return v;
}

template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CVector<DIM, TYPE> v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		ar << v[nAt];
	return ar;
}

template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CVector<DIM, TYPE>& v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		ar >> v[nAt];
	return ar;
}

#define TRACE_VECTOR3(message, vector) \
	TRACE1("%s = ", message); \
	TRACE3("<%lf, %lf, %lf>\n", (vector)[0], (vector)[1], (vector)[2]);

#endif