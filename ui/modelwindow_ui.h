#ifndef SMAK_MODELWINDOW_UI_H
#define SMAK_MODELWINDOW_UI_H

#include <glgui/glgui.h>
#include "crunch/crunch.h"

using namespace glgui;

typedef enum
{
	BA_TOP,
	BA_BOTTOM,
} buttonalignment_t;

class CButtonPanel : public CPanel
{
public:
							CButtonPanel(buttonalignment_t eAlign);

	virtual void			Layout();

	virtual void			AddButton(CButton* pButton, const eastl::string16& sHints, bool bNewSection, IEventListener* pListener = NULL, IEventListener::Callback pfnCallback = NULL);

	virtual void			Think();
	virtual void			Paint(int x, int y, int w, int h);

protected:
	buttonalignment_t		m_eAlign;

	eastl::vector<int>		m_aiSpaces;
	eastl::vector<CButton*>	m_apButtons;
	eastl::vector<CLabel*>	m_apHints;
};

class CProgressBar : public CPanel
{
public:
							CProgressBar();

public:
	void					Layout();
	void					Paint(int x, int y, int w, int h);

	void					SetTotalProgress(size_t iProgress);
	void					SetProgress(size_t iProgress, const eastl::string16& sAction = L"");
	void					SetAction(const eastl::string16& sAction);

	static CProgressBar*	Get();

protected:
	size_t					m_iTotalProgress;
	size_t					m_iCurrentProgress;

	CLabel*					m_pAction;
	eastl::string16			m_sAction;

	static CProgressBar*	s_pProgressBar;
};

#define HEADER_HEIGHT 16

class CCloseButton : public CButton
{
public:
							CCloseButton() : CButton(0, 0, 10, 10, L"") {};

public:
	virtual void			Paint() { CButton::Paint(); };
	virtual void			Paint(int x, int y, int w, int h);
};

class CMinimizeButton : public CButton
{
public:
							CMinimizeButton() : CButton(0, 0, 10, 10, L"") {};

public:
	virtual void			Paint() { CButton::Paint(); };
	virtual void			Paint(int x, int y, int w, int h);
};

class CMovablePanel : public CPanel, public IEventListener
{
public:
							CMovablePanel(const eastl::string16& sName);
							~CMovablePanel();

	virtual void			Layout();

	virtual void			Think();

	virtual void			Paint(int x, int y, int w, int h);

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
	int						m_iStartX;
	int						m_iStartY;
	bool					m_bMoving;

	bool					m_bHasCloseButton;
	bool					m_bMinimized;
	int						m_iNonMinimizedHeight;

	bool					m_bClearBackground;

	CLabel*					m_pName;

	CCloseButton*			m_pCloseButton;
	CMinimizeButton*		m_pMinimizeButton;
};

class COptionsButton : public CButton, public IEventListener
{
public:
	class COptionsPanel : public CPanel
	{
		DECLARE_CLASS(COptionsPanel, CPanel);

	public:
						COptionsPanel(COptionsButton* pButton);

	public:
	virtual void		Layout();
	virtual void		Paint(int x, int y, int w, int h);

	protected:
		CButton*		m_pOkay;
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
								CComboGeneratorPanel(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials);

public:
	virtual void				SetVisible(bool bVisible);

	virtual void				Layout();
	virtual void				UpdateScene();

	virtual void				Think();

	virtual void				Paint(int x, int y, int w, int h);

	virtual bool				KeyPressed(int iKey);

	virtual void				BeginProgress();
	virtual void				SetAction(const wchar_t* pszAction, size_t iTotalProgress);
	virtual void				WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void				EndProgress();

	virtual bool				IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool				DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CComboGeneratorPanel,	Generate);
	EVENT_CALLBACK(CComboGeneratorPanel,	SaveMap);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddLoRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddHiRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	RemoveLoRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	RemoveHiRes);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddLoResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	AddHiResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	DroppedLoResMesh);
	EVENT_CALLBACK(CComboGeneratorPanel,	DroppedHiResMesh);

	static void					Open(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials);
	static CComboGeneratorPanel*	Get() { return s_pComboGeneratorPanel; }

protected:
	CConversionScene*			m_pScene;
	eastl::vector<CMaterial>*		m_paoMaterials;

	CTexelGenerator				m_oGenerator;

	CLabel*						m_pSizeLabel;
	CScrollSelector<int>*		m_pSizeSelector;

	CLabel*						m_pLoResLabel;
	CTree*						m_pLoRes;

	CLabel*						m_pHiResLabel;
	CTree*						m_pHiRes;

	eastl::vector<CConversionMeshInstance*>	m_apLoResMeshes;
	eastl::vector<CConversionMeshInstance*>	m_apHiResMeshes;

	CButton*					m_pAddLoRes;
	CButton*					m_pAddHiRes;

	CButton*					m_pRemoveLoRes;
	CButton*					m_pRemoveHiRes;

	CLabel*						m_pDiffuseLabel;
	CCheckBox*					m_pDiffuseCheckBox;

	CLabel*						m_pAOLabel;
	CCheckBox*					m_pAOCheckBox;

	CLabel*						m_pNormalLabel;
	CCheckBox*					m_pNormalCheckBox;

	COptionsButton*				m_pAOOptions;

	CLabel*						m_pBleedLabel;
	CScrollSelector<int>*		m_pBleedSelector;

	CLabel*						m_pSamplesLabel;
	CScrollSelector<int>*		m_pSamplesSelector;

	CLabel*						m_pFalloffLabel;
	CScrollSelector<float>*		m_pFalloffSelector;

	CLabel*						m_pRandomLabel;
	CCheckBox*					m_pRandomCheckBox;

	CLabel*						m_pGroundOcclusionLabel;
	CCheckBox*					m_pGroundOcclusionCheckBox;

	CButton*					m_pGenerate;
	CButton*					m_pSave;

	class CMeshInstancePicker*	m_pMeshInstancePicker;

	static CComboGeneratorPanel*	s_pComboGeneratorPanel;
};

