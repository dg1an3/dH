// Source.h: interface for the CSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_)
#define AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSource  
{
public:
	CSource();
	virtual ~CSource();

	void ReadDoseSpread(const char *pszFileName);

	void EnergyLookup();

	double InterpEnergy(
		const double bound_1,				// inner boundary
		const double bound_2,				// outer boundary
		const int rad_numb,					// radial label of inner voxel
		const int phi_numb					// angular label of voxel
		);

	int m_numphi;
	int m_numrad;

	double m_ang[48];				// zenith angle list           		// (48)
	double m_rad_bound[49];		// (0:48)
	double m_inc_energy[48][49];	// (48,0:48)

	double m_cum_energy[48][601];	// (48,0:600)
};

#endif // !defined(AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_)
