#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#include <afx.h>
#include <afxwin.h>


#include "GenUtils.h"

/*
static FILE *g_logFile = NULL;

void create_log_file()
{
	CString strModuleFilename;
	::GetModuleFileName(NULL, strModuleFilename.GetBuffer(_MAX_PATH),
		_MAX_PATH);
	strModuleFilename.ReleaseBuffer();

	int nLastDot = strModuleFilename.ReverseFind('.');
	strModuleFilename = strModuleFilename.Left(nLastDot);

	int nLastSlash = strModuleFilename.ReverseFind('\\');
	strModuleFilename = strModuleFilename.Mid(nLastSlash+1);

	CString strLogFilename;
	::GetTempPath(_MAX_PATH, strLogFilename.GetBuffer(_MAX_PATH));
	strLogFilename.ReleaseBuffer();

	strLogFilename += strModuleFilename + ".log";

	g_logFile = fopen(strLogFilename.GetBuffer(_MAX_PATH), "wt");
}

void log_trace(const char *pszFormat, ...)
{
	// make sure the log file exists
	if (g_logFile == NULL)
	{
		create_log_file();
	}

	// variable argument list
	va_list vl;
	va_start(vl, pszFormat);

	// print the trace message
	vfprintf(g_logFile, pszFormat, vl);

	// flush the log file
	fflush(g_logFile);
}
*/