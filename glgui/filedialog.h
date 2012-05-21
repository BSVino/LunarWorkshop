#ifndef LW_GLGUI_FILEDIALOG_H
#define LW_GLGUI_FILEDIALOG_H

#include "movablepanel.h"

namespace glgui
{
	class CFileDialog : public CMovablePanel
	{
		DECLARE_CLASS(CFileDialog, CMovablePanel);

	public:
									CFileDialog(const tstring& sDirectory, const tstring& sExtension, bool bSave);
		virtual						~CFileDialog();

	public:
		virtual void				Layout();

		EVENT_CALLBACK(CFileDialog, NewDirectory);
		EVENT_CALLBACK(CFileDialog, Explore);
		EVENT_CALLBACK(CFileDialog, FileSelected);
		EVENT_CALLBACK(CFileDialog, NewFileChanged);
		EVENT_CALLBACK(CFileDialog, Select);
		EVENT_CALLBACK(CFileDialog, Close);
		EVENT_CALLBACK(CFileDialog, FileConfirmed);

		void						FileConfirmed(const tstring& sFile);

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
		tvector<tstring>			m_asExtensions;
		bool						m_bSave;

		IEventListener*				m_pSelectListener;
		IEventListener::Callback	m_pfnSelectCallback;

		static CFileDialog*			s_pDialog;
	};
};

#endif
