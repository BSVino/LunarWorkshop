#include "hud.h"

#include <tengine/game/gameserver.h>
#include <tengine/renderer/renderer.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>

#include "../characters/tack_character.h"
#include "../characters/tack_playercharacter.h"
#include "../tack_game.h"
#include "../characters/corpse.h"

CTackHUD::CTackHUD()
	: glgui::CPanel(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight())
{
}

void CTackHUD::Paint(float x, float y, float w, float h)
{
	glgui::CBaseControl::PaintRect(95, 15, 150, 55, Color(50, 50, 50, 150));

	CPlayerCharacter* pTack = dynamic_cast<CPlayerCharacter*>(TackGame()->GetLocalPlayerCharacter());

	tstring sHealth = sprintf("Tack - %d%%", (int)(pTack->GetHealth()*100/pTack->GetTotalHealth()));
	glgui::CLabel::PaintText(sHealth, sHealth.length(), "sans-serif", 18, 100, 20);

	tstring sAbility = tstring("Ability: ") + SpecialAbilityName(pTack->GetSpecialAbility());
	glgui::CLabel::PaintText(sAbility, sAbility.length(), "sans-serif", 18, 100, 45);

	float flCorpseAbsorbDistance = pTack->CorpseAbsorbDistance();
	CCorpse* pNearestCorpse = NULL;
	float flNearestDistanceSqr;
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CCorpse* pCorpse = dynamic_cast<CCorpse*>(pEntity);
		if (!pCorpse)
			continue;

		float flDistanceSqr = (pCorpse->GetGlobalCenter() - pTack->GetGlobalCenter()).LengthSqr();
		if (flDistanceSqr < flCorpseAbsorbDistance*flCorpseAbsorbDistance)
		{
			if (!pNearestCorpse || flDistanceSqr < flNearestDistanceSqr)
			{
				pNearestCorpse = pCorpse;
				flNearestDistanceSqr = flDistanceSqr;
			}
		}
	}

	if (pNearestCorpse)
	{
		tstring sTip = "F - Absorb '" + SpecialAbilityName(pNearestCorpse->GetSpecialAbility()) + "'";
		float flTextWidth = glgui::CLabel::GetTextWidth(sTip, sTip.length(), "sans-serif", 18);
		float flFontHeight = glgui::CLabel::GetFontHeight("sans-serif", 18);
		glgui::CBaseControl::PaintRect(w/2+200 - 5, h/2 - flFontHeight - 5, flTextWidth + 10, flFontHeight + 10, Color(50, 50, 50, 150));
		glgui::CLabel::PaintText(sTip, sTip.length(), "sans-serif", 18, (float)w/2+200, (float)h/2-flFontHeight);
	}
}