class CAOPanel : public CMovablePanel, public IWorkListener
{
public:
							CAOPanel(bool bColor, CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials);

	virtual void			SetVisible(bool bVisible);

	virtual void			Layout();

	virtual bool			KeyPressed(int iKey);

	virtual void			BeginProgress();
	virtual void			SetAction(const wchar_t* pszAction, size_t iTotalProgress);
	virtual void			WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void			EndProgress();

	virtual bool			IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool			DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CAOPanel, Generate);
	EVENT_CALLBACK(CAOPanel, SaveMap);
	EVENT_CALLBACK(CAOPanel, AOMethod);

	virtual void			FindBestRayFalloff();

	static void				Open(bool bColor, CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials);
	static CAOPanel*		Get(bool bColor);

protected:
	bool					m_bColor;

	CConversionScene*		m_pScene;
	eastl::vector<CMaterial>*	m_paoMaterials;

	CAOGenerator			m_oGenerator;

	CLabel*					m_pSizeLabel;
	CScrollSelector<int>*	m_pSizeSelector;

	CLabel*					m_pEdgeBleedLabel;
	CScrollSelector<int>*	m_pEdgeBleedSelector;

	CLabel*					m_pAOMethodLabel;
	CScrollSelector<int>*	m_pAOMethodSelector;

	CLabel*					m_pRayDensityLabel;
	CScrollSelector<int>*	m_pRayDensitySelector;

	CLabel*					m_pLightsLabel;
	CScrollSelector<int>*	m_pLightsSelector;

	CLabel*					m_pFalloffLabel;
	CScrollSelector<float>*	m_pFalloffSelector;

	CLabel*					m_pRandomLabel;
	CCheckBox*				m_pRandomCheckBox;

	CLabel*					m_pCreaseLabel;
	CCheckBox*				m_pCreaseCheckBox;

	CLabel*					m_pGroundOcclusionLabel;
	CCheckBox*				m_pGroundOcclusionCheckBox;

	CButton*				m_pGenerate;
	CButton*				m_pSave;

	static CAOPanel*		s_pAOPanel;
	static CAOPanel*		s_pColorAOPanel;
};

class CNormalPanel : public CMovablePanel
{
public:
								CNormalPanel(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials);

public:
	virtual void				SetVisible(bool bVisible);

	virtual void				Layout();
	virtual void				UpdateScene();

	virtual void				Think();

	virtual void				Paint(int x, int y, int w, int h);

	virtual bool				KeyPressed(int iKey);

	virtual bool				IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool				DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CNormalPanel,	Generate);
	EVENT_CALLBACK(CNormalPanel,	SaveMap);
	EVENT_CALLBACK(CNormalPanel,	SetupNormal2);
	EVENT_CALLBACK(CNormalPanel,	UpdateNormal2);

	static void					Open(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials);
	static CNormalPanel*		Get() { return s_pNormalPanel; }

protected:
	CConversionScene*			m_pScene;
	eastl::vector<CMaterial>*	m_paoMaterials;

	CNormalGenerator			m_oGenerator;

	CLabel*						m_pMaterialsLabel;
	CTree*						m_pMaterials;

	CLabel*						m_pProgressLabel;

	CScrollSelector<float>*		m_pDepthSelector;
	CLabel*						m_pDepthLabel;

	CScrollSelector<float>*		m_pHiDepthSelector;
	CLabel*						m_pHiDepthLabel;

	CScrollSelector<float>*		m_pMidDepthSelector;
	CLabel*						m_pMidDepthLabel;

	CScrollSelector<float>*		m_pLoDepthSelector;
	CLabel*						m_pLoDepthLabel;

	CButton*					m_pSave;

	class CMaterialPicker*		m_pMaterialPicker;

	static CNormalPanel*		s_pNormalPanel;
};

class CHelpPanel : public CMovablePanel
{
public:
							CHelpPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	CLabel*					m_pInfo;

	static CHelpPanel*		s_pHelpPanel;
};

class CAboutPanel : public CMovablePanel
{
public:
							CAboutPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	CLabel*					m_pInfo;

	static CAboutPanel*		s_pAboutPanel;
};

class CRegisterPanel : public CMovablePanel
{
public:
							CRegisterPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

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
	CButton*				m_pWebsiteButton;

	CLabel*					m_pInfo;
	CButton*				m_pPirates;

	CTextField*				m_pRegistrationKey;
	CButton*				m_pRegister;
	CLabel*					m_pRegisterResult;

	CButton*				m_pRegisterOffline;
	CLabel*					m_pProductCode;

	static CRegisterPanel*	s_pRegisterPanel;
};

class CPiratesPanel : public CMovablePanel
{
public:
							CPiratesPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	CLabel*					m_pInfo;

	static CPiratesPanel*	s_pPiratesPanel;
};

#endif
