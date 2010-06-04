#include "particles.h"

#include <maths.h>
#include <game/game.h>
#include <shaders/shaders.h>

#include "renderer.h"

#include <ui/debugdraw.h>

CParticleSystemLibrary* CParticleSystemLibrary::s_pParticleSystemLibrary = NULL;
static CParticleSystemLibrary g_pParticleSystemLibrary = CParticleSystemLibrary();

extern void InitSystems();

CParticleSystemLibrary::CParticleSystemLibrary()
{
	s_pParticleSystemLibrary = this;

	m_iSystemInstanceIndex = 0;

	InitSystems();
}

CParticleSystemLibrary::~CParticleSystemLibrary()
{
	for (size_t i = 0; i < m_apParticleSystems.size(); i++)
	{
		delete m_apParticleSystems[i];
	}

	s_pParticleSystemLibrary = NULL;
}

size_t CParticleSystemLibrary::AddParticleSystem(const std::wstring& sName)
{
	m_apParticleSystems.push_back(new CParticleSystem(sName));

	return m_apParticleSystems.size()-1;
}

size_t CParticleSystemLibrary::FindParticleSystem(const std::wstring& sName)
{
	for (size_t i = 0; i < m_apParticleSystems.size(); i++)
	{
		if (wcscmp(m_apParticleSystems[i]->GetName(), sName.c_str()) == 0)
			return i;
	}

	return ~0;
}

void CParticleSystemLibrary::LoadParticleSystem(size_t iSystem)
{
	if (iSystem >= m_apParticleSystems.size())
		return;

	m_apParticleSystems[iSystem]->Load();
}

CParticleSystem* CParticleSystemLibrary::GetParticleSystem(size_t i)
{
	if (i >= m_apParticleSystems.size())
		return NULL;

	return m_apParticleSystems[i];
}

void CParticleSystemLibrary::Simulate()
{
	CParticleSystemLibrary* pPSL = Get();

	std::map<size_t, CSystemInstance*>::iterator it = pPSL->m_apInstances.begin();

	std::vector<size_t> aiDeleted;

	for (; it != pPSL->m_apInstances.end(); it++)
	{
		CSystemInstance* pInstance = (*it).second;

		pInstance->Simulate();

		if (pInstance->IsStopped() && pInstance->GetNumParticles() == 0)
			aiDeleted.push_back((*it).first);
	}

	for (size_t i = 0; i < aiDeleted.size(); i++)
		RemoveInstance(aiDeleted[i]);
}

void CParticleSystemLibrary::Render()
{
	CParticleSystemLibrary* pPSL = Get();
	std::map<size_t, CSystemInstance*>::iterator it = pPSL->m_apInstances.begin();

	for (; it != pPSL->m_apInstances.end(); it++)
		(*it).second->Render();
}

size_t CParticleSystemLibrary::AddInstance(const std::wstring& sName, Vector vecOrigin)
{
	return AddInstance(CParticleSystemLibrary::Get()->FindParticleSystem(sName), vecOrigin);
}

size_t CParticleSystemLibrary::AddInstance(size_t iParticleSystem, Vector vecOrigin)
{
	CParticleSystemLibrary* pPSL = Get();
	CParticleSystem* pSystem = pPSL->GetParticleSystem(iParticleSystem);

	if (!pSystem)
		return ~0;

	pPSL->m_apInstances.insert(std::pair<size_t, CSystemInstance*>(pPSL->m_iSystemInstanceIndex++, new CSystemInstance(pSystem, vecOrigin)));
	return pPSL->m_iSystemInstanceIndex-1;
}

void CParticleSystemLibrary::StopInstance(size_t iInstance)
{
	CSystemInstance* pInstance = GetInstance(iInstance);

	if (!pInstance)
		return;

	pInstance->Stop();
}

void CParticleSystemLibrary::RemoveInstance(size_t iInstance)
{
	std::map<size_t, CSystemInstance*>::iterator it = Get()->m_apInstances.find(iInstance);

	if (it == Get()->m_apInstances.end())
		return;

	delete (*it).second;
	Get()->m_apInstances.erase(it);
}

CSystemInstance* CParticleSystemLibrary::GetInstance(size_t iInstance)
{
	if (Get()->m_apInstances.find(iInstance) == Get()->m_apInstances.end())
		return NULL;

	return Get()->m_apInstances[iInstance];
}

CParticleSystem::CParticleSystem(std::wstring sName)
{
	m_bLoaded = false;
	m_sName = sName;

	m_iTexture = 0;

	m_flLifeTime = 1.0f;
	m_flEmissionRate = 0.1f;
	m_iEmissionMax = 0;
	m_flAlpha = 1.0f;
	m_flStartRadius = 1.0f;
	m_flEndRadius = 1.0f;
	m_flFadeOut = 0.25f;
	m_flInheritedVelocity = 0.0f;
	m_flDrag = 1.0f;
	m_bRandomBillboardYaw = false;
}

