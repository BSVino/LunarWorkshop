#include "token.h"

#include <renderer/renderingcontext.h>
#include <physics/physics.h>
#include <tinker/application.h>

#include "grotto_renderer.h"
#include "receptacle.h"

REGISTER_ENTITY(CToken);

NETVAR_TABLE_BEGIN(CToken);
NETVAR_TABLE_END();

void UnserializeString_TokenReceptacle(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);

SAVEDATA_TABLE_BEGIN_EDITOR(CToken);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_COPYTYPE, CEntityHandle<CReceptacle>, m_hReceptacle, "Receptacle", UnserializeString_TokenReceptacle);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, bool, m_bReflected, "Reflected");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_STRING, tstring, m_sType, "TokenType");
	SAVEDATA_EDITOR_VARIABLE("Receptacle");
	SAVEDATA_EDITOR_VARIABLE("Reflected");
	SAVEDATA_EDITOR_VARIABLE("TokenType");
	SAVEDATA_EDITOR_VARIABLE("Model");
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_NETVAR, const char*, m_iModel, "Model", "models/powersource.toy");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CToken);
INPUTS_TABLE_END();

void CToken::Precache()
{
	PrecacheModel("models/powersource.toy");
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
		pContext->Scale(1, -1, 1);
		pContext->SetWinding(!pContext->GetWinding());
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

void UnserializeString_TokenReceptacle(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CBaseEntity* pReceptacleEntity = CBaseEntity::GetEntityByName(sData);

	TAssert(pReceptacleEntity);
	if (!pReceptacleEntity)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") couldn't find entity named '" + sData + "'\n");
		return;
	}

	CReceptacle* pReceptacle = dynamic_cast<CReceptacle*>(pReceptacleEntity);
	TAssert(pReceptacle);
	if (!pReceptacle)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") entity '" + sData + "' is not a receptacle.\n");
		return;
	}

	CToken* pToken = dynamic_cast<CToken*>(pEntity);
	TAssert(pToken);
	if (!pToken)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") is not a token.\n");
		return;
	}

	pReceptacle->SetToken(pToken);
}
