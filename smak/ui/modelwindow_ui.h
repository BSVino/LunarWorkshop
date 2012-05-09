#ifndef SMAK_MODELWINDOW_UI_H
#define SMAK_MODELWINDOW_UI_H

#include <glgui/panel.h>
#include <glgui/button.h>
#include <glgui/selector.h>
#include "crunch/crunch.h"

typedef enum
{
	BA_TOP,
	BA_BOTTOM,
} buttonalignment_t;

namespace glgui
{
	class CLabel;
	class CTree;
	class CCheckBox;
}

class CButtonPanel : public glgui::CPanel
{
public:
							CButtonPanel(buttonalignment_t eAlign);

	virtual void			Layout();

	virtual void			AddButton(glgui::CButton* pButton, const tstring& sHints, bool bNewSection, glgui::IEventListener* pListener = NULL, glgui::IEventListener::Callback pfnCallback = NULL);

	virtual void			Think();
	virtual void			Paint(float x, float y, float w, float h);

protected:
	buttonalignment_t		m_eAlign;

	tvector<float>			m_aflSpaces;
	tvector<glgui::CButton*>	m_apButtons;
	tvector<glgui::CLabel*>	m_apHints;
};

class CProgressBar : public glgui::CPanel
{
public:
							CProgressBar();

public:
	void					Layout();
	void					Paint(float x, float y, float w, float h);

	void					SetTotalProgress(size_t iProgress);
	void					SetProgress(size_t iProgress, const tstring& sAction = "");
	void					SetAction(const tstring& sAction);

	static CProgressBar*	Get();

protected:
	size_t					m_iTotalProgress;
	size_t					m_iCurrentProgress;

	glgui::CLabel*			m_pAction;
	tstring					m_sAction;

	static CProgressBar*	s_pProgressBar;
};

#define HEADER_HEIGHT 16

class CCloseButton : public glgui::CButton
{
public:
							CCloseButton() : glgui::CButton(0, 0, 10, 10, "") {};

public:
	virtual void			Paint() { glgui::CButton::Paint(); };
	virtual void			Paint(float x, float y, float w, float h);
};

class CMinimizeButton : public glgui::CButton
{
public:
							CMinimizeButton() : glgui::CButton(0, 0, 10, 10, "") {};

public:
	virtual void			Paint() { glgui::CButton::Paint(); };
	virtual void			Paint(float x, float y, float w, float h);
};

class CMovablePanel : public glgui::CPanel, public glgui::IEventListener
{
public:
							CMovablePanel(const tstring& sName);
							~CMovablePanel();

	virtual void			Layout();

	virtual void			Think();

	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);
	virtual bool			MouseReleased(int iButton, int mx, int my);

	virtual void			HasCloseButton(bool bHasClose) { m_bHasCloseButton = bHasClose; };
	virtual void			Minimize();

	virtual void			SetClearBackground(bool bClearBackground) { m_bClearBackground = bClearBackground; };

	EVENT_CALLBACK(CMovablePanel, MinimizeWindow);
	EVENT_CALLBACK(CMovablePanel, CloseWindow);

protected:
	int						m_iMouseStartX;
	int						m_iMouseStartY;
	float					m_flStartX;
	float					m_flStartY;
	bool					m_bMoving;

	bool					m_bHasCloseButton;
	bool					m_bMinimized;
	float					m_flNonMinimizedHeight;

	bool					m_bClearBackground;

	glgui::CLabel*			m_pName;

	CCloseButton*			m_pCloseButton;
	CMinimizeButton*		m_pMinimizeButton;
};

class COptionsButton : public glgui::CButton, public glgui::IEventListener
{
public:
	class COptionsPanel : public glgui::CPanel
	{
		DECLARE_CLASS(COptionsPanel, glgui::CPanel);

	public:
						COptionsPanel(COptionsButton* pButton);

	public:
	virtual void		Layout();
	virtual void		Paint(float x, float y, float w, float h);

	protected:
		glgui::CButton*	m_pOkay;
	};

public:
						COptionsButton();

public:
	EVENT_CALLBACK(COptionsButton, Open);
	EVENT_CALLBACK(COptionsButton, Close);

