////////////////////////////////////////////////////////////////
// PixieLib(TM) Copyright 1997-1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#ifndef _DRAWTOOL_H
#define _DRAWTOOL_H

////////////////////////////////////////////////////////////////
// This is the magic ROP code used to generate the embossed look for
// a disabled button. It's listed in Appendix F of the Win32 Programmer's
// Reference as PSDPxax (!) which is a cryptic reverse-polish notation for
//
// ((Destination XOR Pattern) AND Source) XOR Pattern
//
// which I leave to you to figure out. In the case where I apply it,
// Source is a monochrome bitmap which I want to draw in such a way that
// the black pixels get transformed to the brush color and the white pixels
// draw transparently--i.e. leave the Destination alone.
//
// black ==> Pattern (brush)
// white ==> Destintation (ie, transparent)
//
// 0xb8074a is the ROP code that does this. For more info, see Charles
// Petzold, _Programming Windows_, 2nd Edition, p 622-624.
//
#define TRANSPARENTROP 0xb8074a

// Draw bitmap embossed with "embossed" look
class CImageList; // fwd ref

// Paint a rectangle a given color
extern DLLFUNC void PLFillRect(CDC& dc, const CRect& rc, COLORREF color);

// fill rectangle with 3d light color
extern DLLFUNC void PLFillRect3DLight(CDC& dc, const CRect& rc);

extern DLLFUNC void PLDrawEmbossed(CDC& dc, CImageList& il, int i,
	CPoint p, BOOL bColor=FALSE);

// load bitmaps, converting grays to system colors
extern DLLFUNC HBITMAP
PLLoadSysColorBitmap(LPCTSTR lpResName, BOOL bMono=FALSE);

// overload takes ID
inline HBITMAP PLLoadSysColorBitmap(UINT nResID, BOOL bMono=FALSE)
{	return PLLoadSysColorBitmap(MAKEINTRESOURCE(nResID), bMono); }
	

#endif // _DRAWTOOL_H
