#include "hud.h"

#include <tengine/game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <textures/texturelibrary.h>
#include <tinker/keys.h>

#include "../chain_game.h"
#include "../story.h"
#include "menu.h"

CChainHUD::CChainHUD()
	: m_pMenu(new CChainMenu())
{
	AddControl(m_pMenu.get());
}

void CChainHUD::Paint(float x, float y, float w, float h)
{
	BaseClass::Paint(x, y, w, h);

	if (!ChainGame())
		return;

	CStory* pStory = ChainGame()->GetStory();

	if (!pStory)
		return;

	CPage* pPage = pStory->GetCurrentPage();

	if (!pPage)
		return;

	if (pPage->m_sNextPage.length())
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CBaseControl::PaintTexture(CTextureLibrary::FindTextureID("textures/arrow.png"), w-100, h-100, 50, 25, Color(255, 255, 255, (unsigned char)(pStory->GetAlpha()*255)));
	}

	if (pPage->m_sPrevPage.length())
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CBaseControl::PaintTexture(CTextureLibrary::FindTextureID("textures/arrow.png"), 100, h-100, -50, 25, Color(255, 255, 255, (unsigned char)(pStory->GetAlpha()*255)));
	}
}

bool CChainHUD::KeyPressed(int code, bool bCtrlDown)
{
	if (code == TINKER_KEY_ESCAPE)
	{
		m_pMenu->SetVisible(!m_pMenu->IsVisible());
		return false;
	}

	return BaseClass::KeyPressed(code, bCtrlDown);
}

bool CChainHUD::MousePressed(int code, int mx, int my)
{
	CStory* pStory = nullptr;

	if (ChainGame() && ChainGame()->GetStory())
		pStory = ChainGame()->GetStory();

	CPage* pPage = nullptr;
	if (pStory)
		pPage = pStory->GetCurrentPage();

	if (pPage && pPage->m_sNextPage.length() && mx > GetWidth()-100 && mx < GetWidth()-50 && my > GetHeight()-100 && my < GetHeight()-75)
	{
		pStory->GoToNextPage();
		return false;
	}

	if (pPage && pPage->m_sPrevPage.length() && mx > 50 && mx < 100 && my > GetHeight()-100 && my < GetHeight()-75)
	{
		pStory->GoToPrevPage();
		return false;
	}

	return BaseClass::MousePressed(code, mx, my);
}
