// LightfieldTexture.h: interface for the CLightfieldTexture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIGHTFIELDTEXTURE_H__3975ABF0_9E17_4754_B532_4BDF8C21204D__INCLUDED_)
#define AFX_LIGHTFIELDTEXTURE_H__3975ABF0_9E17_4754_B532_4BDF8C21204D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Beam.h>

#include <Texture.h>

class CLightfieldTexture : public CTexture  
{
public:
	CLightfieldTexture();
	virtual ~CLightfieldTexture();

	CBeam *GetBeam();
	void SetBeam(CBeam *pBeam);

	// event handler for beam changes
	void OnBeamChanged(CObservableEvent *pEvent, void *pOldValue);

private:
	CBeam *m_pBeam;
};

#endif // !defined(AFX_LIGHTFIELDTEXTURE_H__3975ABF0_9E17_4754_B532_4BDF8C21204D__INCLUDED_)