void CParticleSystem::Load()
{
	if (IsLoaded())
		return;

	m_bLoaded = true;

	SetTexture(CRenderer::LoadTextureIntoGL(GetTextureName()));

	for (size_t i = 0; i < GetNumChildren(); i++)
		CParticleSystemLibrary::Get()->GetParticleSystem(GetChild(i))->Load();
}

void CParticleSystem::AddChild(size_t iSystem)
{
	m_aiChildren.push_back(iSystem);
}

CSystemInstance::CSystemInstance(CParticleSystem* pSystem, Vector vecOrigin)
{
	m_pSystem = pSystem;
	m_vecOrigin = vecOrigin;

	m_bStopped = false;

	m_iNumParticlesAlive = 0;

	m_flLastEmission = 0;
	m_iTotalEmitted = 0;

	CParticleSystemLibrary* pPSL = CParticleSystemLibrary::Get();

	for (size_t i = 0; i < m_pSystem->GetNumChildren(); i++)
		m_apChildren.push_back(new CSystemInstance(pPSL->GetParticleSystem(m_pSystem->GetChild(i)), vecOrigin));
}

CSystemInstance::~CSystemInstance()
{
	for (size_t i = 0; i < m_pSystem->GetNumChildren(); i++)
		delete m_apChildren[i];
}

void CSystemInstance::Simulate()
{
	float flGameTime = Game()->GetGameTime();
	float flFrameTime = Game()->GetFrameTime();

	if (m_hFollow != NULL)
	{
		m_vecOrigin = m_hFollow->GetOrigin();
		m_vecInheritedVelocity = m_hFollow->GetVelocity();
	}

	for (size_t i = 0; i < m_aParticles.size(); i++)
	{
		CParticle* pParticle = &m_aParticles[i];

		if (!pParticle->m_bActive)
			continue;

		float flLifeTime = flGameTime - pParticle->m_flSpawnTime;
		if (flLifeTime > m_pSystem->GetLifeTime())
		{
			pParticle->m_bActive = false;
			m_iNumParticlesAlive--;
			continue;
		}

		pParticle->m_vecOrigin += pParticle->m_vecVelocity * flFrameTime;
		pParticle->m_vecVelocity += Vector(0, -10, 0) * flFrameTime;
		pParticle->m_vecVelocity *= (1-((1-m_pSystem->GetDrag()) * flFrameTime));

		float flLifeTimeRamp = flLifeTime / m_pSystem->GetLifeTime();

		if (flLifeTimeRamp < m_pSystem->GetFadeOut())
			pParticle->m_flAlpha = RemapVal(flLifeTimeRamp, 1-m_pSystem->GetFadeOut(), 1, m_pSystem->GetAlpha(), 0);

		pParticle->m_flRadius = RemapVal(flLifeTimeRamp, 0, 1, m_pSystem->GetStartRadius(), m_pSystem->GetEndRadius());
	}

	if (!m_bStopped && flGameTime - m_flLastEmission > m_pSystem->GetEmissionRate() && m_pSystem->GetTexture())
	{
		if (!m_pSystem->GetEmissionMax() || m_iTotalEmitted < m_pSystem->GetEmissionMax())
		{
			SpawnParticle();
			m_flLastEmission = flGameTime;
		}
	}

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->Simulate();

	if (m_pSystem->GetEmissionMax() && m_iTotalEmitted >= m_pSystem->GetEmissionMax())
		m_bStopped = true;
}

