#if !defined(XMLLOGGING_H)
#define XMLLOGGING_H

#include <XMLLogFile.h>
#include <XMLLoggableObject.h>

#ifdef XMLLOGGING_ON
///////////////////////////////////////////////////////////////////////////	
// begins a section in the log file
///////////////////////////////////////////////////////////////////////////	
#define BEGIN_LOG_SECTION(name) \
{															\
	CXMLLogFile::GetLogFile()->NewSectionElement(#name, THIS_FILE)	\

///////////////////////////////////////////////////////////////////////////	
// begins a section in the log file
///////////////////////////////////////////////////////////////////////////	
#define BEGIN_LOG_SECTION_(name) \
{															\
	CXMLLogFile::GetLogFile()->NewSectionElement(name, THIS_FILE)	\

///////////////////////////////////////////////////////////////////////////	
// begins a section in the log file
///////////////////////////////////////////////////////////////////////////	
#define BEGIN_LOG_ONLY(name) \
	if (CXMLLogFile::GetLogFile()->IsLogging())	\
	{											\
		CXMLLogFile::GetLogFile()->NewSectionElement(#name, THIS_FILE)

///////////////////////////////////////////////////////////////////////////	
// exits a section early
///////////////////////////////////////////////////////////////////////////	
#define EXIT_LOG_SECTION() \
	CXMLLogFile::GetLogFile()->CloseElement();

///////////////////////////////////////////////////////////////////////////	
// ends a section in the log file
///////////////////////////////////////////////////////////////////////////	
#define END_LOG_SECTION() \
	CXMLLogFile::GetLogFile()->CloseElement();				\
}

///////////////////////////////////////////////////////////////////////////	
// flushes the log file
///////////////////////////////////////////////////////////////////////////	
#define FLUSH_LOG() \
	CXMLLogFile::GetLogFile()->Flush();


///////////////////////////////////////////////////////////////////////////	
// creates a log message element in the log file
///////////////////////////////////////////////////////////////////////////	
#define LOG \
	CXMLLogFile::GetLogFile()->Message


///////////////////////////////////////////////////////////////////////////	
// basic log expression macro
///////////////////////////////////////////////////////////////////////////	
#define LOG_EXPR(expr) \
	if (CXMLLogFile::GetLogFile()->IsLogging())	\
	{											\
		CXMLLogFile::GetLogFile()->LogExpr(expr, #expr, THIS_FILE);	\
	}

///////////////////////////////////////////////////////////////////////////	
// log expression macro with description
///////////////////////////////////////////////////////////////////////////	
#define LOG_EXPR_DESC(expr, desc) \
	if (CXMLLogFile::GetLogFile()->IsLogging())	\
	{											\
		CXMLLogFile::GetLogFile()->LogExpr(expr, desc, THIS_FILE);	\
	}

///////////////////////////////////////////////////////////////////////////	
// extended log expression macro
///////////////////////////////////////////////////////////////////////////	
#define LOG_EXPR_EXT(expr) \
	if (CXMLLogFile::GetLogFile()->IsLogging())	\
	{											\
		LogExprExt(expr, #expr, THIS_FILE);		\
	}

///////////////////////////////////////////////////////////////////////////	
// extended log expression macro
///////////////////////////////////////////////////////////////////////////	
#define LOG_EXPR_EXT_DESC(expr, desc) \
	if (CXMLLogFile::GetLogFile()->IsLogging())	\
	{											\
		LogExprExt(expr, desc, THIS_FILE);		\
	}


///////////////////////////////////////////////////////////////////////////	
// log stack dump
///////////////////////////////////////////////////////////////////////////	
#define LOG_STACK_DUMP(msg) \
	CXMLLogFile::GetLogFile()->StackDump(msg);


#ifdef _DEBUG

#undef ASSERT
#define ASSERT(f)	\
	do														\
	{														\
		if (!(f))											\
		{													\
			if (AfxAssertFailedLine(THIS_FILE, __LINE__))	\
			{												\
				LOG_STACK_DUMP(FMT("ASSERTION FAILED: %s", #f));							\
				FLUSH_LOG();								\
				AfxDebugBreak();							\
			}												\
		}													\
	} while (0)	\

#endif // _DEBUG

#else

#define BEGIN_LOG_SECTION(name) { 
#define BEGIN_LOG_SECTION_(name) {
#define BEGIN_LOG_ONLY(name) if (0) {
#define EXIT_LOG_SECTION() 
#define END_LOG_SECTION() (void) 0; }
#define FLUSH_LOG()
#define LOG 1 ? (void) 0 : ::AfxTrace
#define LOG_EXPR(expr)
#define LOG_EXPR_DESC(expr, desc)
#define LOG_EXPR_EXT(expr)
#define LOG_EXPR_EXT_DESC(expr, desc) 
#define LOG_STACK_DUMP(msg)

#endif	// XMLLOGGING_ON

///////////////////////////////////////////////////////////////////////////	
// log timing section
///////////////////////////////////////////////////////////////////////////	
#define BEGIN_TIME_LOOP(name, loop_count) \
\
{															\
	CXMLLogFile::GetLogFile()->NewSectionElement(#name, THIS_FILE);	\
																	\
	int nLoopCount = loop_count;									\
	LARGE_INTEGER freq;												\
	::QueryPerformanceFrequency(&freq);								\
																	\
	LARGE_INTEGER time_start;										\
	::QueryPerformanceCounter(&time_start);							\
	for (int nTimeCount = 0; nTimeCount < loop_count; nTimeCount++)	\
	{																\

///////////////////////////////////////////////////////////////////////////	
// log timing section
///////////////////////////////////////////////////////////////////////////	
#define END_TIME_LOOP() \
	}	/* for */													\
	LARGE_INTEGER time_end;											\
	::QueryPerformanceCounter(&time_end);							\
	LOG("Time for section = %lf (ms)", 1000.0 * 					\
		(double) (time_end.QuadPart - time_start.QuadPart)			\
			/ (double) (freq.QuadPart * nLoopCount));				\
	cout << "Time for section = " << 1000.0 * 						\
		(double) (time_end.QuadPart - time_start.QuadPart)			\
			/ (double) (freq.QuadPart * nLoopCount) << "(ms)" << endl;	\
	CXMLLogFile::GetLogFile()->CloseElement();						\
}																	\

#define USES_FMT \
static char g_pszBuffer[1000];						\
static const char *fmt(const char *pszFormat, ...)	\
{													\
	va_list args;									\
	va_start(args, pszFormat);						\
	vsprintf(g_pszBuffer, pszFormat, args);			\
	return g_pszBuffer;								\
}	\

#define FMT fmt

#endif	// !defined(XMLLOGGING_H)