#include "token.h"

#include <renderer/renderingcontext.h>

#include "reflection_renderer.h"

REGISTER_ENTITY(CToken);

NETVAR_TABLE_BEGIN(CToken);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CToken);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bReflected);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CToken);
INPUTS_TABLE_END();

void CToken::Precache()
{
	PrecacheModel("models/r.obj");
}

void CToken::Spawn()
{
	SetModel("models/r.obj");

	m_bReflected = false;
}

CReceptacle* CToken::GetReceptacle() const
{
	return m_hReceptacle;
}

void CToken::ModifyContext(CRenderingContext* pContext, bool bTransparent) const
{
	if (bTransparent)
		return;

	if (m_bReflected)
	{
		pContext->Scale(1, 1, -1);
		pContext->SetReverseWinding(true);
	}
}

REGISTER_ENTITY(CReceptacle);

NETVAR_TABLE_BEGIN(CReceptacle);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReceptacle);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CToken>, m_hToken);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReceptacle);
INPUTS_TABLE_END();

void CReceptacle::SetToken(CToken* pToken)
{
	if (!pToken)
	{
		if (m_hToken != nullptr)
			m_hToken->m_hReceptacle = nullptr;
	}

	m_hToken = pToken;

	if (m_hToken == nullptr)
		return;

	pToken->SetMoveParent(this);
	pToken->SetLocalTransform(TMatrix());
	pToken->m_hReceptacle = this;
}
