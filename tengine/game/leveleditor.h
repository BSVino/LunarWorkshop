#pragma once

#include <tstring.h>
#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <game/camera.h>

class CEntityPropertiesPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CEntityPropertiesPanel, glgui::CPanel);

public:
							CEntityPropertiesPanel(bool bCommon);

public:
	void					Layout();

	EVENT_CALLBACK(CEntityPropertiesPanel, PropertyChanged);

	void					SetPropertyChangedListener(glgui::IEventListener* pListener, glgui::IEventListener::Callback pfnCallback);

	void					SetClass(const tstring& sClass) { m_sClass = sClass; }
	void					SetMaxHeight(float flMaxHeight) { m_flMaxHeight = flMaxHeight; }
	void					SetEntity(class CLevelEntity* pEntity) { m_pEntity = pEntity; }

public:
	tstring								m_sClass;
	float								m_flMaxHeight;
	float								m_bCommonProperties;
	class CLevelEntity*					m_pEntity;

	eastl::vector<glgui::CLabel*>		m_apPropertyLabels;
	eastl::vector<glgui::CBaseControl*>	m_apPropertyOptions;
	eastl::vector<tstring>				m_asPropertyHandle;

	glgui::IEventListener::Callback		m_pfnPropertyChangedCallback;
	glgui::IEventListener*				m_pPropertyChangedListener;
};

class CCreateEntityPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CCreateEntityPanel, glgui::CMovablePanel);

public:
							CCreateEntityPanel();

public:
	void					Layout();

	EVENT_CALLBACK(CCreateEntityPanel, ChooseClass);
	EVENT_CALLBACK(CCreateEntityPanel, ModelChanged);

public:
	bool					m_bReadyToCreate;

	glgui::CMenu*			m_pClass;

	glgui::CLabel*			m_pNameLabel;
	glgui::CTextField*		m_pNameText;

	glgui::CLabel*			m_pModelLabel;
	glgui::CTextField*		m_pModelText;

	CEntityPropertiesPanel*	m_pPropertiesPanel;
};

class CEditorPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CEditorPanel, glgui::CPanel);

public:
							CEditorPanel();

public:
	void					Layout();
	void					LayoutEntities();

	EVENT_CALLBACK(CEditorPanel, EntitySelected);
	EVENT_CALLBACK(CEditorPanel, PropertyChanged);

public:
	glgui::CTree*			m_pEntities;
	glgui::CLabel*			m_pObjectTitle;

	CEntityPropertiesPanel*	m_pPropertiesPanel;
};

class CEditorCamera : public CCamera
{
	DECLARE_CLASS(CEditorCamera, CCamera);

public:
	virtual void	Think();

	virtual TVector	GetCameraPosition();
	virtual TVector	GetCameraDirection();

	virtual void	SetCameraOrientation(TVector vecPosition, Vector vecDirection);

public:
	TVector			m_vecEditCamera;
	EAngle			m_angEditCamera;
};

class CLevelEditor : public glgui::IEventListener
{
public:
							CLevelEditor();
	virtual					~CLevelEditor();

public:
	void					RenderEntity(size_t i, bool bTransparent);
	void					RenderEntity(class CLevelEntity* pEntity, bool bTransparent, bool bSelected=false);
	void					RenderCreateEntityPreview();

	Vector					PositionFromMouse();
	void					EntitySelected();
	void					CreateEntityFromPanel(const Vector& vecPosition);
	static void				PopulateLevelEntityFromPanel(class CLevelEntity* pEntity, CEntityPropertiesPanel* pPanel);

	class CLevel*			GetLevel() { return m_pLevel; }

	EVENT_CALLBACK(CLevelEditor, CreateEntity);

	bool					KeyPress(int c);
	bool					MouseInput(int iButton, int iState);

public:
	static void				Toggle();
	static bool				IsActive();

	static void				Activate();
	static void				Deactivate();

	static void				RenderEntities();
	static void				Render();

	static CCamera*			GetCamera();

protected:
	bool					m_bActive;
	bool					m_bWasMouseActive;

	class CLevel*			m_pLevel;

	CEditorPanel*			m_pEditorPanel;

	glgui::CPictureButton*	m_pCreateEntityButton;
	CCreateEntityPanel*		m_pCreateEntityPanel;

	CEditorCamera*			m_pCamera;

	float					m_flCreateObjectDistance;
};

CLevelEditor* LevelEditor(bool bCreate=true);
