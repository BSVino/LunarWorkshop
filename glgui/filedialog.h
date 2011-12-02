#ifndef LW_GLGUI_FILEDIALOG_H
#define LW_GLGUI_FILEDIALOG_H

#include "panel.h"

namespace glgui
{
	class CFileDialog : public CPanel, public IEventListener
	{
		DECLARE_CLASS(CFileDialog, CPanel);

	public:
									CFileDialog(const tstring& sDirectory, const tstring& sExtension, bool bSave);
		virtual						~CFileDialog();

	public:
		virtual void				Layout();
		virtual void				Paint(float x, float y, float w, float h);

		EVENT_CALLBACK(CFileDialog, NewDirectory);
		EVENT_CALLBACK(CFileDialog, Explore);
		EVENT_CALLBACK(CFileDialog, FileSelected);
		EVENT_CALLBACK(CFileDialog, NewFileChanged);
		EVENT_CALLBACK(CFileDialog, Select);
		EVENT_CALLBACK(CFileDialog, Close);

		static void					ShowOpenDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static void					ShowSaveDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static tstring				GetFile();

	protected:
		CLabel*						m_pDirectoryLabel;
		CTextField*					m_pDirectory;
		CButton*					m_pOpenInExplorer;

		CLabel*						m_pFilesLabel;
		CTree*						m_pFileList;

		CTextField*					m_pNewFile;
		CButton*					m_pSelect;
		CButton*					m_pCancel;

		tstring						m_sDirectory;
		eastl::vector<tstring>		m_asExtensions;
		bool						m_bSave;

		IEventListener*				m_pSelectListener;
		IEventListener::Callback	m_pfnSelectCallback;

		static CFileDialog*			s_pDialog;
	};
};

#endif
