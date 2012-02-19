#pragma once

#include <tstring.h>
#include <glgui/panel.h>
#include <game/camera.h>

class CEditorPanel : public glgui::CPanel
{
	DECLARE_CLASS(CEditorPanel, glgui::CPanel);

public:
							CEditorPanel();

public:
	void					Layout();

public:
	glgui::CTree*			m_pEntities;
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

class CLevelEditor
{
public:
							CLevelEditor();
							~CLevelEditor();

public:
	void					RenderEntity(size_t i, bool bTransparent);

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

	CEditorCamera*			m_pCamera;
};

CLevelEditor* LevelEditor();
