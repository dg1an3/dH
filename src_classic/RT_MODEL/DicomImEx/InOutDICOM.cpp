// InOutDICOM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "InOutDICOM.h"

#include <dcfilefo.h>
#include <dcmetinf.h>
#include <dcdeftag.h>

#include <dcmimage.h>

#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CHK_DCM(x) \
if (!(x.good()))	\
{					\
	ASSERT(FALSE);	\
}

class CStructure
{
public: 
	CStructure(CString strName) { }
};

class CVolume
{
public:
	void SetDimensions(int nWidth, int nHeight, int nDepth) { }
	float ***GetVoxels() { return NULL; }
};

class CSeries
{
public:
	void AddStructure(CStructure *pStruct) { }
	CVolume *GetVolume() { return NULL; }
};

typedef float CVectorD3[3];

class CSeriesDicomImporter
{
public:
	CSeriesDicomImporter(CSeries *pSeries, const CString& strDirectory)
		: m_pSeries(pSeries),
		m_strDirectory(strDirectory),
		m_bReadingItems(TRUE),
		m_bVolumeFormatted(FALSE),
		m_nCount(0)
	{
	   m_bReadingItems = m_finder.FindFile(strDirectory + "\\*.*");
	}

	virtual ~CSeriesDicomImporter() 
	{ 
		// delete objects
	}

	int ProcessNext();

	void PreparseDicomImage(DcmFileFormat *pFileFormat);
	void SortDicomImages();

	void FormatVolume();

	void ResampleNextDicomImage();

	void ImportDicomStructureSet(DcmFileFormat *pFileFormat);

private:
	CSeries *m_pSeries;

	int m_nCount;

	CString m_strDirectory;
	CFileFind m_finder;
	BOOL m_bReadingItems;

	class CDicomImageItem
	{
	public:
		CDicomImageItem(DcmFileFormat *pFileFormat);

		void CalcNormalizedSliceOffset(CDicomImageItem *pPrev);

		static int CompareSlicePosition(CDicomImageItem *pFirst, 
			CDicomImageItem *pSecond);

		DcmFileFormat *m_pFileFormat;

		CVectorD3 m_vImgPos;

		int m_nHeight;
		int m_nWidth;

		CVectorD3 m_vOffset[3];
	};

	vector< CDicomImageItem* > m_arrImageItems;

	BOOL m_bVolumeFormatted;
};

int CSeriesDicomImporter::ProcessNext()
{
	if (m_bReadingItems)
	{
		m_bReadingItems = m_finder.FindNextFile();
		if (!m_finder.IsDirectory())
		{
			// get next filename
			CString strFilepath = m_finder.GetFilePath();

			// open file
			DcmFileFormat *pFileFormat = new DcmFileFormat();
			if (pFileFormat->loadFile(strFilepath).good())
			{
				OFString strModality;
				CHK_DCM(pFileFormat->getDataset()->findAndGetOFString(DCM_Modality, strModality));

				if (strModality == "CT")
				{
					PreparseDicomImage(pFileFormat);
				}
				else if (strModality == "RTSTRUCT")
				{
					ImportDicomStructureSet(pFileFormat);
					delete pFileFormat;
				}
			}
			else
			{
				delete pFileFormat;
			}
		}
	}
	else if (m_arrImageItems.size() > 0)
	{
		ResampleNextDicomImage();
	}

	return ++m_nCount;
}

void CSeriesDicomImporter::PreparseDicomImage(DcmFileFormat *pFileFormat)
{
	CDicomImageItem *pItem = new CDicomImageItem(pFileFormat);

	// compare vOffsets to common offsets

	// calc slice position
	if (m_arrImageItems.size() > 0)
	{
		pItem->CalcNormalizedSliceOffset(m_arrImageItems.back());
	}
	m_arrImageItems.push_back(pItem);
}

void CSeriesDicomImporter::SortDicomImages()
{
	sort(m_arrImageItems.begin(), m_arrImageItems.end(), 
		CDicomImageItem::CompareSlicePosition);
}

