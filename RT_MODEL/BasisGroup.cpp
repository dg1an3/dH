
class CBasisGroup : public CModelObject
{
public:
	CBasisGroup();
	CBasisGroup(CBasisGroup *pParent, CVolume<REAL> *pRegion);
	virtual ~CBasisGroup();

	void AddVolume(CVolume<REAL> *pBasis);
	CTypedPtrArray< CArray, CVolume<REAL>* >& GetVolumes();

	const CVectorN<>& GetWeights() const;
	void SetWeights(const CVectorN<>& vWeights);

	const CVolume<REAL> *GetSumVolume() const;

protected:
	void OnRegionChanged();

private:
	CTypedPtrArray< CArray, CVolume<REAL>* > m_arrVolumes;

	CVectorN<> m_vWeights;

	CBasisGroup *m_pParent;
	CVolume<REAL> *m_pRegion;
	CVolume<REAL> *m_pConfRegion;
	BOOL m_bRecalcConfRegion;

	BOOL m_bRecalcRegionVolumes;

	BOOL m_bRecalcSumVolume;
	CVolume<REAL> m_sumVolume;
};

CBasisGroup::CBasisGroup()
: m_pRegion(NULL),
	m_bRecalcSumVolume(TRUE)
{
}

CBasisGroup::CBasisGroup(CBasisGroup *pParent, CVolume<REAL> *pRegion);
: m_pRegion(pRegion),
	m_bRecalcRegionVolumes(TRUE),
	m_bRecalcConfRegion(TRUE),
	m_bRecalcSumVolume(TRUE)
{
	pRegion->GetChangeEvent().AddListener(this);
}

CBasisGroup::~CBasisGroup()
{
}

void CBasisGroup::AddVolume(CVolume<REAL> *pBasis)
{
	ASSERT(!m_pParent);

	m_arrVolumes.Add(pBasis);

	// CVectorN<> vTemp = m_vWeights;
	m_vWeights.SetDim(m_arrVolumes.GetSize()+1);
	m_vWeights[m_vWeights.GetDim()-1] = 0.0;
	// m_vWeights.CopyElements(vTemp, 0, m_vWeights);

	// TODO: set up listener

	m_bRecalcSumVolume = TRUE;
}

CTypedPtrArray< CArray, CVolume<REAL>* >& CBasisGroup::GetVolumes()
{
	if (m_pRegion && m_bRecalcRegionVolumes)
	{
		// window volumes
		for (int nAt = 0; nAt < m_pParent->GetVolumes().GetSize(); nAt++)
		{
			if (m_arrVolumes.GetSize() < nAt)
			{
				m_arrVolumes.Add(new CVolume<REAL>(m_pParent->GetVolumes()[nAt]);
			}
			else
			{
				m_arrVolumes[nAt] = m_pParent->GetVolumes()[nAt];
			}

			if (!m_pConfRegion)
			{
				// set up conformant region
				m_pConfRegion = new CVolume<REAL>();
				m_pConfRegion->ConformTo(pWinVolume);
			}

			if (m_bRecalcConfRegion)
			{
				Resample(m_pRegion, m_pConfRegion);
				m_bRecalcConfRegion = FALSE;
			}

			m_arrVolumes[nAt].Mult(m_pCongRegion);
		}

		m_bRecalcRegionVolumes = FALSE;
	}

	return m_arrVolumes;
}

const CVectorN<>& CBasisGroup::GetWeights() const
{
	if (m_pParent)
	{
		return m_pParent->GetWeights();
	}

	return m_vWeights;
}

void CBasisGroup::SetWeights(const CVectorN<>& vWeights)
{
	m_vWeights = vWeights;
	m_bRecalcSumVolume = TRUE;
}

const CVolume<REAL> *CBasisGroup::GetSumVolume() const
{
	if (m_bRecalcSumVolume)
	{
		m_sumVolume.ConformTo(m_arrVolumes[0]);
		m_sumVolume.SetZero();

		const CVectorN<>& vWeights = GetWeights();
		for (int nAt = 0; nAt < m_arrVolumes.GetSize(); nAt++)
		{
			m_sumVolume.Accumulate(m_arrVolumes[nAt], vWeights[nAt]);
		}

		m_bRecalcSumVolume = FALSE;
	}

	return &m_sumVolume;
}

void CBasisGroup::OnRegionChanged()
{
	m_bRecalcRegionVolumes = TRUE;
	m_bRecalcConfRegion = TRUE;
	m_bRecalcSumVolume = TRUE;
}


