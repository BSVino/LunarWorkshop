#pragma once

#include <tstring.h>
#include <tinker_memory.h>

#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <game/cameramanager.h>
#include <game/level.h>

#include "tool.h"
#include "manipulator.h"

class CEntityPropertiesPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CEntityPropertiesPanel, glgui::CPanel);

public:
							CEntityPropertiesPanel(bool bCommon);

public:
	void					Layout();

	EVENT_CALLBACK(CEntityPropertiesPanel, ModelChanged);
	EVENT_CALLBACK(CEntityPropertiesPanel, TargetChanged);
	EVENT_CALLBACK(CEntityPropertiesPanel, PropertyChanged);

	void					SetPropertyChangedListener(glgui::IEventListener* pListener, glgui::IEventListener::Callback pfnCallback);

	void					SetClass(const tstring& sClass) { m_sClass = sClass; }
	void					SetEntity(class CLevelEntity* pEntity);

public:
	tstring								m_sClass;
	float								m_bCommonProperties;
	class CLevelEntity*					m_pEntity;

	tvector<glgui::CLabel*>				m_apPropertyLabels;
	tvector<glgui::CBaseControl*>		m_apPropertyOptions;
	tvector<tstring>					m_asPropertyHandle;

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
	void					LayoutEntity();
	void					LayoutOutput();
	void					LayoutInput();

	CLevelEntity*			GetCurrentEntity();
	CLevelEntity::CLevelEntityOutput* GetCurrentOutput();

	EVENT_CALLBACK(CEditorPanel, EntitySelected);
	EVENT_CALLBACK(CEditorPanel, PropertyChanged);
	EVENT_CALLBACK(CEditorPanel, OutputSelected);
	EVENT_CALLBACK(CEditorPanel, AddOutput);
	EVENT_CALLBACK(CEditorPanel, RemoveOutput);
	EVENT_CALLBACK(CEditorPanel, ChooseOutput);
	EVENT_CALLBACK(CEditorPanel, TargetEntityChanged);
	EVENT_CALLBACK(CEditorPanel, ChooseInput);
	EVENT_CALLBACK(CEditorPanel, ArgumentsChanged);

public:
	glgui::CTree*			m_pEntities;
	glgui::CLabel*			m_pObjectTitle;

	glgui::CSlidingContainer*	m_pSlider;
	glgui::CSlidingPanel*	m_pPropertiesSlider;
	glgui::CSlidingPanel*	m_pOutputsSlider;

	CEntityPropertiesPanel*	m_pPropertiesPanel;

	glgui::CTree*			m_pOutputs;
	glgui::CButton*			m_pAddOutput;
	glgui::CButton*			m_pRemoveOutput;

	glgui::CMenu*			m_pOutput;

	glgui::CLabel*			m_pOutputEntityNameLabel;
	glgui::CTextField*		m_pOutputEntityNameText;

	glgui::CMenu*			m_pInput;

	glgui::CLabel*			m_pOutputArgsLabel;
	glgui::CTextField*		m_pOutputArgsText;
};

class CLevelEditor : public CWorkbenchTool, public glgui::IEventListener, public IManipulatorListener
{
public:
							CLevelEditor();
	virtual					~CLevelEditor();

public:
	void					RenderEntity(size_t i);
	void					RenderEntity(class CLevelEntity* pEntity, bool bSelected=false);
	void					RenderCreateEntityPreview();

	Vector					PositionFromMouse();
	void					EntitySelected();
	void					CreateEntityFromPanel(const Vector& vecPosition);
	static void				PopulateLevelEntityFromPanel(class CLevelEntity* pEntity, CEntityPropertiesPanel* pPanel);
	void					DuplicateSelectedEntity();

	class CLevel*			GetLevel() { return m_pLevel; }

	EVENT_CALLBACK(CLevelEditor, CreateEntity);
	EVENT_CALLBACK(CLevelEditor, SaveLevel);

	bool					KeyPress(int c);
	bool					MouseInput(int iButton, tinker_mouse_state_t iState);

	virtual void			CameraThink();
	virtual TVector			GetCameraPosition();
	virtual TVector			GetCameraDirection();
	virtual void			SetCameraOrientation(TVector vecPosition, Vector vecDirection);

	virtual void			Activate();
	virtual void			Deactivate();

	virtual void			RenderScene();

	virtual void			ManipulatorUpdated(const tstring& sArguments);

	virtual tstring			GetToolName() { return "Level Editor"; }

	static CLevelEditor*	Get() { return s_pLevelEditor; }

protected:
	CHandle<class CLevel>	m_pLevel;

	CEditorPanel*			m_pEditorPanel;

	glgui::CPictureButton*	m_pCreateEntityButton;
	CCreateEntityPanel*		m_pCreateEntityPanel;

	float					m_flCreateObjectDistance;

	TVector					m_vecEditCamera;
	EAngle					m_angEditCamera;

private:
	static CLevelEditor*	s_pLevelEditor;
};

inline CLevelEditor* LevelEditor()
{
	return CLevelEditor::Get();
}
