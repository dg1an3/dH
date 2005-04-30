// SeriesDicomImporter.h: interface for the CSeriesDicomImporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIESDICOMIMPORTER_H__718F0516_B419_4DF9_BFF5_5A5578C638AD__INCLUDED_)
#define AFX_SERIESDICOMIMPORTER_H__718F0516_B419_4DF9_BFF5_5A5578C638AD__INCLUDED_

#include <dcfilefo.h>

#include <VectorD.h>

#include <vector>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSeries;

class CSeriesDicomImporter  
{
public:
	CSeriesDicomImporter(CSeries *pSeries, CFileDialog *pDlg);
	virtual ~CSeriesDicomImporter();


	int ProcessNext();

	void PreparseDicomImage(DcmFileFormat *pFileFormat);
	void SortDicomImages();

	void FormatVolume();

	void ResampleNextDicomImage();

	void ImportDicomStructureSet(DcmFileFormat *pFileFormat);

private:
	CSeries *m_pSeries;

	int m_nCount;

	CFileDialog *m_pDlg;
	POSITION m_posFile;

	class CDicomImageItem
	{
	public:
		CDicomImageItem(DcmFileFormat *pFileFormat);
		virtual ~CDicomImageItem() 
		{
			delete m_pFileFormat;
		}

		void CalcNormalizedSliceOffset(CDicomImageItem *pPrev);

		static int CompareSlicePosition(CDicomImageItem *pFirst, 
			CDicomImageItem *pSecond);

		DcmFileFormat *m_pFileFormat;

		CVectorD<3> m_vImgPos;

		int m_nHeight;
		int m_nWidth;

		CVectorD<3> m_vOffset[3];
	};

	vector< CDicomImageItem* > m_arrImageItems;

	BOOL m_bVolumeFormatted;

};

#endif // !defined(AFX_SERIESDICOMIMPORTER_H__718F0516_B419_4DF9_BFF5_5A5578C638AD__INCLUDED_)
