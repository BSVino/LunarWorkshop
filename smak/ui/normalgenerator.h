#pragma once

#include <glgui/movablepanel.h>

#include "crunch/normal.h"

class CNormalPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CNormalPanel, glgui::CMovablePanel);

public:
								CNormalPanel(CConversionScene* pScene);

public:
	virtual void				CreateControls(CResource<glgui::CBaseControl> pThis);

	virtual void				SetVisible(bool bVisible);

	virtual void				Layout();
	virtual void				UpdateScene();

	virtual void				Think();

	virtual void				Paint(float x, float y, float w, float h);

	virtual bool				KeyPressed(int iKey);

	virtual bool				IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool				DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CNormalPanel,	Generate);
	EVENT_CALLBACK(CNormalPanel,	SaveMapDialog);
	EVENT_CALLBACK(CNormalPanel,	SaveMapFile);
	EVENT_CALLBACK(CNormalPanel,	SetupNormal2);
	EVENT_CALLBACK(CNormalPanel,	UpdateNormal2);

	static void					Open(CConversionScene* pScene);
	static glgui::CControl<CNormalPanel>		Get() { return glgui::CControl<CNormalPanel>(s_pNormalPanel->GetHandle()); }

protected:
	CConversionScene*			m_pScene;

	CNormalGenerator			m_oGenerator;

	glgui::CControl<glgui::CLabel>				m_hMaterialsLabel;
	glgui::CControl<glgui::CTree>				m_hMaterials;

	glgui::CControl<glgui::CLabel>				m_hProgressLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hDepthLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hHiDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hHiDepthLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hMidDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hMidDepthLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_hLoDepthSelector;
	glgui::CControl<glgui::CLabel>				m_hLoDepthLabel;

	glgui::CControl<glgui::CButton>				m_hSave;

	glgui::CControl<class CMaterialPicker>		m_hMaterialPicker;

	static CResource<CBaseControl>		s_pNormalPanel;
};
