// XMLLogFile.cpp: implementation of the CXMLLogFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "XMLElement.h"
#include "XMLLogFile.h"

#include "XMLLogging.h"
USES_FMT;

#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")


#define MODULE_NAME_LEN 64
#define SYMBOL_NAME_LEN 128

struct SYMBOL_INFO
{
	DWORD dwAddress;
	DWORD dwOffset;
	CHAR    szModule[MODULE_NAME_LEN];
	CHAR    szSymbol[SYMBOL_NAME_LEN];
};

///////////////////////////////////////////////////////////////////////////////
// FunctionTableAccess
//
// <description>
///////////////////////////////////////////////////////////////////////////////
static LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD dwPCAddress)
{
	return SymFunctionTableAccess(hProcess, dwPCAddress);

}	//  FunctionTableAccess


///////////////////////////////////////////////////////////////////////////////
// GetModuleBase
//
// <description>
///////////////////////////////////////////////////////////////////////////////
static DWORD __stdcall GetModuleBase(HANDLE hProcess, DWORD dwReturnAddress)
{
	IMAGEHLP_MODULE moduleInfo;

	if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
		return moduleInfo.BaseOfImage;
	else
	{
		MEMORY_BASIC_INFORMATION memoryBasicInfo;

		if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress,
			&memoryBasicInfo, sizeof(memoryBasicInfo)))
		{
			DWORD cch = 0;
			char szFile[MAX_PATH] = { 0 };

		 cch = GetModuleFileNameA((HINSTANCE)memoryBasicInfo.AllocationBase,
										 szFile, MAX_PATH);

		 // Ignore the return code since we can't do anything with it.
		 if (!SymLoadModule(hProcess,
			   NULL, ((cch) ? szFile : NULL),
			   NULL, (DWORD) memoryBasicInfo.AllocationBase, 0))
			{
				DWORD dwError = GetLastError();
				TRACE1("Error: %d\n", dwError);
			}
		 return (DWORD) memoryBasicInfo.AllocationBase;
	  }
		else
			TRACE1("Error is %d\n", GetLastError());
	}

	return 0;

}	//  GetModuleBase


///////////////////////////////////////////////////////////////////////////////
// ResolveSymbol
//
// <description>
///////////////////////////////////////////////////////////////////////////////
static BOOL ResolveSymbol(HANDLE hProcess, DWORD dwAddress,
	SYMBOL_INFO &siSymbol)
{
	BOOL fRetval = TRUE;

	siSymbol.dwAddress = dwAddress;

	union {
		CHAR rgchSymbol[sizeof(IMAGEHLP_SYMBOL) + 255];
		IMAGEHLP_SYMBOL  sym;
	};

	CHAR szUndec[256];
	CHAR szWithOffset[256];
	LPSTR pszSymbol = NULL;
	IMAGEHLP_MODULE mi;

	memset(&siSymbol, 0, sizeof(SYMBOL_INFO));
	mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

	if (!SymGetModuleInfo(hProcess, dwAddress, &mi))
		lstrcpyA(siSymbol.szModule, "(no module)");
	else
	{
		LPSTR pszModule = strchr(mi.ImageName, '\\');
		if (pszModule == NULL)
			pszModule = mi.ImageName;
		else
			pszModule++;

		lstrcpynA(siSymbol.szModule, pszModule, strlen(pszModule)+1);
		lstrcatA(siSymbol.szModule, "! ");
	}

	__try
	{
		sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
		sym.Address = dwAddress;
		sym.MaxNameLength = 255;

		if (SymGetSymFromAddr(hProcess, dwAddress, &(siSymbol.dwOffset), &sym))
		{
			pszSymbol = sym.Name;

			if (UnDecorateSymbolName(sym.Name, szUndec, strlen(szUndec),
				UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS))
			{
				pszSymbol = szUndec;
			}
			else if (SymUnDName(&sym, szUndec, strlen(szUndec)))
			{
				pszSymbol = szUndec;
			}

			if (siSymbol.dwOffset != 0)
			{
				wsprintfA(szWithOffset, "%s + %d bytes", pszSymbol, siSymbol.dwOffset);
				pszSymbol = szWithOffset;
			}
	  }
	  else
		  pszSymbol = "(no symbol)";
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		pszSymbol = "(EX: no symbol)";
		siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
	}

	lstrcpynA(siSymbol.szSymbol, pszSymbol, strlen(pszSymbol));
	return fRetval;

}	//  ResolveSymbol


