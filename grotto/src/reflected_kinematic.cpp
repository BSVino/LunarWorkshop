#include "reflected_kinematic.h"

#include <renderer/game_renderingcontext.h>

#include "grotto_renderer.h"
#include "grotto_playercharacter.h"

REGISTER_ENTITY(CReflectedKinematic);

NETVAR_TABLE_BEGIN(CReflectedKinematic);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CReflectedKinematic);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bReflected);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hMirror, "Mirror");
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bReflectionAffectsPlayer, "ReflectionAffectsPlayer", true);
	SAVEDATA_EDITOR_VARIABLE("Mirror");
	SAVEDATA_EDITOR_VARIABLE("ReflectionAffectsPlayer");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectedKinematic);
INPUTS_TABLE_END();

CReflectedKinematic::CReflectedKinematic()
{
	m_bReflected = false;
}

void CReflectedKinematic::PostLoad()
{
	BaseClass::PostLoad();

	TAssert(IsValid());
}

void CReflectedKinematic::Think()
{
	BaseClass::Think();

	if (!IsValid())
		return;
}

const Matrix4x4 CReflectedKinematic::GetRenderTransform() const
{
	if (!IsValid())
		return BaseClass::GetRenderTransform();

	if (GrottoRenderer()->IsRenderingReflection() ^ GrottoRenderer()->IsRenderingTransparent())
		return ReflectProperly();
	else
		return BaseClass::GetRenderTransform();
}

const TFloat CReflectedKinematic::GetBoundingRadius() const
{
	if (!IsValid())
		return BaseClass::GetBoundingRadius();

	return m_aabbVisBoundingBox.Size().Length()/2 + (GetMirror()->GetGlobalOrigin()-GetGlobalOrigin()).Length();
}

void CReflectedKinematic::ModifyContext(class CRenderingContext* pContext) const
{
	pContext->SetColor(Color(0, 0, 255));

	BaseClass::ModifyContext(pContext);
}

void CReflectedKinematic::Reflected(Matrix4x4& mNewPlayerLocal)
{
	if (!IsValid())
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

	SetGlobalTransform(ReflectProperly());

	m_bReflected = !m_bReflected;
}

const Matrix4x4 CReflectedKinematic::ReflectProperly() const
{
	Matrix4x4 mMirror = m_hMirror->GetGlobalTransform();
	mMirror.SetAngles(EAngle(0, 0, 0));
	Matrix4x4 mMirrorToLocal = mMirror.InvertedRT();
	Matrix4x4 mReflection = GetMirror()->GetReflection();

	Matrix4x4 mReflect = mMirror * mReflection * mMirrorToLocal;
	Matrix4x4 mReflected = mReflect * GetGlobalTransform();

	// The physics system can't handle anything that's been reflected.
	// So, reverse the reflection, and for now we'll only reflect symmetric objects this way.
	Matrix4x4 mFixup = mReflected;
	mFixup.SetAngles(GetGlobalTransform().GetAngles());

	return mFixup;
}

CMirror* CReflectedKinematic::GetMirror() const
{
	return dynamic_cast<CMirror*>(m_hMirror.GetPointer());
}

bool CReflectedKinematic::IsValid() const
{
	return !!GetMirror();
}
