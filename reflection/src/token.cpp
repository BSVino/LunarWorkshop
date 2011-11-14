#include "token.h"

#include <renderer/renderingcontext.h>

#include "reflection_renderer.h"

REGISTER_ENTITY(CToken);

NETVAR_TABLE_BEGIN(CToken);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CToken);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CReceptacle>, m_hReceptacle);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bReflected);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CToken);
INPUTS_TABLE_END();

void CToken::Precache()
{
	PrecacheModel("models/r.obj");
	PrecacheModel("models/powersource.obj");
}

void CToken::Spawn()
{
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

	if (IsReflected())
	{
		pContext->Scale(1, 1, -1);
		pContext->SetReverseWinding(true);
	}
}

void CToken::Reflected(reflection_t eReflectionType)
{
	m_bReflected = !m_bReflected;
}

bool CToken::IsReflected() const
{
	return m_bReflected;
}

REGISTER_ENTITY(CReceptacle);

NETVAR_TABLE_BEGIN(CReceptacle);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReceptacle);
	SAVEDATA_DEFINE_OUTPUT(OnNormalToken);
	SAVEDATA_DEFINE_OUTPUT(OnNormalTokenRemoved);
	SAVEDATA_DEFINE_OUTPUT(OnReflectedToken);
	SAVEDATA_DEFINE_OUTPUT(OnReflectedTokenRemoved);
	SAVEDATA_DEFINE_OUTPUT(OnTokenRemoved);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CToken>, m_hToken);
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, tstring, m_sDesiredToken);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bDesiredReflection);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReceptacle);
INPUTS_TABLE_END();

void CReceptacle::SetToken(CToken* pToken)
{
	if (m_hToken.GetPointer())
	{
		if (m_hToken->GetName() == m_sDesiredToken)
		{
			if (m_hToken->IsReflected())
				CallOutput("OnReflectedTokenRemoved");
			else
				CallOutput("OnNormalTokenRemoved");
			CallOutput("OnTokenRemoved");
		}
	}

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

	if (pToken->GetName() == m_sDesiredToken)
	{
		if (pToken->IsReflected())
			CallOutput("OnReflectedToken");
		else
			CallOutput("OnNormalToken");
	}
}