	COptionsPanel*		GetOptionsPanel() { return m_pPanel; }

protected:
	COptionsPanel*		m_pPanel;
};

class CComboGeneratorPanel : public CMovablePanel, public IWorkListener
{
public:
								CComboGeneratorPanel(CConversionScene* pScene, tvector<CMaterial>* paoMaterials);

public:
	virtual void				SetVisible(bool bVisible);

	virtual void				Layout();
	virtual void				UpdateScene();

	virtual void				Think();

	virtual void				Paint(float x, float y, float w, float h);

	virtual bool				KeyPressed(int iKey);

	virtual void				BeginProgress();
	virtual void				SetAction(const tstring& sAction, size_t iTotalProgress);
	virtual void				WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void				EndProgress();

	virtual bool				IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool				DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CComboGeneratorPanel,	Generate);
	EVENT_CALLBACK(CComboGeneratorPanel,	SaveMapDialog);
	EVENT_CALLBACK(CComboGeneratorPanel,	SaveMapFile);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddLoRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddHiRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	RemoveLoRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	RemoveHiRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddLoResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddHiResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	DroppedLoResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	DroppedHiResMesh);

	static void					Open(CConversionScene* pScene, tvector<CMaterial>* paoMaterials);
	static CComboGeneratorPanel*	Get() { return s_pComboGeneratorPanel; }

protected:
	CConversionScene*			m_pScene;
	tvector<CMaterial>*			m_paoMaterials;

	CTexelGenerator				m_oGenerator;

	glgui::CLabel*				m_pSizeLabel;
	glgui::CScrollSelector<int>*	m_pSizeSelector;

	glgui::CLabel*				m_pLoResLabel;
	glgui::CTree*				m_pLoRes;

	glgui::CLabel*				m_pHiResLabel;
	glgui::CTree*				m_pHiRes;

	tvector<CConversionMeshInstance*>	m_apLoResMeshes;
	tvector<CConversionMeshInstance*>	m_apHiResMeshes;

	glgui::CButton*				m_pAddLoRes;
	glgui::CButton*				m_pAddHiRes;

	glgui::CButton*				m_pRemoveLoRes;
	glgui::CButton*				m_pRemoveHiRes;

	glgui::CLabel*				m_pDiffuseLabel;
	glgui::CCheckBox*			m_pDiffuseCheckBox;

	glgui::CLabel*				m_pAOLabel;
	glgui::CCheckBox*			m_pAOCheckBox;

	glgui::CLabel*				m_pNormalLabel;
	glgui::CCheckBox*			m_pNormalCheckBox;

	COptionsButton*				m_pAOOptions;

	glgui::CLabel*				m_pBleedLabel;
	glgui::CScrollSelector<int>*	m_pBleedSelector;

	glgui::CLabel*				m_pSamplesLabel;
	glgui::CScrollSelector<int>*	m_pSamplesSelector;

	glgui::CLabel*				m_pFalloffLabel;
	glgui::CScrollSelector<float>*	m_pFalloffSelector;

	glgui::CLabel*				m_pRandomLabel;
	glgui::CCheckBox*			m_pRandomCheckBox;

	glgui::CLabel*				m_pGroundOcclusionLabel;
	glgui::CCheckBox*			m_pGroundOcclusionCheckBox;

	glgui::CButton*				m_pGenerate;
	glgui::CButton*				m_pSave;

	class CMeshInstancePicker*	m_pMeshInstancePicker;

	static CComboGeneratorPanel*	s_pComboGeneratorPanel;
};

class CAOPanel : public CMovablePanel, public IWorkListener
{
public:
							CAOPanel(bool bColor, CConversionScene* pScene, tvector<CMaterial>* paoMaterials);

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

	static void				Open(bool bColor, CConversionScene* pScene, tvector<CMaterial>* paoMaterials);
	static CAOPanel*		Get(bool bColor);

protected:
	bool					m_bColor;

	CConversionScene*		m_pScene;
	tvector<CMaterial>*		m_paoMaterials;

	CAOGenerator			m_oGenerator;

	glgui::CLabel*			m_pSizeLabel;
	glgui::CScrollSelector<int>*	m_pSizeSelector;

	glgui::CLabel*			m_pEdgeBleedLabel;
	glgui::CScrollSelector<int>*	m_pEdgeBleedSelector;

