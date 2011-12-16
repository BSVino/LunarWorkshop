#include "hud.h"

#include <tengine/game/gameserver.h>
#include <tengine/renderer/renderer.h>
#include <tengine/renderer/renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <tengine/models/texturelibrary.h>

#include "../chain_game.h"
#include "../story.h"

void CChainHUD::Paint(float x, float y, float w, float h)
{
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
