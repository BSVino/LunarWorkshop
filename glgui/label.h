#ifndef TINKER_LABEL_H
#define TINKER_LABEL_H

#include "basecontrol.h"

#include <tmap.h>

class FTFont;

namespace glgui
{
	class CLabel : public CBaseControl
	{
		friend class CRootPanel;

	public:
		class CLineSection
		{
		public:
			tstring		m_sText;
			tstring		m_sLink;
			FRect		m_rArea;
			tstring		m_sFont;
			size_t		m_iFontSize;
		};

		class CLine
		{
		public:
			tvector<CLineSection>	m_aSections;
			float		m_flLineHeight;
			float		m_flLineWidth;
		};

	public:
						CLabel();
						CLabel(const tstring& sText, const tstring& sFont="sans-serif", size_t iSize=13);
						CLabel(float x, float y, float w, float h, const tstring& sText, const tstring& sFont="sans-serif", size_t iSize=13);

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
		virtual void	DrawSection(const CLine& l, const CLineSection& s, float x, float y, float w, float h);
		virtual void	Layout() {};
		virtual void	Think() {};

		virtual void	GetAlignmentOffset(float flLineWidth, float flLineHeight, const tstring& sFont, size_t iFontSize, float flAreaWidth, float flAreaHeight, float& x, float& y) const;

		virtual void	SetWidth(float w);
		virtual void	SetHeight(float h);
		virtual void	SetSize(float w, float h);

		void			GetRealMousePosition(float& x, float& y);
		bool			MouseIsInside(const CLine& oLine, const CLineSection& oSection);

		virtual bool	IsCursorListener() { return true; };
		virtual bool	MousePressed(int code, int mx, int my);
		virtual bool	MouseReleased(int code, int mx, int my);

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
		virtual void	EnsureTextFits();

		virtual void	ComputeLines(float w = -1, float h = -1);
		virtual void	PushSection(const CLineSection& oSection, const tstring& sText);

		virtual size_t				GetNumLines() const { return m_aLines.size(); }
		virtual const CLine&		GetLine(size_t iLine) const { return m_aLines[iLine]; }
		virtual size_t				GetNumSections(size_t iLine) const { return m_aLines[iLine].m_aSections.size(); }
		virtual const CLineSection&	GetSection(size_t iLine, size_t iSection) const { return m_aLines[iLine].m_aSections[iSection]; }

		virtual Color	GetTextColor();
		virtual void	SetTextColor(const Color& clrText);
		virtual void	SetAlpha(int a);
		virtual void	SetAlpha(float a);

		virtual void	SetScissor(bool bScissor); // 61

		virtual void	Set3D(bool b3D) { m_b3D = b3D; }
		virtual bool	Is3D() const { return m_b3D; }
		virtual void	Set3DMousePosition(const Vector& vecMouse) { m_vec3DMouse = vecMouse; }

		virtual void	SetLinkClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual IEventListener::Callback	GetLinkClickedListenerCallback() { return m_pfnLinkClickCallback; };
		virtual IEventListener*				GetLinkClickedListener() { return m_pLinkClickListener; };

		virtual void	SetSectionHoverListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual IEventListener::Callback	GetSectionHoverListenerCallback() { return m_pfnSectionHoverCallback; };
		virtual IEventListener*				GetSectionHoverListener() { return m_pSectionHoverListener; };

		static class ::FTFont*	GetFont(const tstring& sName, size_t iSize);
		static void		AddFont(const tstring& sName, const tstring& sFile);
		static void		AddFontSize(const tstring& sName, size_t iSize);

		static float	GetTextWidth(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize);
		static float	GetFontHeight(const tstring& sFontName, int iFontFaceSize);
		static void		PaintText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, float x, float y, const Color& clrText = Color(255, 255, 255), const FRect& rStencil = FRect(-1, -1, -1, -1));
		static void		PaintText3D(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, Vector vecPosition, const Color& clrText = Color(255, 255, 255));

	protected:
		bool			m_bEnabled;
		bool			m_bWrap;
		bool			m_b3D;
		Vector			m_vec3DMouse;
		tstring			m_sText;
		Color			m_clrText;
		bool			m_bScissor;

		TextAlign		m_eAlign;

		tvector<CLine>	m_aLines;
		float			m_flTotalHeight;
		bool			m_bNeedsCompute;

		int				m_iPrintChars;
		int				m_iCharsDrawn;

		tstring			m_sFontName;
		int				m_iFontFaceSize;

		static tmap<tstring, tmap<size_t, class ::FTFont*> >	s_apFonts;
		static tmap<tstring, tstring>							s_apFontNames;

		IEventListener::Callback m_pfnLinkClickCallback;
		IEventListener*	m_pLinkClickListener;

		IEventListener::Callback m_pfnSectionHoverCallback;
		IEventListener*	m_pSectionHoverListener;
	};
};

#endif
