#pragma once

#include "tool.h"

#include <glgui/panel.h>

class CToyPreviewPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CToyPreviewPanel, glgui::CPanel);

public:
							CToyPreviewPanel();

public:
	void					Layout();

public:
	glgui::CLabel*			m_pInfo;
};

class CToyViewer : public CWorkbenchTool, public glgui::IEventListener
{
public:
							CToyViewer();
	virtual					~CToyViewer();

public:
	virtual void			Activate();
	virtual void			Deactivate();

	void					Layout();
	void					SetupMenu();

	virtual void			RenderScene();

	EVENT_CALLBACK(CToyViewer, ChooseToy);
	EVENT_CALLBACK(CToyViewer, OpenToy);

	bool					MouseInput(int iButton, int iState);
	void					MouseMotion(int x, int y);

	virtual TVector			GetCameraPosition();
	virtual TVector			GetCameraDirection();

	virtual tstring			GetToolName() { return "Toy Viewer"; }

public:
	static CToyViewer*		Get() { return s_pToyViewer; }

protected:
	CToyPreviewPanel*		m_pToyPreviewPanel;

	size_t					m_iToyPreview;

	bool					m_bRotatingPreview;
	EAngle					m_angPreview;

private:
	static CToyViewer*		s_pToyViewer;
};

inline CToyViewer* ToyViewer()
{
	return CToyViewer::Get();
}
