#include "asymmetric_kinematic.h"

#include "grotto_renderer.h"
#include "grotto_playercharacter.h"

REGISTER_ENTITY(CGrottoKinematic);

NETVAR_TABLE_BEGIN(CGrottoKinematic);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CGrottoKinematic);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bReflected);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hNormalPosition, "NormalPosition");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hReflectedPosition, "ReflectedPosition");
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bReflectionAffectsPlayer, "ReflectionAffectsPlayer", true);
	SAVEDATA_EDITOR_VARIABLE("NormalPosition");
	SAVEDATA_EDITOR_VARIABLE("ReflectedPosition");
	SAVEDATA_EDITOR_VARIABLE("ReflectionAffectsPlayer");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrottoKinematic);
INPUTS_TABLE_END();

CGrottoKinematic::CGrottoKinematic()
{
	m_bReflected = false;
}

void CGrottoKinematic::PostLoad()
{
	BaseClass::PostLoad();

	TAssert(!!m_hNormalPosition && !!m_hReflectedPosition);
}

void CGrottoKinematic::Think()
{
	BaseClass::Think();

	if (!m_hNormalPosition || !m_hReflectedPosition)
		return;

	if (m_bReflected)
		SetGlobalTransform(m_hReflectedPosition->GetGlobalTransform());
	else
		SetGlobalTransform(m_hNormalPosition->GetGlobalTransform());
}

const Matrix4x4 CGrottoKinematic::GetRenderTransform() const
{
	if (!m_hNormalPosition || !m_hReflectedPosition)
		return BaseClass::GetRenderTransform();

	if (GrottoRenderer()->IsRenderingReflection() ^ m_bReflected)
		return m_hReflectedPosition->GetGlobalTransform();
	else
		return m_hNormalPosition->GetGlobalTransform();
}

void CGrottoKinematic::Reflected(Matrix4x4& mNewPlayerLocal)
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