void CSystemInstance::SpawnParticle()
{
	m_iNumParticlesAlive++;
	m_iTotalEmitted++;

	CParticle* pNewParticle = NULL;

	for (size_t i = 0; i < m_aParticles.size(); i++)
	{
		CParticle* pParticle = &m_aParticles[i];

		if (pParticle->m_bActive)
			continue;

		pNewParticle = pParticle;
		break;
	}

	if (!pNewParticle)
	{
		m_aParticles.push_back(CParticle());
		pNewParticle = &m_aParticles[m_aParticles.size()-1];
	}

	pNewParticle->Reset();
	pNewParticle->m_vecOrigin = m_vecOrigin;
	pNewParticle->m_vecVelocity = m_vecInheritedVelocity * m_pSystem->GetInheritedVelocity();

	if (m_pSystem->GetRandomVelocity().Size().LengthSqr() > 0)
	{
		pNewParticle->m_vecVelocity.x += RemapVal((float)(rand()%1000), 0, 1000, m_pSystem->GetRandomVelocity().m_vecMins.x, m_pSystem->GetRandomVelocity().m_vecMaxs.x);
		pNewParticle->m_vecVelocity.y += RemapVal((float)(rand()%1000), 0, 1000, m_pSystem->GetRandomVelocity().m_vecMins.y, m_pSystem->GetRandomVelocity().m_vecMaxs.y);
		pNewParticle->m_vecVelocity.z += RemapVal((float)(rand()%1000), 0, 1000, m_pSystem->GetRandomVelocity().m_vecMins.z, m_pSystem->GetRandomVelocity().m_vecMaxs.z);
	}

	pNewParticle->m_flAlpha = m_pSystem->GetAlpha();
	pNewParticle->m_flRadius = m_pSystem->GetStartRadius();

	if (m_pSystem->GetRandomBillboardYaw())
		pNewParticle->m_flBillboardYaw = RemapVal((float)(rand()%1000), 0, 1000, 0, 360);
	else
		pNewParticle->m_flBillboardYaw = 0;
}

void CSystemInstance::Render()
{
	// Render children first so that the contexts don't muss each other up.
	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->Render();

	CRenderer* pRenderer = Game()->GetRenderer();

	Vector vecForward, vecRight, vecUp;
	pRenderer->GetCameraVectors(&vecForward, &vecRight, &vecUp);

	CRenderingContext c(Game()->GetRenderer());

	c.BindTexture(m_pSystem->GetTexture());
	c.SetBlend(BLEND_ADDITIVE);
	c.SetDepthMask(false);
	c.UseProgram(CShaderLibrary::GetModelProgram());
	c.SetUniform("bDiffuse", true);
	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bColorSwapInAlpha", false);

	for (size_t i = 0; i < m_aParticles.size(); i++)
	{
		CParticle* pParticle = &m_aParticles[i];

		if (!pParticle->m_bActive)
			continue;

		float flRadius = pParticle->m_flRadius;
		Vector vecOrigin = pParticle->m_vecOrigin;

		Vector vecParticleUp, vecParticleRight;

		if (fabs(pParticle->m_flBillboardYaw) > 0.01f)
		{
			float flYaw = pParticle->m_flBillboardYaw*M_PI/180;
			float flSin = sin(flYaw);
			float flCos = cos(flYaw);

			vecParticleUp = (flCos*vecUp + flSin*vecRight)*flRadius;
			vecParticleRight = (flCos*vecRight - flSin*vecUp)*flRadius;
		}
		else
		{
			vecParticleUp = vecUp*flRadius;
			vecParticleRight = vecRight*flRadius;
		}

		Vector vecTL = vecOrigin - vecParticleRight + vecParticleUp;
		Vector vecTR = vecOrigin + vecParticleRight + vecParticleUp;
		Vector vecBL = vecOrigin - vecParticleRight - vecParticleUp;
		Vector vecBR = vecOrigin + vecParticleRight - vecParticleUp;

		c.SetUniform("flAlpha", pParticle->m_flAlpha);

		c.BeginRenderQuads();

		c.TexCoord(0, 1);
		c.Vertex(vecTL);
		c.TexCoord(0, 0);
		c.Vertex(vecBL);
		c.TexCoord(1, 0);
		c.Vertex(vecBR);
		c.TexCoord(1, 1);
		c.Vertex(vecTR);

		c.EndRender();
	}
}

void CSystemInstance::FollowEntity(CBaseEntity* pFollow)
{
	m_hFollow = pFollow;

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->FollowEntity(pFollow);
}

void CSystemInstance::SetInheritedVelocity(Vector vecInheritedVelocity)
{
	m_vecInheritedVelocity = vecInheritedVelocity;

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->SetInheritedVelocity(vecInheritedVelocity);
}

void CSystemInstance::Stop()
{
	m_bStopped = true;

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->Stop();
}

bool CSystemInstance::IsStopped()
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
	{
		if (!m_apChildren[i]->IsStopped())
			return false;
	}

	return m_bStopped;
}

size_t CSystemInstance::GetNumParticles()
{
	size_t iAlive = 0;
	for (size_t i = 0; i < m_apChildren.size(); i++)
		iAlive += m_apChildren[i]->GetNumParticles();

	return iAlive + m_iNumParticlesAlive;
}

CParticle::CParticle()
{
	Reset();
}

void CParticle::Reset()
{
	m_vecOrigin = m_vecVelocity = Vector();
	m_flAlpha = 1;
	m_flRadius = 1;
	m_bActive = true;
	m_flSpawnTime = Game()->GetGameTime();
}