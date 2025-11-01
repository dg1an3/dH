// LoggableObject.h: interface for the CXMLLoggableObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGGABLEOBJECT_H__620BB01B_3F74_419B_A33A_C06B74041566__INCLUDED_)
#define AFX_LOGGABLEOBJECT_H__620BB01B_3F74_419B_A33A_C06B74041566__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLElement.h"

class CXMLLoggableObject
{
public:
	CXMLLoggableObject(const char *pszClassName = "CXMLLoggableObject");

	virtual void Log(CXMLElement *pElem) const = 0;

	void LogBase(const char *pszName, const char *pszModule) const;

private:
	const char *m_pszClassName;
};

#define CONSTRUCT_LOGGABLE_OBJECT() \
	CXMLLoggableObject(GetRuntimeClass()->m_lpszClassName)

#define LOG_OBJECT(object) \
	object.LogBase(#object, THIS_FILE)

#endif // !defined(AFX_LOGGABLEOBJECT_H__620BB01B_3F74_419B_A33A_C06B74041566__INCLUDED_)
