#pragma once

#include <glgui/movablepanel.h>

#include "tool.h"

class CCreateToySourcePanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CCreateToySourcePanel, glgui::CMovablePanel);

public:
							CCreateToySourcePanel();

public:
	void					Layout();

	EVENT_CALLBACK(CCreateToySourcePanel, ToyChanged);
	EVENT_CALLBACK(CCreateToySourcePanel, SourceChanged);
	EVENT_CALLBACK(CCreateToySourcePanel, Create);

	tstring					GetToyFileName();
	tstring					GetSourceFileName();

	void					FileNamesChanged();

public:
	glgui::CLabel*			m_pToyFileLabel;
	glgui::CTextField*		m_pToyFileText;

	glgui::CLabel*			m_pSourceFileLabel;
	glgui::CTextField*		m_pSourceFileText;

	glgui::CLabel*			m_pWarnings;

	glgui::CButton*			m_pCreate;
};

class CToySource
{
public:
	void					Save() const;
	void					Build() const;
	void					Open(const tstring& sFile);

public:
	tstring					m_sFilename;
	tstring					m_sToyFile;

	tstring					m_sMesh;
	tstring					m_sPhys;
};

class CSourcePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CSourcePanel, glgui::CPanel);

public:
							CSourcePanel();

public:
	virtual void			SetVisible(bool bVis);

	void					Layout();
	void					UpdateFields();

	EVENT_CALLBACK(CSourcePanel, ModelChanged);
	EVENT_CALLBACK(CSourcePanel, Save);
	EVENT_CALLBACK(CSourcePanel, Build);

public:
	glgui::CLabel*			m_pFilename;

	glgui::CLabel*			m_pToyFileLabel;
	glgui::CTextField*		m_pToyFileText;

	glgui::CLabel*			m_pMeshLabel;
	glgui::CTextField*		m_pMeshText;

	glgui::CLabel*			m_pPhysLabel;
	glgui::CTextField*		m_pPhysText;

	glgui::CButton*			m_pSave;
	glgui::CButton*			m_pBuild;
};

class CToyEditor : public CWorkbenchTool, public glgui::IEventListener
{
public:
							CToyEditor();
	virtual					~CToyEditor();

public:
	virtual void			Activate();
	virtual void			Deactivate();

	void					Layout();
	void					SetupMenu();

	virtual void			RenderScene();

	void					NewToy();
	CToySource&				GetToyToModify();
	const CToySource&		GetToy() { return m_oToySource; }
	void					MarkUnsaved();
	void					MarkSaved();
	bool					IsSaved() { return m_bSaved; }

	EVENT_CALLBACK(CToyEditor, NewToy);
	EVENT_CALLBACK(CToyEditor, SaveToy);
	EVENT_CALLBACK(CToyEditor, ChooseToy);
	EVENT_CALLBACK(CToyEditor, OpenToy);
	EVENT_CALLBACK(CToyEditor, BuildToy);

	bool					KeyPress(int c);
	bool					MouseInput(int iButton, int iState);
	void					MouseMotion(int x, int y);

	virtual TVector			GetCameraPosition();
	virtual TVector			GetCameraDirection();

	virtual tstring			GetToolName() { return "Toy Editor"; }

public:
	static CToyEditor*		Get() { return s_pToyEditor; }

protected:
	CCreateToySourcePanel*	m_pCreateToySourcePanel;

	CSourcePanel*			m_pSourcePanel;

	CToySource				m_oToySource;

	size_t					m_iMeshPreview;
	size_t					m_iPhysPreview;

	bool					m_bRotatingPreview;
	EAngle					m_angPreview;

	bool					m_bSaved;

private:
	static CToyEditor*		s_pToyEditor;
};

inline CToyEditor* ToyEditor()
{
	return CToyEditor::Get();
}
