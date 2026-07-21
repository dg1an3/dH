// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
#pragma once

#include <stdlib.h>

#include <MathUtil.h>

namespace dH
{

///////////////////////////////////////////////////////////////////////////////
// GetSigmoidScale
//
// Height of the beamlet parameter transform: weight = scale * Sigmoid(x, s).
//	This is the maximum beamlet weight, so it sets the maximum deliverable dose
//	per beamlet.
//
// Read once from BRIMSTONE_SIGMOID_SCALE, defaulting to the historical 0.2, so
//	a sweep driver can vary it per run without a rebuild (each run is a fresh
//	process). Same pattern as Prescription::GetEntropyWeight.
//
// NOTE: this value used to be duplicated as a literal in two places --
//	Prescription.cpp (namespace scope) and HistogramGradient.cpp (function
//	local, annotated "should get this from Prescription"). They are read
//	together: Prescription applies the transform, HistogramGradient applies the
//	matching adaptive-variance correction. If they disagree, the variance
//	correction silently stops matching the transform it is correcting for. Route
//	every use through here.
//
// The value cancels exactly out of the adaptive-variance arithmetic --
//	varSlope divides scale*dSigmoid(x) by scale*dSigmoid(0), and varWeight
//	divides scale*Sigmoid(x) by scale/2 -- so changing it moves only the beamlet
//	ceiling, not the variance model.
///////////////////////////////////////////////////////////////////////////////
inline REAL GetSigmoidScale()
{
	// magic static: initialized once, thread-safe, which matters because the
	//	first call can come from the optimizer worker thread
	static const REAL s_scale = []() -> REAL
	{
		const char *pEnv = getenv("BRIMSTONE_SIGMOID_SCALE");
		return (pEnv != NULL) ? (REAL) atof(pEnv) : (REAL) 0.2;
	}();

	return s_scale;

}	// GetSigmoidScale

}	// namespace dH
