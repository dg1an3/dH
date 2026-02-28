// SeriesDicomImporter.cpp: implementation of the CSeriesDicomImporter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SeriesDicomImporter.h"

#include <Series.h>
#include <Structure.h>
#include <Volumep.h>
#include <Polygon.h>

#include <dcmetinf.h>
#include <dcdeftag.h>

#include <dcmimage.h>
#include <dcpixel.h>

#include <didocu.h>
#include <dimo1img.h>
#include <dimo2img.h>

#include <algorithm>

#define CHK_DCM(x) \
if (!(x.good()))	\
{					\
	ASSERT(FALSE);	\
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSeriesDicomImporter::CSeriesDicomImporter(CSeries *pSeries, CFileDialog *pDlg)
: m_pSeries(pSeries),
	m_pDlg(pDlg),
	m_posFile(NULL),
	m_bVolumeFormatted(FALSE),
	m_nCount(0)
{
   m_posFile = m_pDlg->GetStartPosition();
}

CSeriesDicomImporter::~CSeriesDicomImporter()
{
	// delete objects
	while (m_arrImageItems.size() > 0)
	{
		delete m_arrImageItems.back();
		m_arrImageItems.pop_back();
	}
}


int CSeriesDicomImporter::ProcessNext()
{
	if (m_posFile != NULL)
	{
		// get next filename
		CString strFilepath = m_pDlg->GetNextPathName(m_posFile);

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
	else if (m_arrImageItems.size() > 0)
	{
		ResampleNextDicomImage();
	}
	else
	{
		return -1;
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
		m_pSeries->m_pDens->SetDimensions(
			pItem->m_nWidth, 
			pItem->m_nHeight, 
			m_arrImageItems.size());

		// determine slice spacing
		CMatrixD<4> mBasis;
		mBasis[0] = ToHG(pItem->m_vOffset[0]);
		mBasis[0][3] = 0.0;
		mBasis[1] = ToHG(pItem->m_vOffset[1]);
		mBasis[1][3] = 0.0;

		// calculate slice spacing
		CDicomImageItem *pItem2 = m_arrImageItems[1];
		mBasis[2] = ToHG(pItem2->m_vImgPos - pItem->m_vImgPos);
		mBasis[2][3] = 0.0;

		// calculate origin
		mBasis[3] = ToHG(pItem->m_vImgPos);

		m_pSeries->m_pDens->SetBasis(mBasis);

		m_bVolumeFormatted = TRUE;
	}
}


template<class VOXEL_TYPE>
BOOL convertVoxels(VOXEL_TYPE *pOrigVoxels, int nSize, 
				   VOXEL_REAL *pNewVoxels, Float64 slope = 1.0, Float64 intercept = 0.0)
{
	for (int nAt = 0; nAt < nSize; nAt++)
	{
		pNewVoxels[nAt] = (VOXEL_REAL) (slope * pOrigVoxels[nAt] + intercept);
	}

	return TRUE;
}


void CSeriesDicomImporter::ResampleNextDicomImage()
{
	DcmDataset *pDataset = m_arrImageItems.back()->m_pFileFormat->getDataset();
	DcmMetaInfo *pMetaInfo = m_arrImageItems.back()->m_pFileFormat->getMetaInfo();

	FormatVolume();
	VOXEL_REAL ***pppDstPixelData = m_pSeries->m_pDens->GetVoxels();

	int nSlice = m_arrImageItems.size()-1;

	// now open image
	DicomImage *pImage = new DicomImage(pDataset, pMetaInfo->getOriginalXfer());
	
	if (pImage != NULL)
	{
		if (pImage->getStatus() == EIS_Normal)
		{
			
// #define USE_DicomImage
#ifdef USE_DicomImage

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
								1.0 / 48000.0 
									* (VOXEL_REAL) pSrcPixelData[nAtY * pImage->getWidth() + nAtX];
						}
					}
				}
			}
#else

			DcmElement *pElem = NULL;
			CHK_DCM(pDataset->findAndGetElement(DCM_PixelData, pElem));

			DcmPixelData *pDPD = OFstatic_cast(DcmPixelData *, pElem);
			CHK_DCM(pDPD->loadAllDataIntoMemory());

			Uint16 nPR = 0;
			CHK_DCM(pDataset->findAndGetUint16(DCM_PixelRepresentation, nPR));

			ASSERT(pImage->getWidth() % 4 == 0);
			ASSERT(pImage->getWidth() == m_pSeries->m_pDens->GetWidth());

			int nSize = pImage->getHeight() * pImage->getWidth();

			Float64 slope = 0.0;
			CHK_DCM(pDataset->findAndGetFloat64(DCM_RescaleSlope, slope));

			Float64 intercept = 0.0;
			CHK_DCM(pDataset->findAndGetFloat64(DCM_RescaleIntercept, intercept));

			if (pDPD->getVR() == EVR_OW)
			{
				Uint16 *wordVals = NULL;
				CHK_DCM(pDPD->getUint16Array(wordVals));

				if (nPR == 0)	// unsigned
				{
					convertVoxels(wordVals, nSize, &pppDstPixelData[0][0][0], slope, intercept);
				}
				else
				{
					Sint16 *swordVals = (Sint16 *) wordVals;
					convertVoxels(swordVals, nSize, &pppDstPixelData[0][0][0], slope, intercept);
				}
			}
			else if (pDPD->getVR() == EVR_OB)
			{
				Uint8 *byteVals = NULL;
				CHK_DCM(pDPD->getUint8Array(byteVals));

				if (nPR == 0)	// unsigned
				{
					convertVoxels(byteVals, nSize, &pppDstPixelData[0][0][0], slope, intercept);
				}
				else
				{
					Sint8 *sbyteVals = (Sint8 *) byteVals;
					convertVoxels(sbyteVals, nSize, &pppDstPixelData[0][0][0], slope, intercept);
				}
			}

			m_pSeries->m_pDens->VoxelsChanged();
			TRACE("Converted image min = %f", (float) m_pSeries->m_pDens->GetMin());
			TRACE("Converted image max = %f", (float) m_pSeries->m_pDens->GetMax());
#endif
		} 
		else
		{
			cerr << "Error: cannot load DICOM image (" << DicomImage::getString(pImage->getStatus()) << ")" << endl;
		}

		delete pImage;
	}

	// now remove image from vector
	delete m_arrImageItems.back();
	m_arrImageItems.pop_back();
}

