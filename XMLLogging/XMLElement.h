// XMLElement.h: interface for the CXMLElement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLELEMENT_H__519C6A5B_22F3_496F_BC77_897FD12A2D9C__INCLUDED_)
#define AFX_XMLELEMENT_H__519C6A5B_22F3_496F_BC77_897FD12A2D9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLLogFile.h"

class CXMLElement
{
protected:
	// protected construction -- only by the log file
	friend CXMLLogFile;
	CXMLElement(CXMLLogFile *pLogFile, const char *pszLabel, const char *pszModule);

	int m_nEnabled;

public:
	// destructor
	virtual ~CXMLElement();

	// returns the module for the element
	const char *GetModule() const { return m_pszModule; }

	// sets an attribute -- 
	//		can only be called until the tag is permanently closed
	BOOL Attribute(const char *pszName, const char *pszValue);

	// closes the tag --
	//		flag indicates that the tag is not permanently closed
	BOOL CloseTag(BOOL bPermanent);

	// closes the element
	BOOL Close(BOOL bPermanent);

private:
	// pointer the the log file
	CXMLLogFile *m_pLogFile;

	// the elements label
	const char *m_pszLabel;

	// the elements module
	const char *m_pszModule;

	// flag for tag-open
	BOOL m_bTagOpen;

	// flag for element-open
	BOOL m_bElementOpen;
};


#endif // !defined(AFX_XMLELEMENT_H__519C6A5B_22F3_496F_BC77_897FD12A2D9C__INCLUDED_)
