#include "stdafx.h"

#include "MatrixFunction.h"

CValue< CMatrix<4> > mRotX( CreateRotate(0.1, CVector<3>(1.0, 0.0, 0.0)) );
CValue< CMatrix<4> > mRotY( CreateRotate(0.1, CVector<3>(0.0, 1.0, 0.0)) );
CValue< CMatrix<4> > mRotZ( CreateRotate(0.1, CVector<3>(0.0, 0.0, 1.0)) );


CValue< CMatrix<4> >& mInvert = Invert(mRotX);

CValue< double > angle;
CValue< double > diffAngle;

CValue< CMatrix<4> >& mRotate =
	mRotX * CreateRotate(angle, CVector<3>(0.0, 1.0, 0.0)) * mRotZ;

//template<class ARG1_TYPE, class ARG2_TYPE>									
//CValue< double >& operator-(CValue< ARG1_TYPE >& lVal,						
//		CValue< ARG2_TYPE >& rVal)											
//{																			
//	CValue< double > *pFunc =												
//		new CFunction2< double, ARG1_TYPE, ARG2_TYPE >(operator-,			
//			&lVal, &rVal);													
//	lVal.AddFunction((CObject *)pFunc);										
//	return (*pFunc);														
//}																			

//CValue< double >& operator-(CValue< double >& lVal,						
//		CValue< double >& rVal)											
//{																			

//double (*pFunc)(double, double) = &operator-;

//	CValue< double > *pFunc =												
//		new CFunction2< double, double, double >(operator-,
//			&angle, &diffAngle);													
//	lVal.AddFunction((CObject *)pFunc);										
//	return (*pFunc);														
//}																			

CValue< double >& result = angle - diffAngle;

void test()
{
	CValue< CMatrix<4> > mLeft;
	CValue< CMatrix<4> > mRight;

	// CValue< CMatrix<4> >& mProduct = mLeft * mRight;
}