#include "Value.h"
#include "Function.h"
#include "Matrix.h"


#define FUNCTION_FACTORY1(FUNCTION, TYPE) \
template<class ARG_TYPE>													\
CValue< TYPE >& FUNCTION(CValue< ARG_TYPE >& val)							\
{																			\
	CValue< TYPE > *pFunc =													\
		new CFunction1< TYPE, ARG_TYPE >(FUNCTION, &val);					\
	val.AddFunction((CObject *)pFunc);													\
	return (*pFunc);														\
} 

#define FUNCTION_FACTORY2(FUNCTION, TYPE)	\
template<class ARG1_TYPE, class ARG2_TYPE>									\
CValue< TYPE >& FUNCTION(CValue< ARG1_TYPE >& lVal,							\
		CValue< ARG2_TYPE >& rVal)											\
{																			\
	CValue< TYPE > *pFunc =													\
		new CFunction2< TYPE, ARG1_TYPE, ARG2_TYPE >(FUNCTION,				\
			&lVal, &rVal);													\
	lVal.AddFunction((CObject *)pFunc);												\
	return (*pFunc);														\
}																			\
																			\
template<class ARG1_TYPE, class ARG2_TYPE>									\
CValue< TYPE >& FUNCTION(CValue< ARG1_TYPE >& lVal, const ARG2_TYPE& rConst) \
{																			\
	CValue< TYPE > *pFunc =													\
		new CFunction2< TYPE, ARG1_TYPE, ARG2_TYPE>(FUNCTION, &lVal, rConst); \
	lVal.AddFunction((CObject *)pFunc);												\
	return (*pFunc);														\
}																			\
																			\
template<class ARG1_TYPE, class ARG2_TYPE>									\
CValue< TYPE >& FUNCTION(const ARG1_TYPE& lConst, CValue< ARG2_TYPE>& rVal)	\
{																			\
	CValue< TYPE > *pFunc =													\
		new CFunction2< TYPE, ARG1_TYPE, ARG2_TYPE>(FUNCTION, lConst, &rVal); \
	rVal.AddFunction((CObject *)pFunc);												\
	return (*pFunc);														\
} 

#define FUNCTION_FACTORY2A(FUNCTION1, FUNCTION2, TYPE)	\
template<class ARG1_TYPE, class ARG2_TYPE>									\
CValue< TYPE >& FUNCTION1(CValue< ARG1_TYPE >& lVal,							\
		CValue< ARG2_TYPE >& rVal)											\
{																			\
	CValue< TYPE > *pFunc =													\
		new CFunction2< TYPE, ARG1_TYPE, ARG2_TYPE >(FUNCTION2,				\
			&lVal, &rVal);													\
	lVal.AddFunction((CObject *)pFunc);												\
	return (*pFunc);														\
}																			\
																			\
template<class ARG1_TYPE, class ARG2_TYPE>									\
CValue< TYPE >& FUNCTION1(CValue< ARG1_TYPE >& lVal, const ARG2_TYPE& rConst) \
{																			\
	CValue< TYPE > *pFunc =													\
		new CFunction2< TYPE, ARG1_TYPE, ARG2_TYPE>(FUNCTION2, &lVal, rConst); \
	lVal.AddFunction((CObject *)pFunc);												\
	return (*pFunc);														\
}																			\
																			\
template<class ARG1_TYPE, class ARG2_TYPE>									\
CValue< TYPE >& FUNCTION1(const ARG1_TYPE& lConst, CValue< ARG2_TYPE>& rVal)	\
{																			\
	CValue< TYPE > *pFunc =													\
		new CFunction2< TYPE, ARG1_TYPE, ARG2_TYPE>(FUNCTION2, lConst, &rVal); \
	rVal.AddFunction((CObject *)pFunc);												\
	return (*pFunc);														\
} 

FUNCTION_FACTORY2(operator*, CMatrix<4>)
FUNCTION_FACTORY2(CreateRotate, CMatrix<4>)

FUNCTION_FACTORY1(CreateTranslate, CMatrix<4>)
FUNCTION_FACTORY2(CreateTranslate, CMatrix<4>)

FUNCTION_FACTORY1(CreateScale, CMatrix<4>)

FUNCTION_FACTORY1(Invert, CMatrix<4>)

inline double add(const double& l, const double& r)
{
	return l + r;
}

inline double subtract(const double& l, const double& r)
{
	return l - r;
}

FUNCTION_FACTORY2A(operator+, add, double)
FUNCTION_FACTORY2A(operator-, subtract, double)