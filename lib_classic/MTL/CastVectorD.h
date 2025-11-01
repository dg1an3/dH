#ifndef CASTVECTORD_H
#define CASTVECTORD_H

#include <VectorD.h>
#include <VectorN.h>

//////////////////////////////////////////////////////////////////////
// class CCastVectorD<DIM, TYPE>
//
// helper class to easily cast vectors from other element types and 
//		sizes
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = REAL>
class CCastVectorD : public CVectorD<DIM, TYPE>
{
public:
	template<int OTHER_DIM, class OTHER_TYPE>
	explicit CCastVectorD(const CVectorD<OTHER_DIM, OTHER_TYPE>& vFrom)
	{
		if (DIM <= OTHER_DIM)
		{
			CopyValues(&(*this)[0], &vFrom[0], DIM);
		}
		else
		{
			CopyValues(&(*this)[0], &vFrom[0], OTHER_DIM);
			ZeroValues(&(*this)[OTHER_DIM], DIM - OTHER_DIM);
		}
	}

	template<class OTHER_TYPE>
	explicit CCastVectorD(const CVectorN<OTHER_TYPE>& vFrom)
	{
		if (DIM <= vFrom.GetDim())
		{
			CopyValues(&(*this)[0], &vFrom[0], DIM);
		}
		else
		{
			CopyValues(&(*this)[0], &vFrom[0], vFrom.GetDim());
			ZeroValues(&(*this)[vFrom.GetDim()], DIM - vFrom.GetDim());
		}
	}
};


#endif