CSeriesDicomImporter::CDicomImageItem::CDicomImageItem(DcmFileFormat *pFileFormat)
: m_pFileFormat(pFileFormat)
{
	DcmDataset *pDataset = m_pFileFormat->getDataset();

	OFString strImgPositionPatient;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_ImagePositionPatient, strImgPositionPatient));

	sscanf_s(strImgPositionPatient.c_str(), REAL_FMT "\\" REAL_FMT "\\" REAL_FMT,
		&m_vImgPos[0], &m_vImgPos[1], &m_vImgPos[2]);
#define FIX_IMAGE_POS
#ifdef FIX_IMAGE_POS
	m_vImgPos[0] *= 10.0;
	m_vImgPos[1] *= 10.0;
#endif

	OFString strImgOrientPatient;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_ImageOrientationPatient, strImgOrientPatient));
	sscanf_s(strImgOrientPatient.c_str(), 
		REAL_FMT "\\" REAL_FMT "\\" REAL_FMT "\\" REAL_FMT "\\" REAL_FMT "\\" REAL_FMT,
		&m_vOffset[0][0], &m_vOffset[0][1], &m_vOffset[0][2],
		&m_vOffset[1][0], &m_vOffset[1][1], &m_vOffset[1][2]);

	OFString strPixelSpacing;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_PixelSpacing, strPixelSpacing));

	float vPixSpacing[2];
	sscanf_s(strPixelSpacing.c_str(), "%f\\%f", 
		&vPixSpacing[0], &vPixSpacing[1]);

	m_vOffset[0] *= vPixSpacing[0];
	m_vOffset[1] *= vPixSpacing[1];

	OFString strHeight;
	CHK_DCM(pDataset->findAndGetOFString(DCM_Rows, strHeight));
	sscanf_s(strHeight.c_str(), "%i", &m_nHeight);

	OFString strWidth;
	CHK_DCM(pDataset->findAndGetOFString(DCM_Rows, strWidth));
	sscanf_s(strWidth.c_str(), "%i", &m_nWidth);
}

void CSeriesDicomImporter::CDicomImageItem::CalcNormalizedSliceOffset(CDicomImageItem *pPrev)
{
	// compute raw difference
	m_vOffset[2] = m_vImgPos - pPrev->m_vImgPos;
	m_vOffset[2].Normalize();

	// adjust handedness
	if (m_vOffset[2] * Cross(m_vOffset[0], m_vOffset[1]) < 0)
	{
		m_vOffset[2] = (REAL) -1.0 * m_vOffset[2];
	}

	// compare to prev slice offset
	if (pPrev->m_vOffset[2].GetLength() == 0.0)
	{
		pPrev->m_vOffset[2] = m_vOffset[2];
	}
}

