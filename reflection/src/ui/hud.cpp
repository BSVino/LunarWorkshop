#include "hud.h"

#include <tinker_platform.h>

#include <tengine/game/gameserver.h>
#include <tengine/renderer/renderer.h>
#include <tengine/renderer/renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <tinker/cvar.h>
#include <tinker/keys.h>

#include "../reflection_character.h"
#include "../reflection_playercharacter.h"
#include "../reflection_game.h"
#include "../reflection_player.h"
#include "../token.h"
#include "../receptacle.h"
#include "levelselector.h"

CReflectionHUD::CReflectionHUD()
{
	m_pSelector = new CLevelSelector();
	AddControl(m_pSelector);

	m_pSelector->SetVisible(CVar::GetCVarValue("game_mode") == "menu");
}

void CReflectionHUD::Paint(float x, float y, float w, float h)
{
	BaseClass::Paint(x, y, w, h);

	if (CVar::GetCVarValue("game_mode") == "menu")
		return;

	CPlayerCharacter* pPlayerCharacter = static_cast<CPlayerCharacter*>(ReflectionGame()->GetLocalPlayerCharacter());
	CReflectionPlayer* pPlayer = static_cast<CReflectionPlayer*>(ReflectionGame()->GetLocalPlayer());

/*	if (pPlayer)
	{
		tstring sTip = "1 - Vertical Mirror";
		float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
		float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
		if (pPlayer->GetCurrentMirror() == MIRROR_VERTICAL)
			glgui::CBaseControl::PaintRect(50 - 5, h/4 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
		glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, 50, h/4);

		sTip = "2 - Horizontal Mirror";
		flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
		flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
		if (pPlayer->GetCurrentMirror() == MIRROR_HORIZONTAL)
			glgui::CBaseControl::PaintRect(50 - 5, h/4 + 50 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
		glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, 50, h/4 + 50);
	}*/

	if (pPlayerCharacter && pPlayerCharacter->GetToken())
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.Translate(Vector((float)w-100, (float)h-100, 0));
		c.Scale(300, 300, 300);
		c.Rotate(-90.0f, Vector(0, 0, 1));
		c.Rotate(-90.0f, Vector(1, 0, 0));

		CToken* pToken = pPlayerCharacter->GetToken();
		if (pToken->IsReflected() ^ (pPlayerCharacter->IsReflected(REFLECTION_LATERAL) ^ pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)))
		{
			c.Scale(1, 1, -1);
			c.SetReverseWinding(true);
		}

		c.RenderModel(pPlayerCharacter->GetToken()->GetModelID());
	}

	int iKey = TranslateKeyFromQwerty('E');

	float flTokenRadius = 1.5f;
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (!pEntity->IsVisible())
			continue;

		if (pEntity == pPlayerCharacter)
			continue;

		if (pEntity == pPlayerCharacter->GetToken())
			continue;

		TFloat flRadius = pEntity->GetBoundingRadius() + flTokenRadius;
		flRadius = flRadius*flRadius;
		if ((pPlayerCharacter->GetGlobalCenter() - pEntity->GetGlobalCenter()).LengthSqr() > flRadius)
			continue;

		CToken* pToken = dynamic_cast<CToken*>(pEntity);
		if (pToken)
		{
			if (pPlayerCharacter->GetToken())
			{
				tstring sTip = sprintf("%c - Swap", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			else
			{
				tstring sTip = sprintf("%c - Pick up", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			break;
		}

		CReceptacle* pReceptacle = dynamic_cast<CReceptacle*>(pEntity);
		if (pReceptacle && pPlayerCharacter->GetToken() && pReceptacle->IsTokenValid(pPlayerCharacter->GetToken()))
		{
			if (pReceptacle->GetToken())
			{
				tstring sTip = sprintf("%c - Swap", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			else
			{
				tstring sTip = sprintf("%c - Place", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			break;
		}
	}
}

bool CReflectionHUD::KeyPressed(int code, bool bCtrlDown)
{
	if (code == TINKER_KEY_ESCAPE)
	{
		m_pSelector->SetVisible(!m_pSelector->IsVisible());
		return false;
	}

	return BaseClass::KeyPressed(code, bCtrlDown);
}
