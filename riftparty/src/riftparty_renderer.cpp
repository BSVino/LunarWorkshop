#include "riftparty_renderer.h"

#include <GL3/gl3w.h>

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <models/models.h>
#include <tools/workbench.h>

#include "riftparty_game.h"
#include "riftparty_playercharacter.h"

CRiftPartyRenderer::CRiftPartyRenderer()
	: CGameRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
}

void CRiftPartyRenderer::PreRender()
{
	TPROF("CRiftPartyRenderer::SetupFrame");

	if (CWorkbench::IsActive())
	{
		BaseClass::PreRender();
		return;
	}

	BaseClass::PreRender();
}

void CRiftPartyRenderer::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (CWorkbench::IsActive())
	{
		BaseClass::ModifyContext(pContext);
		return;
	}
}

void CRiftPartyRenderer::SetupFrame(class CRenderingContext* pContext)
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		m_bDrawBackground = true;
	else
		m_bDrawBackground = false;

	BaseClass::SetupFrame(pContext);
}

void CRiftPartyRenderer::FinishRendering(class CRenderingContext* pContext)
{
	BaseClass::FinishRendering(pContext);

	if (CVar::GetCVarValue("game_mode") == "menu")
		return;
}

CRiftPartyRenderer* RiftPartyRenderer()
{
	return static_cast<CRiftPartyRenderer*>(GameServer()->GetRenderer());
}
