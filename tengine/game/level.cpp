#include "level.h"

#include <datamanager/data.h>
#include <models/models.h>
#include <textures/texturelibrary.h>
#include <game/entities/baseentity.h>

void CLevel::ReadInfoFromData(const CData* pData)
{
	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		OnReadInfo(pChildData);
	}
}

void CLevel::OnReadInfo(const CData* pData)
{
	if (pData->GetKey() == "Name")
		m_sName = pData->GetValueTString();
	else if (pData->GetKey() == "GameMode")
		m_sGameMode = pData->GetValueTString();
}

void CLevel::CreateEntitiesFromData(const CData* pData)
{
	m_aLevelEntities.clear();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() != "Entity")
			continue;

		m_aLevelEntities.push_back(CLevelEntity(pChildData->GetValueTString()));

		CLevelEntity* pEntity = &m_aLevelEntities.back();

		for (size_t k = 0; k < pChildData->GetNumChildren(); k++)
		{
			CData* pField = pChildData->GetChild(k);

			tstring sHandle = pField->GetKey();
			tstring sValue = pField->GetValueTString();

			if (sHandle == "Output")
			{
				tstring sTarget;
				tstring sInput;
				tstring sArgs;
				bool bKill = false;

				for (size_t o = 0; o < pField->GetNumChildren(); o++)
				{
					CData* pOutputData = pField->GetChild(o);

					if (pOutputData->GetKey() == "Target")
						sTarget = pOutputData->GetValueString();
					else if (pOutputData->GetKey() == "Input")
						sInput = pOutputData->GetValueString();
					else if (pOutputData->GetKey() == "Args")
						sArgs = pOutputData->GetValueString();
					else if (pOutputData->GetKey() == "Kill")
						bKill = pOutputData->GetValueBool();
				}

				CLevelEntity::CLevelEntityOutput& oOutput = pEntity->m_aOutputs.push_back();
				oOutput.m_sOutput = sValue;
				oOutput.m_sTargetName = sTarget;
				oOutput.m_sInput = sInput;
				oOutput.m_sArgs = sArgs;
				oOutput.m_bKill = bKill;
			}
			else
			{
				pEntity->m_asParameters[sHandle] = sValue;
			}
		}
	}
}

const tstring& CLevelEntity::GetParameterValue(const tstring& sKey) const
{
	const auto it = m_asParameters.find(sKey);
	if (it == m_asParameters.end())
	{
		static tstring sEmpty = "";
		return sEmpty;
	}

	return it->second;
}

Matrix4x4 CLevelEntity::CalculateGlobalTransform(CLevelEntity* pThis)
{
	Matrix4x4 mLocal;

	tstring sMoveParent = pThis->GetParameterValue("MoveParent");
	if (sMoveParent.length())
		TAssert(false);

	tstring sLocalOrigin = pThis->GetParameterValue("LocalOrigin");
	if (sLocalOrigin.length())
		mLocal.SetTranslation(UnserializeString_TVector(sLocalOrigin));

	tstring sLocalAngles = pThis->GetParameterValue("LocalAngles");
	if (sLocalAngles.length())
		mLocal.SetAngles(UnserializeString_EAngle(sLocalAngles));

	return mLocal;
}

bool CLevelEntity::CalculateVisible(CLevelEntity* pThis)
{
	tstring sVisible = pThis->GetParameterValue("Visible");
	if (sVisible.length())
		return UnserializeString_bool(sVisible);

	return true;
}

size_t CLevelEntity::CalculateModelID(CLevelEntity* pThis)
{
	tstring sModel = pThis->GetParameterValue("Model");
	return CModelLibrary::FindModel(sModel);
}

size_t CLevelEntity::CalculateTextureID(CLevelEntity* pThis)
{
	tstring sTexture = pThis->GetParameterValue("Model");
	return CTextureLibrary::FindTextureID(sTexture);
}

Vector2D CLevelEntity::CalculateTextureModelScale(CLevelEntity* pThis)
{
	tstring sScale = pThis->GetParameterValue("TextureScale");

	if (sScale.length())
		return UnserializeString_Vector2D(sScale);

	return *((Vector2D*)&CBaseEntity::GetSaveDataByHandle(("C" + pThis->m_sClass).c_str(), "TextureScale")->m_oDefault);
}

AABB CLevelEntity::CalculateBoundingBox(CLevelEntity* pThis)
{
	size_t iModel = pThis->GetModelID();
	CModel* pModel = CModelLibrary::GetModel(iModel);

	if (pModel)
		return pModel->m_aabbBoundingBox;

	tstring sAABB = pThis->GetParameterValue("BoundingBox");

	if (sAABB.length())
		return UnserializeString_AABB(sAABB, pThis->GetName(), pThis->m_sClass, "BoundingBox");

	CSaveData* pSaveData = CBaseEntity::GetSaveDataByHandle(tstring("C"+pThis->m_sClass).c_str(), "BoundingBox");
	if (pSaveData)
	{
		AABB aabbBounds;
		memcpy(&aabbBounds, &pSaveData->m_oDefault, sizeof(aabbBounds));
		return aabbBounds;
	}

	return AABB(Vector(-0.5f, -0.5f, -0.5f), Vector(0.5f, 0.5f, 0.5f));
}

tstring CLevelEntity::CalculateName(CLevelEntity* pThis)
{
	return pThis->GetParameterValue("Name");
}
