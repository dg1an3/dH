#if !defined(FIELD_H)
#define FIELD_H



template<int IN_DIM, int OUT_DIM>
class CField : public CModelObject
{
public:
	virtual CVectorD<OUT_DIM> operator()(const CVectorD<IN_DIM>& v);

	virtual Eval(CVectorD<IN_DIM> *pVecs_in, int nCount, 
		CVectorD<OUT_DIM> *pVecs_out,
		CVectorD<IN_DIM> *pJac_out = NULL);

	virtual CField<OUT_DIM, IN_DIM> *GetInverse() = 0;
};









template<int IN_DIM, int OUT_DIM>
class CCompositeField : public CField<IN_DIM, OUT_DIM>
{
public:
	CCompositeField(); 
	
	template<int MID_DIM>
	void SetFields(CField<OUT_DIM, MID_DIM> *pOuterField, 
			CField<MID_DIM, IN_DIM> *pInnerField);

	template<class TYPE>
	void GetSubstitutionField(TYPE*& pInterface);


	// CField over-rides
	virtual CVectorD<OUT_DIM> operator()(const CVectorD<IN_DIM>& v);

	virtual Eval(CVectorD<IN_DIM> *pVecs_in, int nCount, 
		CVectorD<OUT_DIM> *pVecs_out,
		CVectorD<IN_DIM> *pJac_out = NULL);

	virtual CField<OUT_DIM, IN_DIM> *GetInverse();
};








template<int IN_DIM, int OUT_DIM>
class CBufferField : public CField<IN_DIM, OUT_DIM>
{
public:
	CBufferND<IN_DIM, CVectorD<OUT_DIM> > *GetBuffer();

	// CField over-rides
	virtual CVectorD<OUT_DIM> operator()(const CVectorD<IN_DIM>& v);

	virtual Eval(CVectorD<IN_DIM> *pVecs_in, int nCount, 
		CVectorD<OUT_DIM> *pVecs_out,
		CVectorD<IN_DIM> *pJac_out = NULL);

	virtual CField<OUT_DIM, IN_DIM> *GetInverse();

private:
	CBufferND<IN_DIM, CVectorD<OUT_DIM> > m_buffer;
};





template<int IN_DIM>
class CMeshSetField : public CField<IN_DIM, 1>
{
public:
	int GetMeshCount() const;
	CMeshND<IN_DIM, REAL> *GetMeshAt(int nAt);
	const CMeshND<IN_DIM, REAL> *GetMeshAt(int nAt) const;

	void AddUnionMesh(CMeshND<IN_DIM, REAL> *);

	// CField over-rides
	virtual CVectorD<1> operator()(const CVectorD<IN_DIM>& v);

	virtual Eval(CVectorD<IN_DIM> *pVecs_in, int nCount, 
		CVectorD<1> *pVecs_out,
		CVectorD<IN_DIM> *pJac_out = NULL);

	virtual CField<1, IN_DIM> *GetInverse();

private:
//	CPtrArray< CMeshND<IN_DIM, REAL> > m_arrMeshes;
};







template<int IN_DIM, int OUT_DIM>
class CIrregularField : public CField<IN_DIM, OUT_DIM>
{
public:
	int GetVertexCount() const;
	void AddVertex(const CVectorD<IN_DIM>& vPos, const CVectorD<OUT_DIM>& vValue);

	// CField over-rides
	virtual CVectorD<OUT_DIM> operator()(const CVectorD<IN_DIM>& v);

	virtual Eval(CVectorD<IN_DIM> *pVecs_in, int nCount, 
		CVectorD<OUT_DIM> *pVecs_out,
		CVectorD<IN_DIM> *pJac_out = NULL);

	virtual CField<OUT_DIM, IN_DIM> *GetInverse();


private:
//	CPtrArray< CVectorD<IN_DIM> > m_arrPositions;
//	CPtrArray< CVectorD<OUT_DIM> > m_arrValues;
};





template<int DIM>
class CReferenceFrame
{
public:
	template<int OTHER_DIM>
	BOOL GetTransformTo(CReferenceFrame<OTHER_DIM>& refFrame, 
				CField<DIM, OTHER_DIM>*& pField)
	{
		CField<DIM, OTHER_DIM> *pField = NULL;
		m_mapTransform.Lookup((void*) &refFrame, (void*&) pField);
		return pField;
	}

	template<int OTHER_DIM, class TYPE>
	void SetTransformTo(CReferenceFrame<OTHER_DIM>& refFrame, 
			CField<DIM, OTHER_DIM> *pField)
	{
		m_mapTransform.SetAt((void*) &refFrame, (void*) pField);
	}

//	CUnits *GetUnits(int nDim);

private:
	CMapPtrToPtr m_mapTransforms;
};




template<int DIM, class TYPE>
void MakeLapKernel(CBufferND<DIM, TYPE>& kernel_Lap, REAL sigma)
{
	REAL sig_sq = sigma * sigma;

	CBufferIndex<DIM> vIndex(kernel_Lap.GetExtent());
	while (vIndex.IsInRange())
	{
		REAL x = vIndex[0]; 
		REAL y = vIndex[1];
		REAL x2_y2 = (x * x + y * y) 
			/ (2.0 * sig_sq);

		kernel_Lap[vIndex][0] = 
			-1.0 / (PI * sig_sq * sig_sq)
				* (1.0 - x2_y2)
					* exp(-x2_y2);
		kernel_Lap[vIndex][1] = 
			-1.0 / (PI * sig_sq * sig_sq)
				* (1.0 - x2_y2)
					* exp(-x2_y2);
		kernel_Lap[vIndex][2] = 
			-1.0 / (PI * sig_sq * sig_sq)
				* (1.0 - x2_y2)
					* exp(-x2_y2);

		vIndex++;
	}
}


template<int DIM, class TYPE>
void WriteField(FILE *file, const CBufferND<DIM, TYPE>& buf_in)
{
	CBufferIndex<DIM> vBufIndex(buf_in.GetExtent());
	while (vBufIndex.IsInRange())
	{
		fprintf(file, "%lf,", buf_in[vBufIndex][1]);

		vBufIndex++;
		if (vBufIndex[0] == buf_in.GetExtent().GetMin()[0])
		{
			fprintf(file, "\n");
		}
	}

	fprintf(file, "\n\n");
}

#endif