#pragma once

#include <VectorD.h>
#include <MatrixNxM.h>

#include <ModelObject.h>
#include <Attributes.h>

class CGraph;

class CDataSeries :
	public CModelObject
{
public:
	CDataSeries(void);
	virtual ~CDataSeries(void);

	// the object of the data series
	DECLARE_ATTRIBUTE_PTR(Object, CObject);

	// curve attributes
	DECLARE_ATTRIBUTE_GI(Color, COLORREF);
	DECLARE_ATTRIBUTE(PenStyle, int);

	// accessors for the data series data
	virtual const CMatrixNxM<>& GetDataMatrix() const;
	virtual void SetDataMatrix(const CMatrixNxM<>& mData);
	void AddDataPoint(const CVectorD<2>& vDataPt);

	// flag to indicate whether the data series should have handles
	//		for interaction
	DECLARE_ATTRIBUTE(HasHandles, bool);

	// accessors to indicate the data series monotonic direction: 
	//		-1 for decreasing, 
	//		1 for increasing,
	//		0 for not monotonic
	DECLARE_ATTRIBUTE(MonotonicDirection, int);

protected:
	friend CGraph;

public:
	// my graph (if part of graph
	CGraph *m_pGraph;

	// use a std vector instead of CArray, because CArray is bad
	mutable CMatrixNxM<> m_mData;	

};	// class CDataSeries
