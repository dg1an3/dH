// CoordSys.h: interface for the CCoordSys class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COORDSYS_H__768EA467_1AD6_43DC_8DCA_D28D8C67C61F__INCLUDED_)
#define AFX_COORDSYS_H__768EA467_1AD6_43DC_8DCA_D28D8C67C61F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ModelObject.h>
#include <IField.h>

class CCoordSys : public CModelObject
{
public:
	CCoordSys();
	virtual ~CCoordSys();

// ICoordSys interface methods
	STDMETHOD(GetTransformTo)(CCoordSys *pTo, IField **pField);
	STDMETHOD(SetTransformTo)(CCoordSys *pTo, IField *pField);
	STDMETHOD(GetTransformFrom)(CCoordSys *pTo, IField **pField);

private:
	// mapping of fields to other CSs
	CMapPtrToPtr m_mapToCS;
};

#endif // !defined(AFX_COORDSYS_H__768EA467_1AD6_43DC_8DCA_D28D8C67C61F__INCLUDED_)
