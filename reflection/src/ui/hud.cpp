#include "hud.h"

#include <tengine/game/gameserver.h>
#include <tengine/renderer/renderer.h>
#include <tengine/renderer/renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>

#include "../reflection_character.h"
#include "../reflection_playercharacter.h"
#include "../reflection_game.h"
#include "../token.h"

CReflectionHUD::CReflectionHUD()
	: glgui::CPanel(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight())
{
}

void CReflectionHUD::Paint(float x, float y, float w, float h)
{
	CPlayerCharacter* pPlayer = static_cast<CPlayerCharacter*>(ReflectionGame()->GetLocalPlayerCharacter());
	if (pPlayer && pPlayer->GetToken())
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.Translate(Vector((float)w, (float)h-100, 0));
		c.Scale(300, 300, 300);
		c.Rotate(-90.0f, Vector(0, 0, 1));
		c.Rotate(-90.0f, Vector(1, 0, 0));

		if (pPlayer->GetToken()->IsReflected() ^ pPlayer->IsReflected())
		{
			c.Scale(1, 1, -1);
			c.SetReverseWinding(true);
		}

		c.RenderModel(pPlayer->GetToken()->GetModel());
	}

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

		if (pEntity == pPlayer)
			continue;

		if (pEntity == pPlayer->GetToken())
			continue;

		TFloat flRadius = pEntity->GetBoundingRadius() + flTokenRadius;
		flRadius = flRadius*flRadius;
		if ((pPlayer->GetGlobalCenter() - pEntity->GetGlobalCenter()).LengthSqr() > flRadius)
			continue;

		CToken* pToken = dynamic_cast<CToken*>(pEntity);
		if (pToken)
		{
			tstring sTip = "E - Pick up";
			float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
			float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
			glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - flFontHeight, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
			glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			break;
		}

		CReceptacle* pReceptacle = dynamic_cast<CReceptacle*>(pEntity);
		if (pReceptacle && pPlayer->GetToken())
		{
			tstring sTip = "E - Place";
			float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
			float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
			glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - flFontHeight, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
			glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			break;
		}
	}
}
