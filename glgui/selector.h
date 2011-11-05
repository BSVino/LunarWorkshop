#ifndef TINKER_SELECTOR_H
#define TINKER_SELECTOR_H

#include "panel.h"

namespace glgui
{
	template <typename T>
	class CScrollSelection
	{
	public:
		CScrollSelection(T oParam, const tstring& sLabel)
		{
			m_oParam = oParam;
			m_sLabel = sLabel;
		}

		T									m_oParam;
		tstring						m_sLabel;
	};

	template <typename T>
	class CScrollSelector : public CPanel
	{
	public:
		CScrollSelector(const tstring& sFont=_T("sans-serif"), size_t iSize=13)
			: CPanel(0, 0, 100, 16)
		{
			m_flHandlePositionGoal = 0;
			m_flHandlePosition = 0;
			m_bMovingHandle = false;

			m_iSelection = 0;

			m_pfnSelectedCallback = NULL;
			m_pSelectedListener = NULL;

			m_pOption = new CLabel(0, 0, 100, 100, _T(""));
			m_pOption->SetWrap(false);
			m_pOption->SetFont(sFont, iSize);
			AddControl(m_pOption);
		}

		virtual void Layout()
		{
			m_pOption->SetSize(1, 1);

			// Make sure there's some text to be fit to.
			if (m_pOption->GetText() == _T(""))
				m_pOption->SetText(_T("-"));

			m_pOption->EnsureTextFits();
			m_pOption->SetPos(GetWidth()/2 - m_pOption->GetWidth()/2, GetHeight()-15);

			CPanel::Layout();
		}

		virtual void Think()
		{
			if (m_bMovingHandle)
			{
				int mx, my;
				CRootPanel::GetFullscreenMousePos(mx, my);

				int x, y, w, h;
				GetAbsDimensions(x, y, w, h);

				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x, (float)(x + w), 0.0f, 1.0f);
			}
			else
			{
				if (m_aSelections.size() < 2)
					m_flHandlePositionGoal = ((float)GetWidth()*(float)m_iSelection)/GetWidth();
				else
					m_flHandlePositionGoal = ((float)GetWidth()/((float)m_aSelections.size()-1)*(float)m_iSelection)/GetWidth();
			}

			m_flHandlePosition = Approach(m_flHandlePositionGoal, m_flHandlePosition, CRootPanel::Get()->GetFrameTime()*10);

			int iSelection = SelectionByHandle();
			m_pOption->SetText(m_aSelections[iSelection].m_sLabel.c_str());

			if (iSelection != m_iSelection)
			{
				m_iSelection = iSelection;
				if (m_pSelectedListener)
					m_pfnSelectedCallback(m_pSelectedListener);
			}
		}

#define HANDLE_SIZE 12

		virtual void Paint(int x, int y, int w, int h)
		{
			//CRootPanel::PaintRect(x, y, w, h, g_clrBoxHi);

			int iLeft = x+HANDLE_SIZE/2;
			int iWidth = w-HANDLE_SIZE;

			CRootPanel::PaintRect(iLeft, y+h/2, iWidth, 1, Color(200, 200, 200, 255));

			if (m_aSelections.size() < 2)
			{
				CRootPanel::PaintRect(iLeft, y+h/2-5, 1, 10, Color(200, 200, 200, 255));
				CRootPanel::PaintRect(iLeft + iWidth, y+h/2-5, 1, 10, Color(200, 200, 200, 255));
			}
			else
			{
				for (size_t i = 0; i < m_aSelections.size(); i++)
					CRootPanel::PaintRect(iLeft + iWidth*(int)i/((int)m_aSelections.size()-1), y+h/2-5, 1, 10, Color(200, 200, 200, 255));
			}

			CRootPanel::PaintRect(HandleX()+2, HandleY()+2, HANDLE_SIZE-4, HANDLE_SIZE-4, g_clrBoxHi);

			CPanel::Paint(x, y, w, h);
		}

		virtual bool MousePressed(int code, int mx, int my)
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			int hx, hy;
			hx = HandleX();
			hy = HandleY();

