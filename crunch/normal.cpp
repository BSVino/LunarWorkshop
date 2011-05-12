#include "crunch.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>

#include <raytracer/raytracer.h>
#include <maths.h>

#if 0
#ifdef _DEBUG
#define NORMAL_DEBUG
#endif
#endif

#ifdef NORMAL_DEBUG
#include "ui/modelwindow.h"
#endif

CNormalGenerator::CNormalGenerator(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
{
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	m_bPixelMask = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;

	m_iMaterial = 0;
	m_iNormal2GLId = 0;
	m_iNormal2ILId = 0;
	m_aflTextureTexels = NULL;
	m_aflMidPassTexels = NULL;
	m_aflLowPassTexels = NULL;
	m_aflNormal2Texels = NULL;
	m_bNewNormal2Available = false;
	m_bNormal2Generated = false;

	m_pGenerationParallelizer = NULL;
	m_pNormal2Parallelizer = NULL;
}

CNormalGenerator::~CNormalGenerator()
{
	free(m_bPixelMask);

	if (m_iNormal2GLId)
		glDeleteTextures(1, &m_iNormal2GLId);

	if (m_aflNormal2Texels)
	{
		delete[] m_aflTextureTexels;
		delete[] m_aflMidPassTexels;
		delete[] m_aflLowPassTexels;
		delete[] m_aflNormal2Texels;
	}
}

void CNormalGenerator::Think()
{
	if (IsSetupComplete() && m_bNormal2Changed)
	{
		StartGenerationJobs();
	}
}

void CNormalGenerator::SaveToFile(const wchar_t *pszFilename)
{
	if (!pszFilename)
		return;

	ilEnable(IL_FILE_OVERWRITE);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	if (m_aflNormal2Texels)
		ilTexImage((ILint)m_iNormal2Width, (ILint)m_iNormal2Height, 1, 3, IL_RGB, IL_FLOAT, &m_aflNormal2Texels[0]);

	// Formats like PNG and VTF don't work unless it's in integer format.
	ilConvertImage(IL_RGB, IL_UNSIGNED_INT);

	if (!ModelWindow()->IsRegistered() && (m_iNormal2Width > 128 || m_iNormal2Height > 128))
	{
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale(128, 128, 1);
	}

	ilSaveImage(pszFilename);

	ilDeleteImages(1,&iDevILId);
}

bool CNormalGenerator::Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask)
{
	if (w < 0 || h < 0 || w >= tw || h >= th)
		return false;

	iTexel = th*h + w;

	TAssert(iTexel >= 0 && iTexel < tw * th);

	if (abMask && !abMask[iTexel])
		return false;

	return true;
}

typedef struct
{
	CNormalGenerator*	pGenerator;
	size_t				x;
	size_t				y;
} normal2_data_t;

void NormalizeHeightValue(void* pVoidData)
{
	normal2_data_t* pJobData = (normal2_data_t*)pVoidData;

	pJobData->pGenerator->NormalizeHeightValue(pJobData->x, pJobData->y);
}

