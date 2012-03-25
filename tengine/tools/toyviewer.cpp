#include "toyviewer.h"

#include <glgui/rootpanel.h>
#include <glgui/menu.h>
#include <glgui/filedialog.h>
#include <models/models.h>
#include <tinker/application.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <game/gameserver.h>
#include <tinker/keys.h>
#include <ui/gamewindow.h>

#include "workbench.h"

REGISTER_WORKBENCH_TOOL(ToyViewer);

CToyPreviewPanel::CToyPreviewPanel()
{
	SetBackgroundColor(Color(0, 0, 0, 150));
	SetBorder(glgui::CPanel::BT_SOME);

	m_pInfo = new glgui::CLabel("", "sans-serif", 16);
	AddControl(m_pInfo);
}

void CToyPreviewPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	m_pInfo->SetPos(0, 15);
	m_pInfo->SetSize(GetWidth(), 25);

	BaseClass::Layout();
}

CToyViewer* CToyViewer::s_pToyViewer = nullptr;

CToyViewer::CToyViewer()
{
	s_pToyViewer = this;

	m_pToyPreviewPanel = new CToyPreviewPanel();
	m_pToyPreviewPanel->SetVisible(false);
	glgui::CRootPanel::Get()->AddControl(m_pToyPreviewPanel);

	m_iToyPreview = ~0;

	m_bRotatingPreview = false;
	m_angPreview = EAngle(-20, 20, 0);
}

CToyViewer::~CToyViewer()
{
}

void CToyViewer::Activate()
{
	Layout();
}

void CToyViewer::Deactivate()
{
	m_pToyPreviewPanel->SetVisible(false);
}

void CToyViewer::Layout()
{
	m_pToyPreviewPanel->SetVisible(false);

	if (m_iToyPreview != ~0)
		m_pToyPreviewPanel->SetVisible(true);

	SetupMenu();
}

void CToyViewer::SetupMenu()
{
	GetFileMenu()->ClearSubmenus();

	GetFileMenu()->AddSubmenu("Open", this, ChooseToy);
}

void CToyViewer::RenderScene()
{
	if (m_iToyPreview != ~0)
		TAssert(CModelLibrary::GetModel(m_iToyPreview));

	if (m_iToyPreview != ~0 && CModelLibrary::GetModel(m_iToyPreview))
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.SetColor(Color(255, 255, 255));

		c.RenderModel(m_iToyPreview);
	}
}

void CToyViewer::ChooseToyCallback(const tstring& sArgs)
{
	glgui::CFileDialog::ShowOpenDialog(".", ".toy", this, OpenToy);
}

void CToyViewer::OpenToyCallback(const tstring& sArgs)
{
	CModelLibrary::ReleaseModel(m_iToyPreview);

	m_iToyPreview = CModelLibrary::AddModel(sArgs);
}

bool CToyViewer::MouseInput(int iButton, int iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT)
	{
		m_bRotatingPreview = (iState == 1);
		return true;
	}

	return false;
}

void CToyViewer::MouseMotion(int x, int y)
{
	if (m_bRotatingPreview)
	{
		int lx, ly;
		if (GameWindow()->GetLastMouse(lx, ly))
		{
			m_angPreview.y += (float)(x-lx);
			m_angPreview.p -= (float)(y-ly);
		}
	}
}

TVector CToyViewer::GetCameraPosition()
{
	if (m_iToyPreview == ~0)
		return TVector(0, 0, 0);

	CModel* pMesh = CModelLibrary::GetModel(m_iToyPreview);

	if (!pMesh)
		return TVector(0, 0, 0);

	return pMesh->m_aabbBoundingBox.Center() - AngleVector(m_angPreview)*10;
}

TVector CToyViewer::GetCameraDirection()
{
	return AngleVector(m_angPreview);
}