			if (mx >= hx && mx < hx + HANDLE_SIZE && my >= hy && my < hy + HANDLE_SIZE)
				m_bMovingHandle = true;
			else
			{
				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x + HANDLE_SIZE/2, (float)(x + w - HANDLE_SIZE/2), 0.0f, 1.0f);
				m_iSelection = SelectionByHandle();

				if (m_pSelectedListener)
					m_pfnSelectedCallback(m_pSelectedListener);
			}

			return true;
		}

		virtual bool MouseReleased(int code, int mx, int my)
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			if (m_bMovingHandle)
			{
				DoneMovingHandle();
				return true;
			}

			return CPanel::MouseReleased(code, mx, my);
		}

		virtual void CursorOut()
		{
			if (m_bMovingHandle)
			{
				int mx, my;
				CRootPanel::GetFullscreenMousePos(mx, my);

				int x, y, w, h;
				GetAbsDimensions(x, y, w, h);

				// If the mouse went out of the left or right side, make sure we're all the way to that side.
				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x, (float)(x + w), 0.0f, 1.0f);

				DoneMovingHandle();
			}
		}

		virtual void DoneMovingHandle()
		{
			m_bMovingHandle = false;

			m_iSelection = SelectionByHandle();

			if (m_pSelectedListener)
				m_pfnSelectedCallback(m_pSelectedListener);
		}

		virtual void AddSelection(const CScrollSelection<T>& oSelection)
		{
			m_aSelections.push_back(oSelection);
		}

		virtual size_t GetNumSelections()
		{
			return m_aSelections.size();
		}
		
		virtual void RemoveAllSelections()
		{
			m_aSelections.clear();
		}

		virtual void SetSelection(size_t i)
		{
			if (i >= m_aSelections.size())
				i = m_aSelections.size() - 1;

			if (i == m_iSelection)
				return;

			m_iSelection = i;
			m_flHandlePositionGoal = m_flHandlePosition = ((float)GetWidth()/((float)m_aSelections.size()-1)*(float)m_iSelection)/GetWidth();

			if (m_pSelectedListener)
				m_pfnSelectedCallback(m_pSelectedListener);
		}

		virtual T GetSelectionValue()
		{
			return m_aSelections[m_iSelection].m_oParam;
		}

		virtual size_t FindClosestSelectionValue(float flValue)
		{
			size_t iClosest;
			T flClosestValue;
			for (size_t i = 0; i < m_aSelections.size(); i++)
			{
				if (i == 0)
				{
					flClosestValue = m_aSelections[0].m_oParam;
					iClosest = 0;
					continue;
				}

				if (fabs((float)(m_aSelections[i].m_oParam - flValue)) < fabs((float)(flClosestValue - flValue)))
				{
					flClosestValue = m_aSelections[i].m_oParam;
					iClosest = i;
				}
			}

			return iClosest;
		}

		virtual int SelectionByHandle()
		{
			int iSelection = (int)(m_flHandlePositionGoal*m_aSelections.size());

			if (iSelection < 0)
				return 0;

			if (iSelection >= (int)m_aSelections.size())
				return (int)m_aSelections.size()-1;

			return iSelection;
		}

		virtual int HandleX()
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			int iLeft = x+HANDLE_SIZE/2;
			int iWidth = w-HANDLE_SIZE;
			return iLeft + (int)(iWidth*m_flHandlePosition) - HANDLE_SIZE/2;
		}

		virtual int HandleY()
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			return y+h/2-HANDLE_SIZE/2;
		}

		void SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
		{
			m_pfnSelectedCallback = pfnCallback;
			m_pSelectedListener = pListener;
		}

	protected:
		eastl::vector<CScrollSelection<T> >	m_aSelections;

		CLabel*								m_pOption;

		size_t								m_iSelection;

		float								m_flHandlePosition;
		float								m_flHandlePositionGoal;

		bool								m_bMovingHandle;

		IEventListener::Callback			m_pfnSelectedCallback;
		IEventListener*						m_pSelectedListener;
	};
};

#endif
