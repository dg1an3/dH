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

///////////////////////////////////////////////////////////////////////////////
// GetInputScale
//
// Steepness s of the beamlet parameter transform: weight = height * Sigmoid(x, s).
//	Controls how fast a beamlet traverses from off to its ceiling, and so how
//	quickly the gradient dSigmoid = s*q*(1-q) dies off near the rails.
//
// Unlike the height, s does NOT cancel out of the adaptive-variance correction:
//	varSlope = dSigmoid(x,s) / dSigmoid(0,s) keeps its s dependence. Sweeping it
//	therefore moves the parameterization and the variance normalization together,
//	and results are confounded between the two.
//
// The caller supplies the fallback because the two call sites disagreed on where
//	the default comes from: Prescription reads the registry
//	(GetProfileReal("Prescription","InputScale",0.5)), while HistogramGradient
//	hardcoded 0.5 with a "should get this from the registry" note. GetProfileReal
//	is #ifdef __AFXWIN_H__-guarded and needs a live CWinApp, so pulling it in here
//	would put an MFC dependency into every consumer of this header.
//
// CAVEAT: this unifies the ENV path only. With BRIMSTONE_INPUT_SCALE unset, the
//	two call sites still take their own fallbacks -- so setting the *registry*
//	value alone moves Prescription's transform while HistogramGradient's variance
//	correction silently stays at 0.5. Sweep via the env var, not the registry.
///////////////////////////////////////////////////////////////////////////////
inline REAL GetInputScale(REAL fallback)
{
	static const REAL s_fromEnv = []() -> REAL
	{
		const char *pEnv = getenv("BRIMSTONE_INPUT_SCALE");
		return (pEnv != NULL) ? (REAL) atof(pEnv) : (REAL) -1.0;
	}();

	return (s_fromEnv > 0.0) ? s_fromEnv : fallback;

}	// GetInputScale

}	// namespace dH
