#include "asymmetric_kinematic.h"

#include <renderer/game_renderingcontext.h>

#include "grotto_renderer.h"
#include "grotto_playercharacter.h"

REGISTER_ENTITY(CAsymmetricKinematic);

NETVAR_TABLE_BEGIN(CAsymmetricKinematic);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CAsymmetricKinematic);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bReflected);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hNormalPosition, "NormalPosition");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hReflectedPosition, "ReflectedPosition");
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bReflectionAffectsPlayer, "ReflectionAffectsPlayer", true);
	SAVEDATA_EDITOR_VARIABLE("NormalPosition");
	SAVEDATA_EDITOR_VARIABLE("ReflectedPosition");
	SAVEDATA_EDITOR_VARIABLE("ReflectionAffectsPlayer");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CAsymmetricKinematic);
INPUTS_TABLE_END();

CAsymmetricKinematic::CAsymmetricKinematic()
{
	m_bReflected = false;
}

void CAsymmetricKinematic::PostLoad()
{
	BaseClass::PostLoad();

	TAssert(!!m_hNormalPosition && !!m_hReflectedPosition);
}

void CAsymmetricKinematic::Think()
{
	BaseClass::Think();

	if (!m_hNormalPosition || !m_hReflectedPosition)
		return;

	if (m_bReflected)
		SetGlobalTransform(m_hReflectedPosition->GetGlobalTransform());
	else
		SetGlobalTransform(m_hNormalPosition->GetGlobalTransform());
}

const Matrix4x4 CAsymmetricKinematic::GetRenderTransform() const
{
	if (!m_hNormalPosition || !m_hReflectedPosition)
		return BaseClass::GetRenderTransform();

	if (GrottoRenderer()->IsRenderingReflection() ^ m_bReflected ^ GrottoRenderer()->IsRenderingTransparent())
		return m_hReflectedPosition->GetGlobalTransform();
	else
		return m_hNormalPosition->GetGlobalTransform();
}

const TFloat CAsymmetricKinematic::GetBoundingRadius() const
{
	if (!m_hNormalPosition || !m_hReflectedPosition)
		return BaseClass::GetBoundingRadius();

	return m_aabbVisBoundingBox.Size().Length()/2 + (m_hReflectedPosition->GetGlobalOrigin()-m_hNormalPosition->GetGlobalOrigin()).Length();
}

void CAsymmetricKinematic::ModifyContext(class CRenderingContext* pContext) const
{
	pContext->SetColor(Color(0, 0, 255));

	if (GrottoRenderer()->IsRenderingTransparent())
	{
		pContext->SetBlend(BLEND_ALPHA);
		pContext->SetColor(Color(0, 0, 255, 50));
	}

	BaseClass::ModifyContext(pContext);
}

bool CAsymmetricKinematic::ShouldRenderTransparent() const
{
	return true;
}

void CAsymmetricKinematic::Reflected(Matrix4x4& mNewPlayerLocal)
{
	if (!m_hNormalPosition || !m_hReflectedPosition)
		return;

	if (!m_bReflectionAffectsPlayer)
	{
		for (size_t i = 0; i < m_ahMoveChildren.size(); i++)
		{
			if (dynamic_cast<CPlayerCharacter*>(m_ahMoveChildren[i].GetPointer()))
			{
				// Break the connection just before we do this transform. It'll reset on its own.
				m_ahMoveChildren[i]->SetMoveParent(nullptr);

				// mNewPlayerLocal is the new transform just after the reflection. It's in local
				// space but the player's not anymore so let's transform it to global space.
				// Super hacky.
				mNewPlayerLocal = GetGlobalTransform() * mNewPlayerLocal;
				break;
			}
		}
	}

	if (m_bReflected)
		SetGlobalTransform(m_hNormalPosition->GetGlobalTransform());
	else
		SetGlobalTransform(m_hReflectedPosition->GetGlobalTransform());

	m_bReflected = !m_bReflected;
}