/////////////////////////////////////////////////////////////////////////////
// AfxDumpStack API


CXMLLogFile CXMLLogFile::m_pGlobalFile("TestXML");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::CXMLLogFile
//
// <description>
///////////////////////////////////////////////////////////////////////////////
CXMLLogFile::CXMLLogFile(const char *pszAppName)
	: m_fLogFile(NULL),
		m_nNextUID(1),
		m_nEnabled(1)
{
	SetFormat((int) 0,			" % i ");
	SetFormat((float) 0,		" % 6.3lf ");
	SetFormat((double) 0,		" % lf ");
	SetFormat((char *) 0,		" %s ");
	SetFormat((const char *) 0, " %s ");

	// open the file
	m_fLogFile = fopen("LogFile.xml", "w+c");

	// set the top-level element
	FormatAnywhere("<la name='%s' uid='%08x'>\n", pszAppName, m_nNextUID);
	m_nNextUID++;

	CString strDir;
	::GetCurrentDirectory(100, strDir.GetBuffer(100));
	strDir.ReleaseBuffer();
	strDir += "\\DEBUG";

	if (!SymInitialize(GetCurrentProcess(), strDir.GetBuffer(100), FALSE))
	{
		DWORD dwError = GetLastError();
	}

}	//  CXMLLogFile::CXMLLogFile


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::~CXMLLogFile
//
// <description>
///////////////////////////////////////////////////////////////////////////////
CXMLLogFile::~CXMLLogFile()
{
	// flush and close
	Flush();

	while (m_arrElementStack.size() > 0)
		CloseElement();

	fclose(m_fLogFile);

}	// CXMLLogFile::~CXMLLogFile


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::NewSectionElement
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CXMLElement *CXMLLogFile::NewSectionElement(const char *pszName, const char *pszModule)
{
	// get the registry key for the section
	char pszSection[1000];
	strcpy(pszSection, "XMLLogging");
	strcat(pszSection, strrchr(pszModule, '\\'));
	if (m_nEnabled)
	{
		int nEnabled = ::AfxGetApp()->GetProfileInt(pszSection, pszName, -1);
		if (-1 == nEnabled)
		{
			BOOL bRes = ::AfxGetApp()->WriteProfileInt(pszSection, pszName, 1);
			ASSERT(bRes);
			nEnabled = 1;
		}
		
		// need to close previous tag before changing closed state
		if (m_arrElementStack.size() > 0)
		{
			// get the previous element and close its tag
			CXMLElement& elemPrev = m_arrElementStack.back();
			elemPrev.CloseTag(TRUE);
		}

		m_nEnabled = nEnabled;
	}

	CXMLElement *pSecElem = NewElement("ls", pszModule);
	pSecElem->Attribute("name", pszName);
	pSecElem->m_nEnabled = m_nEnabled;

	return pSecElem;

}	// CXMLLogFile::NewSectionElement


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::NewElement
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CXMLElement *CXMLLogFile::NewElement(const char *pszLabel, const char *pszModule)
{
	// flag to set the module
	BOOL bSetModule = TRUE;

	// if we have elements,
	if (m_arrElementStack.size() > 0)
	{
		// get the previous element and close its tag
		CXMLElement& elemPrev = m_arrElementStack.back();
		elemPrev.CloseTag(TRUE);

		// see if we need to set the module for the new element
		bSetModule = (elemPrev.GetModule() != pszModule);
	}

	// write the first part of the tag
	FormatAnywhere("\n<%s", pszLabel);

	// create the new element
	m_arrElementStack.push_back(CXMLElement(this, pszLabel, pszModule));
	CXMLElement& elem = m_arrElementStack.back();
	elem.m_nEnabled = m_nEnabled;

	elem.Attribute("uid", FMT("%08x", m_nNextUID));
	m_nNextUID++;

#ifdef USE_MODULE
	// see if the new element needs the module set
	if (bSetModule)
	{
		elem.Attribute("file", pszModule);
	}
#endif

	return &elem;

}	// CXMLLogFile::NewElement


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::CloseElement
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CXMLLogFile::CloseElement()
{
	// close the top element
	m_arrElementStack.back().Close(TRUE);
	
	// remove from stack
	m_arrElementStack.pop_back();

	m_nEnabled = 1;

	// iterate over elements, checking for any that are 
	std::vector< CXMLElement >::iterator iter = m_arrElementStack.end();
	while (iter != m_arrElementStack.begin())
	{	
		iter--;

		if (0 == iter->m_nEnabled)
		{
			m_nEnabled = 0;
		}
	}

	return TRUE;

}	// CXMLLogFile::CloseElement


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::Format
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CXMLLogFile::Format(const char *pszFormat, ...)
{
	if (!IsLogging())
	{
		return TRUE;
	}

	// close tag for current element
	m_arrElementStack.back().CloseTag(TRUE);

	// and do the printf
	va_list args;
	va_start(args, pszFormat);
	vfprintf(m_fLogFile, pszFormat, args);

	return TRUE;

}	// CXMLLogFile::Format


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::Message
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CXMLLogFile::Message(const char *pszFormat, ...)
{
	if (!IsLogging())
	{
		return TRUE;
	}

	if (m_arrElementStack.size() > 0)
	{
		// close tag for current element
		m_arrElementStack.back().CloseTag(TRUE);
	}

	NewElement("lm", "THIS_FILE");
	m_arrElementStack.back().CloseTag(TRUE);

	// and do the printf
	va_list args;
	va_start(args, pszFormat);
	vfprintf(m_fLogFile, pszFormat, args);

	CloseElement();			

	return TRUE;

}	// CXMLLogFile::Message

	
///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::FormatAnywhere
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CXMLLogFile::FormatAnywhere(const char *pszFormat, ...)
{
	if (!IsLogging())
	{
		return TRUE;
	}

	// do the printf
	va_list args;
	va_start(args, pszFormat);
	vfprintf(m_fLogFile, pszFormat, args);

	return TRUE;

}	// CXMLLogFile::FormatAnywhere


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::StackDump
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CXMLLogFile::StackDump(const char *pszMessage)
{
	if (!IsLogging())
	{
		return TRUE;
	}

	NewElement("lsd", __FILE__);
	Format("%s", pszMessage);

	CDWordArray adwAddress;
	HANDLE hProcess = ::GetCurrentProcess();
	if (SymInitialize(hProcess, NULL, FALSE))
	{
		// force undecorated names to get params
		DWORD dw = SymGetOptions();
		dw &= ~SYMOPT_UNDNAME;
		SymSetOptions(dw);

		HANDLE hThread = ::GetCurrentThread();
		CONTEXT threadContext;

		threadContext.ContextFlags = CONTEXT_FULL;

		if (::GetThreadContext(hThread, &threadContext))
		{
			STACKFRAME stackFrame;
			memset(&stackFrame, 0, sizeof(stackFrame));
			stackFrame.AddrPC.Mode = AddrModeFlat;

			DWORD dwMachType;

#if defined(_M_IX86)
			dwMachType                  = IMAGE_FILE_MACHINE_I386;

			// program counter, stack pointer, and frame pointer
			stackFrame.AddrPC.Offset    = threadContext.Eip;
			stackFrame.AddrStack.Offset = threadContext.Esp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
			stackFrame.AddrFrame.Offset = threadContext.Ebp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;
#elif defined(_M_MRX000)
			// only program counter
			dwMachType                  = IMAGE_FILE_MACHINE_R4000;
			stackFrame.AddrPC. Offset    = treadContext.Fir;
#elif defined(_M_ALPHA)
			// only program counter
			dwMachType                  = IMAGE_FILE_MACHINE_ALPHA;
			stackFrame.AddrPC.Offset    = (unsigned long) threadContext.Fir;
#elif defined(_M_PPC)
			// only program counter
			dwMachType                  = IMAGE_FILE_MACHINE_POWERPC;
			stackFrame.AddrPC.Offset    = threadContext.Iar;
#elif
#error("Unknown Target Machine");
#endif

			adwAddress.SetSize(0, 16);

			int nFrame;
			for (nFrame = 0; nFrame < 1024; nFrame++)
			{
				if (!StackWalk(dwMachType, hProcess, hProcess,
					&stackFrame, &threadContext, NULL,
					FunctionTableAccess, GetModuleBase, NULL))
				{
					break;
				}

				adwAddress.SetAtGrow(nFrame, stackFrame.AddrPC.Offset);
			}
		}
	}
	else
	{
		DWORD dw = GetLastError();
		Message("AfxDumpStack Error: IMAGEHLP.DLL wasn't found. "
			"GetLastError() returned 0x%8.8X\r\n", dw);
	}

	// dump it out now
	int nAddress;
	int cAddresses = adwAddress.GetSize();
	for (nAddress = 0; nAddress < cAddresses; nAddress++)
	{
		SYMBOL_INFO info;
		DWORD dwAddress = adwAddress[nAddress];

		CXMLElement *pElem = NewElement("lse", __FILE__);
		pElem->Attribute("address", FMT("%8.8X", dwAddress));

		if (ResolveSymbol(hProcess, dwAddress, info))
		{
			Format("%s%s", info.szModule, info.szSymbol);
		}
		else
			Format("symbol not found");

		CloseElement();
	}

	CloseElement();

	return TRUE;

}	// CXMLLogFile::StackDump


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::Flush
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CXMLLogFile::Flush()
{
	// store the current position
	long nAtPos = ftell(m_fLogFile);

	int nEnabled = m_nEnabled;

	// iterate over elements
	std::vector< CXMLElement >::iterator iter = m_arrElementStack.end();
	while (iter != m_arrElementStack.begin())
	{
		// back-up
		iter--;

		m_nEnabled = iter->m_nEnabled;

		// close, but not permanently
		iter->Close(FALSE);
	}

	// close top-level element
	FormatAnywhere("</la>\n");

	m_nEnabled = nEnabled;

	// flush and restore position
	fflush(m_fLogFile);
	fseek(m_fLogFile, nAtPos, SEEK_SET);

}	// CXMLLogFile::Flush


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::GetLogFile
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CXMLLogFile *CXMLLogFile::GetLogFile()
{
	return &m_pGlobalFile;

}	// CXMLLogFile::GetLogFile


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::IsLogging
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CXMLLogFile::IsLogging()
{
	return m_nEnabled;

}	// CXMLLogFile::IsLogging


///////////////////////////////////////////////////////////////////////////////
// CXMLLogFile::ClearLog
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CXMLLogFile::ClearLog()
{
	// open the file
	fclose(m_fLogFile);
	m_fLogFile = fopen("LogFile.xml", "w+c");

	int nEnabled = m_nEnabled;

	m_nNextUID = 0;

	// iterate over elements
	std::vector< CXMLElement >::iterator iter = m_arrElementStack.begin();
	while (iter != m_arrElementStack.begin())
	{
		m_nEnabled = iter->m_nEnabled;

		// write the first part of the tag
		FormatAnywhere("\n<%s", iter->m_pszLabel);

		iter->Attribute("uid", FMT("%08x", m_nNextUID));
		m_nNextUID++;

		iter->CloseTag(TRUE);

		// back-up
		iter++;
	}
	
	m_nEnabled = nEnabled;

	return TRUE;

}	// CXMLLogFile::ClearLog
