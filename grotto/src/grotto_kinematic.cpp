#include "grotto_kinematic.h"

#include "grotto_renderer.h"

REGISTER_ENTITY(CGrottoKinematic);

NETVAR_TABLE_BEGIN(CGrottoKinematic);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CGrottoKinematic);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bReflected);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hNormalPosition, "NormalPosition");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hReflectedPosition, "ReflectedPosition");
	SAVEDATA_EDITOR_VARIABLE("NormalPosition");
	SAVEDATA_EDITOR_VARIABLE("ReflectedPosition");
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

	if (m_bReflected)
		SetGlobalTransform(m_hReflectedPosition->GetGlobalTransform());
	else
		SetGlobalTransform(m_hNormalPosition->GetGlobalTransform());
}

const Matrix4x4 CGrottoKinematic::GetRenderTransform() const
{
	if (GrottoRenderer()->IsRenderingReflection() ^ m_bReflected)
		return m_hReflectedPosition->GetGlobalTransform();
	else
		return m_hNormalPosition->GetGlobalTransform();
}

void CGrottoKinematic::Reflected()
{
	if (m_bReflected)
		SetGlobalTransform(m_hNormalPosition->GetGlobalTransform());
	else
		SetGlobalTransform(m_hReflectedPosition->GetGlobalTransform());

	m_bReflected = !m_bReflected;
}
