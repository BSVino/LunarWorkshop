#include "mirror.h"

#include <models/models.h>
#include <physics/physics.h>
#include <renderer/renderingcontext.h>
#include <renderer/shaders.h>

#include "grotto_renderer.h"
#include "grotto_game.h"
#include "grotto_character.h"
#include "grotto_playercharacter.h"

REGISTER_ENTITY(CMirror);

NETVAR_TABLE_BEGIN(CMirror);
NETVAR_TABLE_END();

void UnserializeString_MirrorType(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);

SAVEDATA_TABLE_BEGIN(CMirror);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_COPYTYPE, mirror_t, m_eMirrorType, "MirrorType", UnserializeString_MirrorType);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iBuffer);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hDragTo, "DragTo");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecOriginalPosition);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMirror);
INPUTS_TABLE_END();

tvector<CEntityHandle<CMirror> > CMirror::m_ahMirrors;

CMirror::CMirror()
{
	m_eMirrorType = MIRROR_NONE;
}

CMirror::~CMirror()
{
	for (size_t i = 0; i < m_ahMirrors.size(); i++)
	{
		if (m_ahMirrors[i] == (const CMirror*)this)
		{
			m_ahMirrors.erase(m_ahMirrors.begin()+i);
			break;
		}
	}
}

void CMirror::Precache()
{
	PrecacheModel("models/mirror.toy");
	PrecacheModel("models/mirror_horizontal.toy");
}

void CMirror::Spawn()
{
	if (!m_eMirrorType)
		SetMirrorType(MIRROR_VERTICAL);

	if (IsInPhysics())
		RemoveFromPhysics();
	AddToPhysics(CT_KINEMATIC);

	m_ahMirrors.push_back(this);
}

void CMirror::PostLoad()
{
	m_vecOriginalPosition = GetGlobalOrigin();

	BaseClass::PostLoad();

	if (!!m_hDragTo)
		SetUsable(true);
}

bool CMirror::ModifyShader(class CRenderingContext* pContext) const
{
	if (!pContext->GetActiveShader())
		return true;

	if (CanDrag() && pContext->GetActiveShader()->m_sName == "model")
		pContext->SetColor(Color(0, 255, 0));

	if (pContext->GetActiveShader()->m_sName != "reflection")
		return true;

	if (GrottoRenderer()->IsRenderingReflection())
		return false;

	size_t iBuffer = GetBuffer();
	if (iBuffer == ~0)
		return false;

	CFrameBuffer &oBuffer = GrottoRenderer()->GetReflectionBuffer(iBuffer);

	pContext->SetUniform("flScreenWidth", (float)oBuffer.m_iWidth);
	pContext->SetUniform("flScreenHeight", (float)oBuffer.m_iHeight);

	pContext->SetUniform("flTime", (float)GameServer()->GetGameTime());

	pContext->SetUniform("bDiffuse", true);
	pContext->SetUniform("iDiffuse", 0);

	pContext->BindTexture(oBuffer.m_iMap);

	return true;
}

bool CMirror::IsPointInside(const Vector& vecPoint, bool bPhysics) const
{
	AABB aabbBox = bPhysics?GetPhysBoundingBox():GetVisBoundingBox();

	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		if (GetGlobalTransform().GetUpVector().z > 0)
		{
			if (vecPoint.z < GetGlobalOrigin().z + 0.05f)
				return false;

			if (vecPoint.z > GetGlobalOrigin().z + aabbBox.Size().z)
				return false;
		}
		else
		{
			if (vecPoint.z > GetGlobalOrigin().z - 0.05f)
				return false;

			if (vecPoint.z < GetGlobalOrigin().z - aabbBox.Size().z)
				return false;
		}

		return (vecPoint - GetGlobalOrigin()).Length2D() < aabbBox.Size().Length2D()/2;

	case MIRROR_HORIZONTAL:
		if (bPhysics)
		{
			Vector vecUp = Matrix4x4(GetGlobalAngles(), Vector()).GetUpVector();

			if (vecUp.z > 0)
			{
				if (vecPoint.z < GetGlobalOrigin().z - vecUp.z * 2.0f)
					return false;

				if (vecPoint.z > GetGlobalOrigin().z + vecUp.z * 0.15f)
					return false;
			}
			else
			{
				if (vecPoint.z > GetGlobalOrigin().z - vecUp.z * 2.0f)
					return false;

				if (vecPoint.z < GetGlobalOrigin().z + vecUp.z * 0.15f)
					return false;
			}
		}

		CGrottoCharacter* pPlayerCharacter = GrottoGame()->GetLocalPlayerCharacter();
		AABB aabbPlayerBox = bPhysics?pPlayerCharacter->GetPhysBoundingBox():pPlayerCharacter->GetVisBoundingBox();

		aabbBox.m_vecMins += aabbPlayerBox.m_vecMins;
		aabbBox.m_vecMaxs += aabbPlayerBox.m_vecMaxs;

		// Use a tighter area for physics to make sure we never fall outside the level
		if (bPhysics)
			return (aabbBox*0.8f).Inside2D(vecPoint - GetGlobalOrigin());
		else
			return (aabbBox*1.2f).Inside2D(vecPoint - GetGlobalOrigin());
	}

	return false;
}