void CSeriesDicomImporter::FormatVolume()
{
	if (!m_bVolumeFormatted)
	{
		CDicomImageItem *pItem = m_arrImageItems[0];

		// determine dimensions
		m_pSeries->GetVolume()->SetDimensions(
			pItem->m_nWidth, 
			pItem->m_nHeight, 
			m_arrImageItems.size());

		// determine slice spacing
/*		CMatrixD4 mBasis;
		mBasis[0] = ToHG(pItem->m_vOffset[0]);
		mBasis[1] = ToHG(pItem->m_vOffset[1]);

		// calculate slice spacing
		CDicomImageItem *pItem2 = m_arrImageItems[1];
		mBasis[2] = ToHG(pItem2->m_vImgPos - pItem->m_vImgPos);

		// calculate origin
		mBasis[3] = ToHG(pItem->m_vImgPos);

		m_pSeries->GetVolume()->SetBasis(mBasis);
*/
		m_bVolumeFormatted = FALSE;
	}
}


void CSeriesDicomImporter::ResampleNextDicomImage()
{
	DcmDataset *pDataset = m_arrImageItems.back()->m_pFileFormat->getDataset();
	DcmMetaInfo *pMetaInfo = m_arrImageItems.back()->m_pFileFormat->getMetaInfo();

	FormatVolume();
	float ***pppDstPixelData = m_pSeries->GetVolume()->GetVoxels();

	int nSlice = m_arrImageItems.size()-1;

	// now open image
	DicomImage *pImage = new DicomImage(pDataset, pMetaInfo->getOriginalXfer());
	
	if (pImage != NULL)
	{
		if (pImage->getStatus() == EIS_Normal)
		{
			if (pImage->isMonochrome())
			{
				pImage->setWindow(1024.0, 4096.0);

				Uint16 *pSrcPixelData = (Uint16 *)(pImage->getOutputData(16));
				if (pSrcPixelData != NULL)
				{
					for (int nAtY = 0; nAtY < pImage->getHeight(); nAtY++)
					{
						for (int nAtX = 0; nAtX < pImage->getWidth(); nAtX++)
						{
							pppDstPixelData[nSlice][nAtY][nAtX] = 
								2.0 / 16384.0 
									* (float) pSrcPixelData[nAtY * pImage->getWidth() + nAtX];
						}
					}
				}
			}
		} 
		else
		{
			cerr << "Error: cannot load DICOM image (" << DicomImage::getString(pImage->getStatus()) << ")" << endl;
		}
	}

	// now remove image from vector
	m_arrImageItems.pop_back();
}

CSeriesDicomImporter::CDicomImageItem::CDicomImageItem(DcmFileFormat *pFileFormat)
: m_pFileFormat(pFileFormat)
{
	DcmDataset *pDataset = m_pFileFormat->getDataset();

	OFString strImgPositionPatient;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_ImagePositionPatient, strImgPositionPatient));
	sscanf(strImgPositionPatient.c_str(), "%f\\%f\\%f", 
		&m_vImgPos[0], &m_vImgPos[1], &m_vImgPos[2]);

	OFString strImgOrientPatient;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_ImageOrientationPatient, strImgOrientPatient));
	sscanf(strImgOrientPatient.c_str(), "%f\\%f\\%f\\%f\\%f\\%f", 
		&m_vOffset[0][0], &m_vOffset[0][1], &m_vOffset[0][2],
		&m_vOffset[1][0], &m_vOffset[1][1], &m_vOffset[1][2]);

	OFString strPixelSpacing;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_PixelSpacing, strPixelSpacing));

	float vPixSpacing[2];
	sscanf(strPixelSpacing.c_str(), "%f\\%f", 
		&vPixSpacing[0], &vPixSpacing[1]);

//	m_vOffset[0] *= vPixSpacing[0];
//	m_vOffset[1] *= vPixSpacing[1];

	OFString strHeight;
	CHK_DCM(pDataset->findAndGetOFString(DCM_Rows, strHeight));
	sscanf(strHeight.c_str(), "%i", &m_nHeight);

	OFString strWidth;
	CHK_DCM(pDataset->findAndGetOFString(DCM_Rows, strWidth));
	sscanf(strWidth.c_str(), "%i", &m_nWidth);
}

void CSeriesDicomImporter::CDicomImageItem::CalcNormalizedSliceOffset(CDicomImageItem *pPrev)
{
	// compute raw difference
/*	m_vOffset[2] = m_vImgPos - pPrev->m_vImgPos;
	m_vOffset[2] .Normalize();

	// adjust handedness
	if (m_vOffset[2] * m_vOffset[0].Cross(m_vOffset[1]) < 0)
	{
		m_vOffset[2] = -m_vOffset[2] ;
	}

	// compare to prev slice offset
	if (pPrev->m_vOffset[2].GetLength() == 0.0)
	{
		pPrev->m_vOffset[2] = m_vOffset[2];
	} */
}

