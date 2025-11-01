#if !defined(_GENUTILS_H_)
#define _GENUTILS_H_

// log_trace function dec
void log_trace(const char *, ...);

// LOG_TRACE macro
#ifdef _DEBUG
#define LOG_TRACE TRACE
#else
#define LOG_TRACE log_trace
#endif

// timing macros
#define START_TIMER(EVENT_FLAG)			\
{										\
	LARGE_INTEGER freq;					\
	QueryPerformanceFrequency(&freq);	\
										\
	LARGE_INTEGER start;				\
	QueryPerformanceCounter(&start);	\

// timer end macro
#define STOP_TIMER(EVENT_FLAG, LABEL)	\
	LARGE_INTEGER end;					\
	QueryPerformanceCounter(&end);		\
										\
	LOG_TRACE("Time: " LABEL " = %lf\n",	\
		(double) (end.QuadPart - start.QuadPart)	\
			/ (double) freq.QuadPart);	\
}										\

// check COM result
#define CR(Cmd) \
{						\
	HRESULT hr = (Cmd); \
	if (FAILED(hr))		\
	{					\
		TRACE("COM Command Failed -- Error %x\n", hr); \
		return hr;		\
	}					\
}

// profile flag to control behavior
#define PROFILE_FLAG(Section, Key) \
	static BOOL bInit = FALSE;		\
	static BOOL bShow = FALSE;		\
	if (!bInit)						\
	{								\
		bShow =						\
			(BOOL) ::AfxGetApp()->GetProfileInt(Section, Key, 0);	\
		bInit = TRUE;				\
	}								\
	if (bShow)				

#endif  // !defined(_GENUTILS_H_)
