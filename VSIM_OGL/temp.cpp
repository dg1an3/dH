CMatrix<4> CSimView::ComputeBEVScale(const CVector<2>& vMin, const CVector<2>& vMax, const double& aspect)
{
	CMatrix<4> mScale;

	// translate forward slightly (will not affect projection, since it occurs _after_ the perspective)
	mScale *= CreateTranslate(0.01, CVector<3>(0.0, 0.0, 1.0));

	CVector<2> vMin = beam.myCollimMin;
	CVector<2> vMax = beam.myCollimMax;

	double collimHeight = max(fabs(vMin[1]), fabs(vMax[1])) + 1.0;
	double collimWidth = max(fabs(vMin[0]), fabs(vMax[0])) + 1.0;
	double height = collimWidth / aspect;

	// scale, including a slight dilation along the z-axis to avoid clipping the collimator plane
	mScale *= CreateScale(CVector<3>(1.0 / collimWidth, 1.0 / height, 1.0));

	return mScale;
}

