#include "hud.h"

#include <tinker_platform.h>

#include <tengine/game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <tinker/cvar.h>
#include <tinker/keys.h>

#include "../grotto_character.h"
#include "../grotto_playercharacter.h"
#include "../grotto_game.h"
#include "../grotto_player.h"
#include "../token.h"
#include "../receptacle.h"
#include "../momento.h"
#include "levelselector.h"

CGrottoHUD::CGrottoHUD()
{
	m_pSelector = new CLevelSelector();
	AddControl(m_pSelector);

	m_pSelector->SetVisible(CVar::GetCVarValue("game_mode") == "menu");
}

void CGrottoHUD::Paint(float x, float y, float w, float h)
{
	BaseClass::Paint(x, y, w, h);

	if (CVar::GetCVarValue("game_mode") == "menu")
		return;

	CPlayerCharacter* pPlayerCharacter = static_cast<CPlayerCharacter*>(GrottoGame()->GetLocalPlayerCharacter());
	CGrottoPlayer* pPlayer = static_cast<CGrottoPlayer*>(GrottoGame()->GetLocalPlayer());

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
		CGameRenderingContext c(GameServer()->GetRenderer());

		c.ClearDepth();

		float flRatio = w/h;

		c.SetProjection(Matrix4x4::ProjectOrthographic(-flRatio, flRatio, -1, 1, -100, 100));

		c.SetView(Matrix4x4());

		c.Translate(Vector(flRatio*0.7f, -0.7f, 0));

		float flScale = (1/pPlayerCharacter->GetToken()->GetBoundingBox().Size().Length())/2;
		c.Scale(flScale, flScale, flScale);

		c.Rotate(90.0f, Vector(0, 0, 1));
		c.Rotate(90.0f, Vector(0, 1, 0));
		c.Rotate(90.0f, Vector(1, 0, 0));

		CToken* pToken = pPlayerCharacter->GetToken();
		if (pToken->IsReflected() ^ (pPlayerCharacter->IsReflected(REFLECTION_LATERAL) ^ pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)))
		{
			c.Scale(1, 1, -1);
			c.SetWinding(false);
		}

		c.RenderMaterialModel(pPlayerCharacter->GetToken()->GetMaterialModel(), pPlayerCharacter->GetToken());
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

		TFloat flRadius = flTokenRadius*flTokenRadius;

		CMomento* pMomento = dynamic_cast<CMomento*>(pEntity);
		if (pMomento)
		{
			if (!GameServer()->GetRenderer()->IsSphereInFrustum(pMomento->GetGlobalOrigin(), pMomento->GetBoundingRadius()))
				continue;

			if (pMomento->GetGlobalOrigin().DistanceSqr(pPlayerCharacter->GetGlobalCenter()) > flRadius)
				continue;

			Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pMomento->GetGlobalOrigin()) + Vector(30, 0, 0);

			if (vecScreen.x < 100)
				continue;
			if (vecScreen.y < 100)
				continue;
			if (vecScreen.x > GetWidth()-100)
				continue;
			if (vecScreen.y > GetHeight()-100)
				continue;

			tstring sTip = pMomento->GetMomentoName();
			float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
			float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
			glgui::CBaseControl::PaintRect(vecScreen.x - 5, vecScreen.y - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150), 1);
			glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, vecScreen.x, vecScreen.y);

			continue;
		}

		CToken* pToken = dynamic_cast<CToken*>(pEntity);
		if (pToken)
		{
			if ((pPlayerCharacter->GetGlobalCenter() - pEntity->GetGlobalCenter()).LengthSqr() > flRadius)
				continue;

			if (pToken->GetReceptacle() && !pToken->GetReceptacle()->IsActive())
				continue;

			if (pPlayerCharacter->GetToken())
			{
				tstring sTip = sprintf("%c - Swap", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150), 2);
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			else
			{
				tstring sTip = sprintf("%c - Pick up", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150), 2);
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			break;
		}

		CReceptacle* pReceptacle = dynamic_cast<CReceptacle*>(pEntity);
		if (pReceptacle && pPlayerCharacter->GetToken() && pReceptacle->IsTokenValid(pPlayerCharacter->GetToken()))
		{
			if (!pReceptacle->IsActive())
				continue;

			if ((pPlayerCharacter->GetGlobalCenter() - pReceptacle->GetTokenPosition()).LengthSqr() > flRadius)
				continue;

			if (pReceptacle->GetToken())
			{
				tstring sTip = sprintf("%c - Swap", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150), 2);
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			else
			{
				tstring sTip = sprintf("%c - Place", iKey);
				float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
				float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
				glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150), 2);
				glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, w/2+200, h/2);
			}
			break;
		}
	}
}

bool CGrottoHUD::KeyPressed(int code, bool bCtrlDown)
{
	if (code == TINKER_KEY_ESCAPE)
	{
		m_pSelector->SetVisible(!m_pSelector->IsVisible());
		return false;
	}

	return BaseClass::KeyPressed(code, bCtrlDown);
}