void CNormalGenerator::NormalizeHeightValue(size_t x, size_t y)
{
	if (!m_aflTextureTexels)
		return;

	float flHiScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/200.0f * m_flNormalTextureDepth;
	float flMidScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/100.0f * m_flNormalTextureDepth;
	float flLowScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/50.0f * m_flNormalTextureDepth;

	size_t iTexel;
	Texel(x, y, iTexel, m_iNormal2Width, m_iNormal2Height, false);

	eastl::vector<Vector> avecHeights;

	float flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
	float flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
	float flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;

	Vector vecCenter((float)x, (float)y, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
	Vector vecNormal(0,0,0);

	if (Texel(x+1, y, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor(x+1.0f, (float)y, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, 1, 0));
	}

	if (Texel(x-1, y, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor(x-1.0f, (float)y, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, -1, 0));
	}

	if (Texel(x, y+1, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor((float)x, y+1.0f, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(-1, 0, 0));
	}

	if (Texel(x, y-1, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flMidPass = m_aflMidPassTexels[iTexel] * flMidScale;
		flLowPass = m_aflLowPassTexels[iTexel] * flLowScale;
		Vector vecNeighbor((float)x, y-1.0f, flHeight*m_flNormalTextureHiDepth + flMidPass*m_flNormalTextureMidDepth + flLowPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(1, 0, 0));
	}

	vecNormal.Normalize();

	for (size_t i = 0; i < 3; i++)
		vecNormal[i] = RemapVal(vecNormal[i], -1.0f, 1.0f, 0.0f, 0.99f);	// Don't use 1.0 because of integer overflow.

	// Don't need to lock the data because we're guaranteed never to access the same texel twice due to the generation method.
	m_aflNormal2Texels[iTexel*3] = vecNormal.x;
	m_aflNormal2Texels[iTexel*3+1] = vecNormal.y;
	m_aflNormal2Texels[iTexel*3+2] = vecNormal.z;
}

typedef struct
{
	CNormalGenerator*	pGenerator;
	size_t				x;
	size_t				y;
} generatepass_data_t;

void GeneratePass(void* pVoidData)
{
	generatepass_data_t* pJobData = (generatepass_data_t*)pVoidData;

	pJobData->pGenerator->GeneratePass(pJobData->x, pJobData->y);
}

void CNormalGenerator::GeneratePass(int x, int y)
{
	// Generate the low/mid pass values from a fast gaussian filter.
	static const float flWeightTable[11][11] = {
		{ 0.004f, 0.004f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.004f, 0.004f },
		{ 0.004f, 0.005f, 0.005f, 0.006f, 0.007f, 0.007f, 0.007f, 0.006f, 0.005f, 0.005f, 0.004f },
		{ 0.005f, 0.005f, 0.006f, 0.008f, 0.009f, 0.009f, 0.009f, 0.008f, 0.006f, 0.005f, 0.005f },
		{ 0.005f, 0.006f, 0.008f, 0.010f, 0.012f, 0.014f, 0.012f, 0.010f, 0.008f, 0.006f, 0.005f },
		{ 0.005f, 0.007f, 0.009f, 0.012f, 0.019f, 0.027f, 0.019f, 0.012f, 0.009f, 0.007f, 0.005f },
		{ 0.005f, 0.007f, 0.009f, 0.014f, 0.027f, 0.054f, 0.027f, 0.014f, 0.009f, 0.007f, 0.005f },
		{ 0.005f, 0.007f, 0.009f, 0.012f, 0.019f, 0.027f, 0.019f, 0.012f, 0.009f, 0.007f, 0.005f },
		{ 0.005f, 0.006f, 0.008f, 0.010f, 0.012f, 0.014f, 0.012f, 0.010f, 0.008f, 0.006f, 0.005f },
		{ 0.005f, 0.005f, 0.006f, 0.008f, 0.009f, 0.009f, 0.009f, 0.008f, 0.006f, 0.005f, 0.005f },
		{ 0.004f, 0.005f, 0.005f, 0.006f, 0.007f, 0.007f, 0.007f, 0.006f, 0.005f, 0.005f, 0.004f },
		{ 0.004f, 0.004f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.004f, 0.004f },
	};

	float flLowHeight = 0;
	float flTotalLowHeight = 0;
	float flMidHeight = 0;
	float flTotalMidHeight = 0;

	for (int i = -10; i <= 10; i++)
	{
		for (int j = -10; j <= 10; j++)
		{
			size_t iTexel2;
			if (Texel(x+i, y+j, iTexel2, m_iNormal2Width, m_iNormal2Height))
			{
				size_t iTexelOffset = iTexel2*3;
				float flWeight = flWeightTable[(i/2)+5][(j/2)+5];
				flLowHeight += (m_aflTextureTexels[iTexelOffset]+m_aflTextureTexels[iTexelOffset+1]+m_aflTextureTexels[iTexelOffset+2])/3 * flWeight;
				flTotalLowHeight += flWeight;

				if (i >= -5 && i <= 5 && j >= -5 && j <= 5)
				{
					flWeight = flWeightTable[i+5][j+5];
					flMidHeight += (m_aflTextureTexels[iTexelOffset]+m_aflTextureTexels[iTexelOffset+1]+m_aflTextureTexels[iTexelOffset+2])/3 * flWeight;
					flTotalMidHeight += flWeight;
				}
			}
		}
	}

	size_t iTexel;
	Texel(x, y, iTexel, m_iNormal2Width, m_iNormal2Height);

	m_aflLowPassTexels[iTexel] = flLowHeight/flTotalLowHeight;
	m_aflMidPassTexels[iTexel] = flMidHeight/flTotalMidHeight;
}

void CNormalGenerator::Setup()
{
	if (m_pGenerationParallelizer)
		delete m_pGenerationParallelizer;
	m_pGenerationParallelizer = new CParallelizer((JobCallback)::GeneratePass);

	generatepass_data_t oJob;
	oJob.pGenerator = this;

	for (size_t x = 0; x < m_iNormal2Width; x++)
	{
		for (size_t y = 0; y < m_iNormal2Height; y++)
		{
			oJob.x = x;
			oJob.y = y;
			m_pGenerationParallelizer->AddJob(&oJob, sizeof(oJob));
		}
	}

	m_pGenerationParallelizer->Start();
}

bool CNormalGenerator::IsSettingUp()
{
	if (!m_pGenerationParallelizer)
		return false;

	if (m_pGenerationParallelizer->AreAllJobsDone())
		return false;

	return true;
}

bool CNormalGenerator::IsSetupComplete()
{
	if (IsSettingUp())
		return false;

	if (!m_pGenerationParallelizer)
		return false;

	return m_pGenerationParallelizer->AreAllJobsDone();
}

float CNormalGenerator::GetSetupProgress()
{
	if (!m_pGenerationParallelizer)
		return 0;

	if (m_pGenerationParallelizer->GetJobsTotal() == 0)
		return 0;

	return (float)m_pGenerationParallelizer->GetJobsDone() / (float)m_pGenerationParallelizer->GetJobsTotal();
}

void CNormalGenerator::SetNormalTexture(bool bNormalTexture, size_t iMaterial)
{
	// Materials not loaded yet?
	if (!m_paoMaterials->size())
		return;

	CMaterial* pMaterial = &(*m_paoMaterials)[iMaterial];

	if (!pMaterial->m_iBase)
		return;

	m_iMaterial = iMaterial;

	if (m_iNormal2GLId)
		glDeleteTextures(1, &m_iNormal2GLId);
	m_iNormal2GLId = 0;

	// Don't let the listeners know yet, we want to generate the new one first so there is no lapse in displaying.
//	m_bNewNormal2Available = true;

	if (!bNormalTexture)
	{
		if (m_pNormal2Parallelizer)
		{
			delete m_pNormal2Parallelizer;
			m_pNormal2Parallelizer = NULL;
		}

		if (m_aflNormal2Texels)
		{
			delete[] m_aflTextureTexels;
			delete[] m_aflMidPassTexels;
			delete[] m_aflLowPassTexels;
			delete[] m_aflNormal2Texels;
		}
		m_aflTextureTexels = NULL;
		m_aflMidPassTexels = NULL;
		m_aflLowPassTexels = NULL;
		m_aflNormal2Texels = NULL;

		m_bNewNormal2Available = true;
		return;
	}

	glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);

	GLint iWidth, iHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight);

	if (!m_aflTextureTexels)
		m_aflTextureTexels = new float[iWidth*iHeight*3];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &m_aflTextureTexels[0]);

	m_iNormal2Width = iWidth;
	m_iNormal2Height = iHeight;

	if (!m_aflLowPassTexels)
		m_aflLowPassTexels = new float[iWidth*iHeight];
	if (!m_aflMidPassTexels)
		m_aflMidPassTexels = new float[iWidth*iHeight];

	if (!m_aflNormal2Texels)
		m_aflNormal2Texels = new float[iWidth*iHeight*3];

	Setup();

	UpdateNormal2();
}

