#pragma once

#include <VectorD.h>
#include <MatrixNxM.h>
#include <ModelObject.h>

class CGraph;

class CDataSeries :
	public CModelObject
{
public:
	CDataSeries(void);
	virtual ~CDataSeries(void);

	COLORREF GetColor() const;
	void SetColor(COLORREF color);

	void SetObject(CObject *pObject);
	CObject * GetObject();

	// accessors for the data series data
	const CMatrixNxM<>& GetDataMatrix() const;
	void SetDataMatrix(const CMatrixNxM<>& mData);
	void AddDataPoint(const CVectorD<2>& vDataPt);

	// flag to indicate whether the data series should have handles
	//		for interaction
	BOOL HasHandles() const;
	void SetHasHandles(BOOL bHandles = TRUE);

	// accessors to indicate the data series monotonic direction: 
	//		-1 for decreasing, 
	//		1 for increasing,
	//		0 for not monotonic
	int GetMonotonicDirection() const;
	void SetMonotonicDirection(int nDirection);

protected:
	friend CGraph;

	// my graph (if part of graph
	CGraph *m_pGraph;

	// the object pointer
	CObject * m_pObject;

	// use a std vector instead of CArray, because CArray is bad
	CMatrixNxM<> m_mData;	

	COLORREF m_color;

	// flag to indicate display of handles for the series
	BOOL m_bHandles;

	// indicates the data series monotonic direction: 
	//		-1 for decreasing, 
	//		1 for increasing,
	//		0 for not monotonic
	int m_nMonotonicDirection;

public:
	// pen style
	int m_nPenStyle;
};	// class CDataSeries
