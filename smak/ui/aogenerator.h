#pragma once

#include <glgui/movablepanel.h>
#include "crunch/ao.h"

class CAOPanel : public glgui::CMovablePanel, public IWorkListener
{
	DECLARE_CLASS(CAOPanel, glgui::CMovablePanel);

public:
							CAOPanel(bool bColor, CConversionScene* pScene);

public:
	virtual void			CreateControls(CResource<glgui::CBaseControl> pThis);

	virtual void			SetVisible(bool bVisible);

	virtual void			Layout();

	virtual bool			KeyPressed(int iKey);

	virtual void			BeginProgress();
	virtual void			SetAction(const tstring& sAction, size_t iTotalProgress);
	virtual void			WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void			EndProgress();

	virtual bool			IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool			DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CAOPanel, Generate);
	EVENT_CALLBACK(CAOPanel, SaveMapDialog);
	EVENT_CALLBACK(CAOPanel, SaveMapFile);
	EVENT_CALLBACK(CAOPanel, AOMethod);

	virtual void			FindBestRayFalloff();

	static void				Open(CConversionScene* pScene);
	static glgui::CControl<CAOPanel>	Get();

protected:
	bool					m_bColor;

	CConversionScene*		m_pScene;

	CAOGenerator			m_oGenerator;

	glgui::CControl<glgui::CLabel>			m_hSizeLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hSizeSelector;

	glgui::CControl<glgui::CLabel>			m_hEdgeBleedLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hEdgeBleedSelector;

	glgui::CControl<glgui::CLabel>			m_hAOMethodLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hAOMethodSelector;

	glgui::CControl<glgui::CLabel>			m_hRayDensityLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hRayDensitySelector;

	glgui::CControl<glgui::CLabel>			m_hLightsLabel;
	glgui::CControl<glgui::CScrollSelector<int>>	m_hLightsSelector;

	glgui::CControl<glgui::CLabel>			m_hFalloffLabel;
	glgui::CControl<glgui::CScrollSelector<float>>	m_hFalloffSelector;

	glgui::CControl<glgui::CLabel>			m_hRandomLabel;
	glgui::CControl<glgui::CCheckBox>		m_hRandomCheckBox;

	glgui::CControl<glgui::CLabel>			m_hCreaseLabel;
	glgui::CControl<glgui::CCheckBox>		m_hCreaseCheckBox;

	glgui::CControl<glgui::CLabel>			m_hGroundOcclusionLabel;
	glgui::CControl<glgui::CCheckBox>		m_hGroundOcclusionCheckBox;

	glgui::CControl<glgui::CButton>			m_hGenerate;
	glgui::CControl<glgui::CButton>			m_hSave;

	static CResource<CBaseControl>		s_pAOPanel;
	static CResource<CBaseControl>		s_pColorAOPanel;
};
