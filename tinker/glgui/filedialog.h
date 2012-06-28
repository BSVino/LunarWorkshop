#ifndef LW_GLGUI_FILEDIALOG_H
#define LW_GLGUI_FILEDIALOG_H

#include "movablepanel.h"

namespace glgui
{
	class CFileDialog : public CMovablePanel
	{
		DECLARE_CLASS(CFileDialog, CMovablePanel);

		using CMovablePanel::Close;

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
		EVENT_CALLBACK(CFileDialog, FileType);

		void						FileConfirmed(const tstring& sFile);

		static void					ShowOpenDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static void					ShowSaveDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static tstring				GetFile();
		static void					SetDefaultDirectory(const tstring sDirectory) { s_sDirectory = sDirectory; }

	protected:
		CControl<CLabel>			m_hDirectoryLabel;
		CControl<CTextField>		m_hDirectory;
		CControl<CButton>			m_hOpenInExplorer;

		CControl<CLabel>			m_hFilesLabel;
		CControl<CTree>				m_hFileList;

		CControl<CTextField>		m_hNewFile;
		CControl<CButton>			m_hSelect;
		CControl<CButton>			m_hCancel;

		CControl<CMenu>				m_hFileTypes;

		tstring						m_sDirectory;
		tvector<tstring>			m_asExtensions;
		size_t						m_iCurrentExtension;	// (m_iCurrentExtension - 1) is an index into m_asExtensions. 0 means all.
		bool						m_bSave;

		IEventListener*				m_pSelectListener;
		IEventListener::Callback	m_pfnSelectCallback;

		static CControl<CFileDialog>	s_hDialog;
		static tstring				s_sDirectory;
	};
};

#endif
