// Workaround for VNL SSE inline issues with modern MSVC
// This file is force-included before any other headers via /FI compiler flag
// to override the __forceinline definition in vnl_sse.h

#ifndef VNL_FIX_H
#define VNL_FIX_H

// Define _MSC_VER temporarily as 0 to prevent vnl_sse.h from using __forceinline
// We'll restore it after including vnl_sse.h
#ifdef _MSC_VER
#define VNL_FIX_SAVED_MSC_VER _MSC_VER
#undef _MSC_VER
#define _MSC_VER 0
#endif

#endif // VNL_FIX_H
