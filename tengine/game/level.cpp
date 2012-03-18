#include "level.h"

#include <iostream>
#include <fstream>

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

void CLevel::SaveToFile()
{
	if (!m_sFile.length())
	{
		TAssert(m_sFile.length());
		TError("Can't find level file \'" + m_sFile + "\' to save.\n");
		return;
	}

	std::basic_ofstream<tchar> f(m_sFile.c_str());

	tstring sMessage = "// Generated by the Tinker Engine\n// Feel free to modify\n\n";
	f.write(sMessage.data(), sMessage.length());

	tstring sName = "Name: " + m_sName + "\n";
	f.write(sName.data(), sName.length());

	tstring sGameMode = "GameMode: " + m_sGameMode + "\n\n";
	f.write(sGameMode.data(), sGameMode.length());

	for (size_t i = 0; i < m_aLevelEntities.size(); i++)
	{
		auto pEntity = &m_aLevelEntities[i];

		tstring sEntity = "Entity: " + pEntity->GetClass() + "\n{\n";
		f.write(sEntity.data(), sEntity.length());

		if (pEntity->GetName().length())
		{
			tstring sName = "\tName: " + pEntity->GetName() + "\n";
			f.write(sName.data(), sName.length());
		}

		for (auto it = pEntity->GetParameters().begin(); it != pEntity->GetParameters().end(); it++)
		{
			tstring sName = "\t" + it->first + ": " + it->second + "\n";
			f.write(sName.data(), sName.length());
		}

		for (auto it = pEntity->GetOutputs().begin(); it != pEntity->GetOutputs().end(); it++)
		{
			auto pOutput = &pEntity->GetOutputs()[i];
			tstring sOutput = "\n\tOutput: " + pOutput->m_sOutput + "\n{\n";
			f.write(sOutput.data(), sOutput.length());

			if (pOutput->m_sTargetName.length())
			{
				tstring sTarget = "\t\tTarget: " + pOutput->m_sTargetName + "\n";
				f.write(sTarget.data(), sTarget.length());
			}

			if (pOutput->m_sInput.length())
			{
				tstring sInput = "\t\tInput: " + pOutput->m_sInput + "\n";
				f.write(sInput.data(), sInput.length());
			}

			if (pOutput->m_sArgs.length())
			{
				tstring sArgs = "\t\tArgs: " + pOutput->m_sArgs + "\n";
				f.write(sArgs.data(), sArgs.length());
			}

			if (pOutput->m_bKill)
			{
				tstring sKill = "\t\tKill: yes\n";
				f.write(sKill.data(), sKill.length());
			}

			tstring sClose = "\t}\n";
			f.write(sClose.data(), sClose.length());
		}

		tstring sClose = "}\n\n";
		f.write(sClose.data(), sClose.length());
	}

	TMsg("Wrote level file '" + m_sFile + "'\n");
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

				CLevelEntity::CLevelEntityOutput& oOutput = pEntity->GetOutputs().push_back();
				oOutput.m_sOutput = sValue;
				oOutput.m_sTargetName = sTarget;
				oOutput.m_sInput = sInput;
				oOutput.m_sArgs = sArgs;
				oOutput.m_bKill = bKill;
			}
			else
			{
				pEntity->SetParameterValue(sHandle, sValue);
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

void CLevelEntity::SetParameterValue(const tstring& sKey, const tstring& sValue)
{
	auto it = m_asParameters.find(sKey);
	if (it == m_asParameters.end())
	{
		if (!sValue.length())
			return;

		m_asParameters[sKey] = sValue;
	}
	else
	{
		if (!sValue.length())
		{
			m_asParameters.erase(sKey);
			Dirtify();
			return;
		}

		tstring& sCurrentValue = it->second;
		if (sCurrentValue == sValue)
			return;

		sCurrentValue = sValue;
	}

	Dirtify();

	CSaveData oSaveData;
	CSaveData* pSaveData = CBaseEntity::FindSaveDataValuesByHandle(("C" + GetClass()).c_str(), sKey.c_str(), &oSaveData);

	if (pSaveData->m_bDefault)
	{
		if (strcmp(pSaveData->m_pszType, "bool") == 0)
		{
			bool bValue = UnserializeString_bool(sValue);

			bool b = *((bool*)&pSaveData->m_oDefault[0]);
			if (bValue == b)
				m_asParameters.erase(sKey);
		}
		else
		{
			if (strcmp(pSaveData->m_pszType, "size_t") == 0)
			{
				size_t i = *((size_t*)&pSaveData->m_oDefault[0]);
				if (stoi(sValue) == i)
					m_asParameters.erase(sKey);
			}
			else if (strcmp(pSaveData->m_pszType, "float") == 0)
			{
				float f = *((float*)&pSaveData->m_oDefault[0]);
				if (stof(sValue) == f)
					m_asParameters.erase(sKey);
			}
			else if (strcmp(pSaveData->m_pszType, "Vector") == 0)
			{
				if (CanUnserializeString_TVector(sValue))
				{
					Vector v = *((Vector*)&pSaveData->m_oDefault[0]);
					if (UnserializeString_TVector(sValue) == v)
						m_asParameters.erase(sKey);
				}
			}
			else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
			{
				if (CanUnserializeString_Vector2D(sValue))
				{
					Vector2D v = *((Vector2D*)&pSaveData->m_oDefault[0]);
					if (UnserializeString_Vector2D(sValue) == v)
						m_asParameters.erase(sKey);
				}
			}
			else if (strcmp(pSaveData->m_pszType, "EAngle") == 0)
			{
				if (CanUnserializeString_EAngle(sValue))
				{
					EAngle v = *((EAngle*)&pSaveData->m_oDefault[0]);
					if (UnserializeString_EAngle(sValue) == v)
						m_asParameters.erase(sKey);
				}
			}
			else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
			{
				if (CanUnserializeString_Matrix4x4(sValue))
				{
					Matrix4x4 m = *((Matrix4x4*)&pSaveData->m_oDefault[0]);
					if (UnserializeString_Matrix4x4(sValue) == m)
						m_asParameters.erase(sKey);
				}
			}
			else if (strcmp(pSaveData->m_pszType, "AABB") == 0)
			{
				if (CanUnserializeString_AABB(sValue))
				{
					AABB b = *((AABB*)&pSaveData->m_oDefault[0]);
					if (UnserializeString_AABB(sValue) == b)
						m_asParameters.erase(sKey);
				}
			}
			else
			{
				TAssert(false);
			}
		}
	}
}

void CLevelEntity::RemoveParameter(const tstring& sKey)
{
	m_asParameters.erase(sKey);
	Dirtify();
}

bool CLevelEntity::HasParameterValue(const tstring& sKey) const
{
	return m_asParameters.find(sKey) != m_asParameters.end();
}

Matrix4x4 CLevelEntity::CalculateGlobalTransform(CLevelEntity* pThis)
{
	Matrix4x4 mLocal;

	tstring sMoveParent = pThis->GetParameterValue("MoveParent");
	if (sMoveParent.length())
		TAssert(false);

	tstring sLocalOrigin = pThis->GetParameterValue("LocalOrigin");
	if (sLocalOrigin.length() && CanUnserializeString_TVector(sLocalOrigin))
		mLocal.SetTranslation(UnserializeString_TVector(sLocalOrigin));

	tstring sLocalAngles = pThis->GetParameterValue("LocalAngles");
	if (sLocalAngles.length() && CanUnserializeString_EAngle(sLocalAngles))
		mLocal.SetAngles(UnserializeString_EAngle(sLocalAngles));

	return mLocal;
}

bool CLevelEntity::CalculateVisible(CLevelEntity* pThis)
{
	tstring sVisible = pThis->GetParameterValue("Visible");
	if (sVisible.length() && CanUnserializeString_bool(sVisible))
		return UnserializeString_bool(sVisible);

	return *((bool*)&CBaseEntity::FindSaveDataByHandle(("C" + pThis->m_sClass).c_str(), "Visible")->m_oDefault);
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

	if (CanUnserializeString_Vector2D(sScale))
		return UnserializeString_Vector2D(sScale);

	return *((Vector2D*)&CBaseEntity::FindSaveDataByHandle(("C" + pThis->m_sClass).c_str(), "TextureScale")->m_oDefault);
}

AABB CLevelEntity::CalculateBoundingBox(CLevelEntity* pThis)
{
	size_t iModel = pThis->GetModelID();
	CModel* pModel = CModelLibrary::GetModel(iModel);

	if (pModel)
		return pModel->m_aabbBoundingBox;

	tstring sAABB = pThis->GetParameterValue("BoundingBox");

	if (CanUnserializeString_AABB(sAABB))
		return UnserializeString_AABB(sAABB, pThis->GetName(), pThis->m_sClass, "BoundingBox");

	CSaveData* pSaveData = CBaseEntity::FindSaveDataByHandle(tstring("C"+pThis->m_sClass).c_str(), "BoundingBox");
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