void CNormalGenerator::UpdateNormal2()
{
	m_bNormal2Changed = true;
	m_bNormal2Generated = false;
}

void CNormalGenerator::StartGenerationJobs()
{
	m_bNormal2Changed = false;

	if (m_pNormal2Parallelizer && m_aflNormal2Texels)
	{
		m_pNormal2Parallelizer->RestartJobs();
		return;
	}

	if (m_pNormal2Parallelizer)
		delete m_pNormal2Parallelizer;
	m_pNormal2Parallelizer = new CParallelizer((JobCallback)::NormalizeHeightValue);

	normal2_data_t oJob;
	oJob.pGenerator = this;

	for (size_t x = 0; x < m_iNormal2Width; x++)
	{
		for (size_t y = 0; y < m_iNormal2Height; y++)
		{
			oJob.x = x;
			oJob.y = y;
			m_pNormal2Parallelizer->AddJob(&oJob, sizeof(oJob));
		}
	}

	m_pNormal2Parallelizer->Start();
}

void CNormalGenerator::RegenerateNormal2Texture()
{
	if (!m_aflNormal2Texels)
		return;

	if (m_iNormal2GLId)
		glDeleteTextures(1, &m_iNormal2GLId);

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iNormal2Width, (GLint)m_iNormal2Height, GL_RGB, GL_FLOAT, &m_aflNormal2Texels[0]);

	m_iNormal2GLId = iGLId;

	if (m_iNormal2ILId)
		ilDeleteImages(1, &m_iNormal2ILId);

	ILuint iILId;
	ilGenImages(1, &iILId);
	ilBindImage(iILId);
	ilTexImage((ILint)m_iNormal2Width, (ILint)m_iNormal2Height, 1, 3, IL_RGB, IL_FLOAT, &m_aflNormal2Texels[0]);
	ilConvertImage(IL_RGB, IL_UNSIGNED_INT);

	m_iNormal2ILId = iILId;

	m_bNewNormal2Available = true;
	m_bNormal2Generated = true;
}

bool CNormalGenerator::IsNewNormal2Available()
{
	if (m_pNormal2Parallelizer)
	{
		if (m_pNormal2Parallelizer->AreAllJobsDone() && !m_bNormal2Generated && !m_bNormal2Changed)
		{
			RegenerateNormal2Texture();

			m_bNewNormal2Available = true;
		}
	}

	return m_bNewNormal2Available;
}

bool CNormalGenerator::IsGeneratingNewNormal2()
{
	if (!m_pNormal2Parallelizer)
		return false;

	if (m_pNormal2Parallelizer->AreAllJobsDone())
		return false;

	return true;
}

float CNormalGenerator::GetNormal2GenerationProgress()
{
	if (!m_pNormal2Parallelizer)
		return 0;

	if (m_pNormal2Parallelizer->GetJobsTotal() == 0)
		return 0;

	return (float)m_pNormal2Parallelizer->GetJobsDone() / (float)m_pNormal2Parallelizer->GetJobsTotal();
}

void CNormalGenerator::GetNormalMap2(size_t& iNormal2, size_t& iNormal2IL)
{
	iNormal2 = m_iNormal2GLId;
	iNormal2IL = m_iNormal2ILId;
	m_iNormal2GLId = 0;
	m_iNormal2ILId = 0;
	m_bNewNormal2Available = false;
}
