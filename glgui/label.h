#ifndef TINKER_LABEL_H
#define TINKER_LABEL_H

#include "basecontrol.h"

#include <EASTL/map.h>

class FTFont;

namespace glgui
{
	class CLabel : public CBaseControl
	{
		friend class CRootPanel;

	protected:
		class CLineSection
		{
		public:
			tstring		m_sText;
			FRect		m_rArea;
			tstring		m_sFont;
			size_t		m_iFontSize;
			float		m_flStart;	// X coordinate of the start of this section
		};

		class CLine
		{
		public:
			eastl::vector<CLineSection>	m_aSections;
			float		m_flLineHeight;
			float		m_flLineWidth;
		};

	public:
						CLabel();
						CLabel(float x, float y, float w, float h, const tstring& sText, const tstring& sFont=_T("sans-serif"), size_t iSize=13);

	public:
		typedef enum
		{
			TA_TOPLEFT		= 0,
			TA_TOPCENTER	= 1,
			TA_LEFTCENTER	= 2,
			TA_MIDDLECENTER	= 3,
			TA_RIGHTCENTER	= 4,
			TA_BOTTOMCENTER	= 5,
			TA_BOTTOMLEFT	= 6,
		} TextAlign;

		virtual void	Paint() { float x = 0, y = 0; GetAbsPos(x, y); Paint(x, y); };
		virtual void	Paint(float x, float y) { Paint(x, y, m_flW, m_flH); };
		virtual void	Paint(float x, float y, float w, float h);
		virtual void	DrawSection(const CLine& l, const CLineSection& s, float x, float y, float w, float h, float flLineHeight);
		virtual void	Layout() {};
		virtual void	Think() {};

		virtual void	SetSize(float w, float h);

		virtual bool	MousePressed(int code, int mx, int my) {return false;};
		virtual bool	MouseReleased(int code, int mx, int my) {return false;};

		virtual bool	IsEnabled() {return m_bEnabled;};
		virtual void	SetEnabled(bool bEnabled) {m_bEnabled = bEnabled;};

		virtual TextAlign	GetAlign() { return m_eAlign; };
		virtual void	SetAlign(TextAlign eAlign) { m_eAlign = eAlign; };

		virtual bool	GetWrap() { return m_bWrap; };
		virtual void	SetWrap(bool bWrap) { m_bWrap = bWrap; m_bNeedsCompute = true; };

		virtual void	SetPrintChars(int iPrintChars) { m_iPrintChars = iPrintChars; }

		virtual void	SetText(const tstring& sText);
		virtual void	AppendText(const tstring& sText);
		virtual tstring	GetText();

		virtual void	SetFont(const tstring& sFontName, int iSize=13);
		virtual int		GetFontFaceSize() { return m_iFontFaceSize; };

		virtual float	GetTextWidth();
		virtual float	GetTextHeight();
		virtual void	ComputeLines(float w = -1, float h = -1);
		virtual void	EnsureTextFits();

		virtual Color	GetFGColor();
		virtual void	SetFGColor(Color FGColor);
		virtual void	SetAlpha(int a);
		virtual void	SetAlpha(float a);

		virtual void	SetScissor(bool bScissor); // 61

		virtual void	Set3D(bool b3D) { m_b3D = b3D; }
		virtual bool	Get3D() const { return m_b3D; }

		static class ::FTFont*	GetFont(const tstring& sName, size_t iSize);
		static void		AddFont(const tstring& sName, const tstring& sFile);
		static void		AddFontSize(const tstring& sName, size_t iSize);

		static float	GetTextWidth(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize);
		static float	GetFontHeight(const tstring& sFontName, int iFontFaceSize);
		static void		PaintText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, float x, float y);
		static void		PaintText3D(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, Vector vecPosition);

	protected:
		bool			m_bEnabled;
		bool			m_bWrap;
		bool			m_b3D;
		tstring			m_sText;
		Color			m_FGColor;
		bool			m_bScissor;

		TextAlign		m_eAlign;

		eastl::vector<CLine>	m_aLines;
		float			m_flTotalHeight;
		bool			m_bNeedsCompute;

		int				m_iPrintChars;
		int				m_iCharsDrawn;

		tstring			m_sFontName;
		int				m_iFontFaceSize;

		static eastl::map<tstring, eastl::map<size_t, class ::FTFont*> >	s_apFonts;
		static eastl::map<tstring, tstring>									s_apFontNames;
	};
};

#endif
