#pragma once

#include <tstring.h>
#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <game/camera.h>

class CCreateEntityPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CCreateEntityPanel, glgui::CMovablePanel);

public:
					CCreateEntityPanel();

public:
	void					Layout();

	EVENT_CALLBACK(CCreateEntityPanel, ChooseClass);

public:
	bool					m_bReadyToCreate;

	glgui::CMenu*			m_pClass;

	glgui::CLabel*			m_pNameLabel;
	glgui::CTextField*		m_pNameText;

	glgui::CLabel*			m_pModelLabel;
	glgui::CTextField*		m_pModelText;
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

public:
	glgui::CTree*			m_pEntities;
	glgui::CLabel*			m_pObjectTitle;
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
