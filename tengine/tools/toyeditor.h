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
	void					Save();

public:
	tstring					m_sFilename;
	tstring					m_sToyFile;
};

class CToyEditor : public CWorkbenchTool, public glgui::IEventListener
{
public:
							CToyEditor();
	virtual					~CToyEditor();

public:
	virtual void			Activate();
	virtual void			Deactivate();

	void					SetupMenu();

	void					NewToy();
	CToySource&				GetToy() { return m_oToySource; }

	EVENT_CALLBACK(CToyEditor, NewToy);
	EVENT_CALLBACK(CToyEditor, SaveToy);

	bool					KeyPress(int c);

	virtual tstring			GetToolName() { return "Toy Editor"; }

public:
	static CToyEditor*		Get() { return s_pToyEditor; }

protected:
	CCreateToySourcePanel*	m_pCreateToySourcePanel;

	CToySource				m_oToySource;

private:
	static CToyEditor*		s_pToyEditor;
};

inline CToyEditor* ToyEditor()
{
	return CToyEditor::Get();
}