	glgui::CLabel*			m_pAOMethodLabel;
	glgui::CScrollSelector<int>*	m_pAOMethodSelector;

	glgui::CLabel*			m_pRayDensityLabel;
	glgui::CScrollSelector<int>*	m_pRayDensitySelector;

	glgui::CLabel*			m_pLightsLabel;
	glgui::CScrollSelector<int>*	m_pLightsSelector;

	glgui::CLabel*			m_pFalloffLabel;
	glgui::CScrollSelector<float>*	m_pFalloffSelector;

	glgui::CLabel*			m_pRandomLabel;
	glgui::CCheckBox*		m_pRandomCheckBox;

	glgui::CLabel*			m_pCreaseLabel;
	glgui::CCheckBox*		m_pCreaseCheckBox;

	glgui::CLabel*			m_pGroundOcclusionLabel;
	glgui::CCheckBox*		m_pGroundOcclusionCheckBox;

	glgui::CButton*			m_pGenerate;
	glgui::CButton*			m_pSave;

	static CAOPanel*		s_pAOPanel;
	static CAOPanel*		s_pColorAOPanel;
};

class CNormalPanel : public CMovablePanel
{
public:
								CNormalPanel(CConversionScene* pScene, tvector<CMaterial>* paoMaterials);

public:
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

	static void					Open(CConversionScene* pScene, tvector<CMaterial>* paoMaterials);
	static CNormalPanel*		Get() { return s_pNormalPanel; }

protected:
	CConversionScene*			m_pScene;
	tvector<CMaterial>*			m_paoMaterials;

	CNormalGenerator			m_oGenerator;

	glgui::CLabel*				m_pMaterialsLabel;
	glgui::CTree*				m_pMaterials;

	glgui::CLabel*				m_pProgressLabel;

	glgui::CScrollSelector<float>*	m_pDepthSelector;
	glgui::CLabel*				m_pDepthLabel;

	glgui::CScrollSelector<float>*	m_pHiDepthSelector;
	glgui::CLabel*				m_pHiDepthLabel;

	glgui::CScrollSelector<float>*	m_pMidDepthSelector;
	glgui::CLabel*				m_pMidDepthLabel;

	glgui::CScrollSelector<float>*	m_pLoDepthSelector;
	glgui::CLabel*				m_pLoDepthLabel;

	glgui::CButton*				m_pSave;

	class CMaterialPicker*		m_pMaterialPicker;

	static CNormalPanel*		s_pNormalPanel;
};

class CHelpPanel : public CMovablePanel
{
public:
							CHelpPanel();

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	glgui::CLabel*			m_pInfo;

	static CHelpPanel*		s_pHelpPanel;
};

class CAboutPanel : public CMovablePanel
{
public:
							CAboutPanel();

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	glgui::CLabel*			m_pInfo;

	static CAboutPanel*		s_pAboutPanel;
};

class CRegisterPanel : public CMovablePanel
{
public:
							CRegisterPanel();

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);
	virtual bool			KeyPressed(int iKey);

	static void				Open();
	static void				Close();

	EVENT_CALLBACK(CRegisterPanel, Pirates);
	EVENT_CALLBACK(CRegisterPanel, Website);
	EVENT_CALLBACK(CRegisterPanel, Register);
	EVENT_CALLBACK(CRegisterPanel, RegisterOffline);
	EVENT_CALLBACK(CRegisterPanel, CopyProductCode);
	EVENT_CALLBACK(CRegisterPanel, SetKey);

protected:
	glgui::CButton*			m_pWebsiteButton;

	glgui::CLabel*			m_pInfo;
	glgui::CButton*			m_pPirates;

	glgui::CTextField*		m_pRegistrationKey;
	glgui::CButton*			m_pRegister;
	glgui::CLabel*			m_pRegisterResult;

	glgui::CButton*			m_pRegisterOffline;
	glgui::CLabel*			m_pProductCode;

	static CRegisterPanel*	s_pRegisterPanel;
};

class CPiratesPanel : public CMovablePanel
{
public:
							CPiratesPanel();

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	glgui::CLabel*			m_pInfo;

	static CPiratesPanel*	s_pPiratesPanel;
};

#endif
