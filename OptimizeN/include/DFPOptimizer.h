#pragma once
#include <Optimizer.h>

class CDFPOptimizer :
	public COptimizer
{
public:
	CDFPOptimizer(CObjectiveFunction *pFunc);
	~CDFPOptimizer(void);
};
