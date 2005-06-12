// XMLLogFile.h: interface for the CXMLLogFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCTLOGXMLFILE_H__EBCA9A41_6C53_45F8_8B79_9E958F3786DD__INCLUDED_)
#define AFX_STRUCTLOGXMLFILE_H__EBCA9A41_6C53_45F8_8B79_9E958F3786DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <typeinfo.h>

#include <vector>

class CXMLElement;

//////////////////////////////////////////////////////////////////////
// class CXMLLogFile
//
// main object for controlling XML logging
//////////////////////////////////////////////////////////////////////
class CXMLLogFile
{
public:
	BOOL ClearLog();
	long m_nNextUID;
	// construction / destruction
	CXMLLogFile(const char *pszAppName);
	virtual ~CXMLLogFile();

	// creates a new element and places on stack
	CXMLElement *NewSectionElement(const char *pszName, const char *pszFile);

	// creates a new element and places on stack
	CXMLElement *NewElement(const char *pszLabel, const char *pszFile);

	// closes top stack element
	BOOL CloseElement();

	// outputs a formatted string, closing the opening tag for the
	//		top stack element
	BOOL Format(const char *pszFormat, ...);

	// creates a new message element, and outputs text
	BOOL Message(const char *pszFormat, ...);

	// creates a stack dump in to the log file
	BOOL StackDump(const char *pszMessage);

	// retrieves the current format for a type
	template<typename TYPE>
	const char *GetFormat(TYPE var)
	{
		CString strFormat;
		if (m_mapFormats.Lookup(typeid(TYPE).name(), strFormat))
		{
			return strFormat;
		}
		return NULL;
	}

	// sets the current format for a type
	template<typename TYPE>
	void SetFormat(TYPE var, const char *pszFormat)
	{
		m_mapFormats[typeid(TYPE).name()] = pszFormat;
	}

	// creates an expression element
	template<typename TYPE>
	void LogExpr(TYPE var, const char *pszName, const char *pszModule)
	{
		CXMLElement *pVarElem = NewElement("lx", pszModule);
		if (strlen(pszName) > 0)
			pVarElem->Attribute("name", pszName);

		const char *pszFormat = GetFormat(var);
		if (pszFormat)
		{
			pVarElem->Attribute("type", typeid(TYPE).name());
			Format(pszFormat, var);
		}
		else
		{
			pVarElem->Attribute("type", "UNKNOWN-TYPE");
		}
		CloseElement();
	}

	// flag to indicate logging is currently on
	BOOL IsLogging();

	// flush the log file
	void Flush();

	// returns the singleton log file
	static CXMLLogFile *GetLogFile();

protected:

	// only CXMLElement can access here
	friend class CXMLElement;

	// formats without closing current tag
	BOOL FormatAnywhere(const char *pszFormat, ...);

private:

	// the log file
	FILE *m_fLogFile;

	// the stack of elements
	std::vector< CXMLElement > m_arrElementStack;

	// flag to indicate enabled
public:
	int m_nEnabled;

	// the map of formats
	CMapStringToString m_mapFormats;

	// the singleton
	static CXMLLogFile m_pGlobalFile;

};	// class CXMLLogFile



#endif // !defined(AFX_STRUCTLOGXMLFILE_H__EBCA9A41_6C53_45F8_8B79_9E958F3786DD__INCLUDED_)
