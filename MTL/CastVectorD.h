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
			AssignValues(&(*this)[0], &vFrom[0], DIM);
		}
		else
		{
			AssignValues(&(*this)[0], &vFrom[0], OTHER_DIM);
			memset(&(*this)[OTHER_DIM], 0, (DIM - OTHER_DIM) * sizeof)TYPE);
		}
	}

	template<class OTHER_TYPE>
	explicit CCastVectorD(const CVectorN<OTHER_TYPE>& vFrom)
	{
		if (DIM <= vFrom.GetDim())
		{
			AssignValues(&(*this)[0], &vFrom[0], DIM);
		}
		else
		{
			AssignValues(&(*this)[0], &vFrom[0], vFrom.GetDim());
			memset(&(*this)[vFrom.GetDim()], 0, (DIM - vFrom.GetDim()) * sizeof(TYPE));
		}
	}
};


#endif