int CSeriesDicomImporter::CDicomImageItem::CompareSlicePosition(CDicomImageItem *pFirst, 
			CDicomImageItem *pSecond)
{
/*	ASSERT(pFirst->m_vOffset[2].IsApproxEqual(pSecond->m_vOffset[2]));

	float pos1 = pFirst->m_vImgPos * pFirst->m_vOffset[2];
	float pos2 = pSecond->m_vImgPos * pSecond->m_vOffset[2];

	return (pos1 < pos2); */
	return FALSE;
}

void CSeriesDicomImporter::ImportDicomStructureSet(DcmFileFormat *pFileFormat)
{
	DcmDataset *pDataset = pFileFormat->getDataset();

	vector< CStructure * > arrROIs;

	DcmSequenceOfItems *pSSROISequence = NULL;
	CHK_DCM(pDataset->findAndGetSequence(DCM_StructureSetROISequence, pSSROISequence));
	for (int nAtROI = 0; nAtROI < pSSROISequence->card(); nAtROI++)
	{
		DcmItem *pROIItem = pSSROISequence->getItem(nAtROI);

		unsigned short nROINumber;
		CHK_DCM(pROIItem->findAndGetUint16(DCM_ROINumber, nROINumber));

		OFString strName;
		CHK_DCM(pROIItem->findAndGetOFString(DCM_ROIName, strName));

		CStructure *pStruct = new CStructure(strName.c_str());

		// add to ROI list
		arrROIs.resize(nROINumber);
		arrROIs[nROINumber] = pStruct;

		// add structure to series
		m_pSeries->AddStructure(pStruct);
	}

	DcmSequenceOfItems *pROIContourSequence = NULL;
	CHK_DCM(pDataset->findAndGetSequence(DCM_StructureSetROISequence, pROIContourSequence));
	for (int nAtROIContour = 0; nAtROIContour< pROIContourSequence->card(); nAtROIContour++)
	{
		DcmItem *pROIContourItem = pROIContourSequence->getItem(nAtROIContour);

		unsigned short nRefROINumber;
		CHK_DCM(pROIContourItem->findAndGetUint16(DCM_ReferencedROINumber, nRefROINumber));
		CStructure *pStruct = arrROIs[nRefROINumber];

		DcmSequenceOfItems *pContourSequence = NULL;
		CHK_DCM(pROIContourItem->findAndGetSequence(DCM_ContourSequence, pContourSequence));
		for (int nAtContour = 0; nAtContour< pContourSequence->card(); nAtContour++)
		{
			DcmItem *pContourItem = pContourSequence->getItem(nAtContour);

			OFString strContourGeometricType;
			CHK_DCM(pContourItem->findAndGetOFString(DCM_ContourGeometricType, strContourGeometricType));
			ASSERT(strContourGeometricType == "CLOSED_PLANAR");
			USHORT nContourPoints;
			CHK_DCM(pContourItem->findAndGetUint16(DCM_NumberOfContourPoints, nContourPoints));

			OFString strContourData;
			CHK_DCM(pContourItem->findAndGetOFStringArray(DCM_ContourData, strContourData));

			//  CPolygon *pPoly = new CPolygon();

			int nStart = 0;
			for (int nAtPoint = 0; nAtPoint < nContourPoints; nAtPoint++)
			{
				// now parse contour data, one vertex at a time
				int nNext = strContourData.find('\\', nStart);

				float coord_x;
				sscanf(strContourData.substr(nStart, nNext-nStart).c_str(), "%f", &coord_x);
				nStart = nNext+1;

				float coord_y;
				sscanf(strContourData.substr(nStart, nNext-nStart).c_str(), "%f", &coord_y);
				nStart = nNext+1;

				float coord_z;
				sscanf(strContourData.substr(nStart, nNext-nStart).c_str(), "%f", &coord_z);
				nStart = nNext+1;

				// pPoly->AddVertex(CVectorD3(coord_x, coord_y, coord_z));
			}
		}
	}

}


/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
		cout << (LPCTSTR)strHello << endl;

		CSeries series;
		CSeriesDicomImporter(&series, "");
	}

	return nRetCode;
}