void CMirror::SetMirrorType(mirror_t eType)
{
	Precache();

	m_eMirrorType = eType;
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		SetModel("models/mirror.toy");
		break;

	case MIRROR_HORIZONTAL:
		SetModel("models/mirror_horizontal.toy");
		break;
	}

	if (IsInPhysics())
		RemoveFromPhysics();
	AddToPhysics(CT_KINEMATIC);
}

reflection_t CMirror::GetReflectionType() const
{
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		return REFLECTION_LATERAL;

	case MIRROR_HORIZONTAL:
		return REFLECTION_VERTICAL;
	}

	return REFLECTION_NONE;
}

bool CMirror::GetSide(const Vector& vecPoint) const
{
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		return (vecPoint - GetGlobalOrigin()).Normalized().Dot(GetGlobalTransform().GetForwardVector()) > 0;

	case MIRROR_HORIZONTAL:
		return (vecPoint - GetGlobalOrigin()).Normalized().Dot(Vector(0, 0, 1)) > 0;
	}

	return false;
}

const Vector CMirror::GetMirrorFace() const
{
	if (m_eMirrorType == REFLECTION_LATERAL)
		return GetGlobalTransform().GetForwardVector();
	else if (m_eMirrorType == REFLECTION_VERTICAL)
		return Vector(0, 0, 1);

	return Vector();
}

Matrix4x4 CMirror::GetReflection() const
{
	return Matrix4x4().AddReflection(GetMirrorFace());
}

void CMirror::OnUse(CBaseEntity* pUser)
{
	if (!pUser)
		return;

	CPlayerCharacter* pPlayer = dynamic_cast<CPlayerCharacter*>(pUser);
	if (!pPlayer)
		return;

	if (pPlayer->DraggingMirror())
		pPlayer->ReleaseMirror();
	else
		pPlayer->DragMirror(this);
}

void CMirror::SetDragLocation(const Vector& vecLocation)
{
	Vector vecDragEnd = m_hDragTo->GetGlobalOrigin();
	vecDragEnd.z = m_vecOriginalPosition.z;

	Vector vecClosest;
	DistanceToLineSegment(vecLocation, m_vecOriginalPosition, vecDragEnd, &vecClosest);

	SetGlobalOrigin(vecClosest);
}

void UnserializeString_MirrorType(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CMirror* pMirror = static_cast<CMirror*>(pEntity);

	if (sData == "horizontal")
		pMirror->SetMirrorType(MIRROR_HORIZONTAL);
	else
		pMirror->SetMirrorType(MIRROR_VERTICAL);
}


REGISTER_ENTITY(CVerticalMirror);

NETVAR_TABLE_BEGIN(CVerticalMirror);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CVerticalMirror);
	SAVEDATA_EDITOR_VARIABLE("Model");
	SAVEDATA_EDITOR_VARIABLE("DragTo");
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_NETVAR, const char*, m_iModel, "Model", "models/mirror.toy");
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_COPYTYPE, mirror_t, m_eMirrorType, "MirrorType", MIRROR_VERTICAL);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CVerticalMirror);
INPUTS_TABLE_END();


REGISTER_ENTITY(CHorizontalMirror);

NETVAR_TABLE_BEGIN(CHorizontalMirror);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CHorizontalMirror);
	SAVEDATA_EDITOR_VARIABLE("Model");
	SAVEDATA_EDITOR_VARIABLE("DragTo");
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_NETVAR, const char*, m_iModel, "Model", "models/mirror_horizontal.toy");
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_COPYTYPE, mirror_t, m_eMirrorType, "MirrorType", MIRROR_HORIZONTAL);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CHorizontalMirror);
INPUTS_TABLE_END();
