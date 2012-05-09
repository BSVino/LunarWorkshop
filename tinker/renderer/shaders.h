#ifndef DT_SHADERS_H
#define DT_SHADERS_H

#include <tvector.h>
#include <EASTL/string.h>
#include <EASTL/map.h>

#include <tstring.h>
#include <vector.h>

class CShader
{
public:
							CShader(const tstring& sName, const tstring& sVertexFile, const tstring& sFragmentFile);

public:
	bool					Compile();
	void					Destroy();

public:
	eastl::string			m_sName;
	eastl::string			m_sVertexFile;
	eastl::string			m_sFragmentFile;
	size_t					m_iVShader;
	size_t					m_iFShader;
	size_t					m_iProgram;

	size_t					m_iPositionAttribute;
	size_t					m_iNormalAttribute;
	size_t					m_iTexCoordAttribute;
	size_t					m_iColorAttribute;

	class CParameter
	{
	public:
		tstring				m_sName;

		class CUniform
		{
		public:
			tstring			m_sName;
			union
			{
				float		m_flValue;
				int			m_iValue;
				bool		m_bValue;
			};
			Vector2D		m_vec2Value;
			Vector			m_vecValue;
			Vector4D		m_vec4Value;
			tstring			m_sValue;
			bool			m_bTexture;
		};

		tvector<CUniform>	m_aActions;

		tstring				m_sBlend;
	};

	eastl::map<tstring, CParameter>				m_aParameters;	// What the shader.txt has.
	eastl::map<tstring, tstring>				m_asUniforms;	// What the hardware has. Values are types.
	eastl::map<tstring, CParameter::CUniform>	m_aDefaults;	// Defaults for each uniform as specified by shader .txt (not GLSL)
	tvector<tstring>							m_asTextures;	// List of textures for purposes of assigning to channels and whatnot.
};

class CShaderLibrary
{
	friend class CShader;

public:
							CShaderLibrary();
							~CShaderLibrary();

public:
	static size_t			GetNumShaders() { return Get()->m_aShaders.size(); };

	static CShader*			GetShader(const tstring& sName);
	static CShader*			GetShader(size_t i);

	static void				AddShader(const tstring& sFile);

	static size_t			GetProgram(const tstring& sName);

	static void				CompileShaders(int iSamples = -1);
	static void				DestroyShaders();

	static bool				IsCompiled() { return Get()->m_bCompiled; };

	static tstring			GetShaderHeader() { return Get()->m_sHeader; }
	static tstring			GetShaderFunctions() { return Get()->m_sFunctions; }

	static CShaderLibrary*	Get() { return s_pShaderLibrary; };

protected:
	void					ClearLog();
	void					WriteLog(const char* pszLog, const char* pszShaderText);

protected:
	eastl::map<tstring, size_t>	m_aShaderNames;
	tvector<CShader>		m_aShaders;
	bool					m_bCompiled;

	bool					m_bLogNeedsClearing;

	int						m_iSamples;

	tstring					m_sHeader;
	tstring					m_sFunctions;

private:
	static CShaderLibrary*	s_pShaderLibrary;
};

#endif