int CSeriesDicomImporter::CDicomImageItem::CompareSlicePosition(CDicomImageItem *pFirst, 
			CDicomImageItem *pSecond)
{
	ASSERT(pFirst->m_vOffset[2].IsApproxEqual(pSecond->m_vOffset[2]));

	REAL pos1 = pFirst->m_vImgPos * pFirst->m_vOffset[2];
	REAL pos2 = pSecond->m_vImgPos * pSecond->m_vOffset[2];

	return (pos1 < pos2); 
}

void CSeriesDicomImporter::ImportDicomStructureSet(DcmFileFormat *pFileFormat)
{
	DcmDataset *pDataset = pFileFormat->getDataset();

	CTypedPtrArray< CObArray, CStructure * > arrROIs;

	DcmSequenceOfItems *pSSROISequence = NULL;
	CHK_DCM(pDataset->findAndGetSequence(DCM_StructureSetROISequence, pSSROISequence));
	for (int nAtROI = 0; nAtROI < (int) pSSROISequence->card(); nAtROI++)
	{
		DcmItem *pROIItem = pSSROISequence->getItem(nAtROI);

		long nROINumber;
		CHK_DCM(pROIItem->findAndGetSint32(DCM_ROINumber, nROINumber));

		OFString strName;
		CHK_DCM(pROIItem->findAndGetOFString(DCM_ROIName, strName));

		CStructure *pStruct = new CStructure(strName.c_str());

		// add to ROI list
		arrROIs.SetAtGrow(nROINumber, pStruct);

		// add structure to series
		m_pSeries->AddStructure(pStruct);
	}

	DcmSequenceOfItems *pROIContourSequence = NULL;
	CHK_DCM(pDataset->findAndGetSequence(DCM_ROIContourSequence, pROIContourSequence));
	for (int nAtROIContour = 0; nAtROIContour < (int) pROIContourSequence->card(); nAtROIContour++)
	{
		DcmItem *pROIContourItem = pROIContourSequence->getItem(nAtROIContour);

		long nRefROINumber = 0;
		CHK_DCM(pROIContourItem->findAndGetSint32(DCM_ReferencedROINumber, nRefROINumber));
		CStructure *pStruct = arrROIs[nRefROINumber];
		ASSERT(pStruct != NULL);

		OFString strColor;
		CHK_DCM(pROIContourItem->findAndGetOFStringArray(DCM_ROIDisplayColor, strColor));
		
		int nRed, nGrn, nBlu;
		sscanf_s(strColor.c_str(), "%i\\%i\\%i", &nRed, &nGrn, &nBlu);
		pStruct->SetColor(RGB(nRed, nGrn, nBlu));

		DcmSequenceOfItems *pContourSequence = NULL;
		CHK_DCM(pROIContourItem->findAndGetSequence(DCM_ContourSequence, pContourSequence));
		for (int nAtContour = 0; nAtContour < (int) pContourSequence->card(); nAtContour++)
		{
			DcmItem *pContourItem = pContourSequence->getItem(nAtContour);

			OFString strContourGeometricType;
			CHK_DCM(pContourItem->findAndGetOFString(DCM_ContourGeometricType, strContourGeometricType));
			if (strContourGeometricType == "CLOSED_PLANAR")
			{
				long nContourPoints;
				CHK_DCM(pContourItem->findAndGetSint32(DCM_NumberOfContourPoints, nContourPoints));

				OFString strContourData;
				CHK_DCM(pContourItem->findAndGetOFStringArray(DCM_ContourData, strContourData));

				CPolygon *pPoly = new CPolygon();

				int nStart = 0;
				int nNext = 0;
				float slice_z = 0.0;
				for (int nAtPoint = 0; nAtPoint < nContourPoints; nAtPoint++)
				{
					// now parse contour data, one vertex at a time
					nNext = strContourData.find('\\', nStart);
					float coord_x;
					sscanf_s(strContourData.substr(nStart, nNext-nStart).c_str(), "%f", &coord_x);
					nStart = nNext+1;

					nNext = strContourData.find('\\', nStart);
					float coord_y;
					sscanf_s(strContourData.substr(nStart, nNext-nStart).c_str(), "%f", &coord_y);
					nStart = nNext+1;

					nNext = strContourData.find('\\', nStart);
					float coord_z;
					sscanf_s(strContourData.substr(nStart, nNext-nStart).c_str(), "%f", &coord_z);
					nStart = nNext+1;

					if (nAtPoint == 0)
					{
						slice_z = coord_z;
					}
					ASSERT(IsApproxEqual(coord_z, slice_z));

					CVectorD<2> vVert(coord_x, coord_y);
#define FIX_IMAGE_POS
#ifdef FIX_IMAGE_POS
					vVert[0] += 25.6 - 256.0;
					vVert[1] += 25.6 - 256.0;
#endif
					pPoly->AddVertex(vVert);
				}

				pStruct->AddContour(pPoly, slice_z);
			}
		}
	}

}


