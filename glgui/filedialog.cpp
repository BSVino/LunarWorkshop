#include "filedialog.h"

#include <tinker_platform.h>

#include <tinker/shell.h>

#include "label.h"
#include "textfield.h"
#include "button.h"
#include "tree.h"
#include "rootpanel.h"

using namespace glgui;

CResource<CBaseControl> CFileDialog::s_pDialog;

CFileDialog::CFileDialog(const tstring& sDirectory, const tstring& sExtension, bool bSave)
	: CMovablePanel(bSave?"Save File...":"Open File...")
{
	m_sDirectory = FindAbsolutePath(sDirectory.length()?sDirectory:".");
	m_bSave = bSave;

	SetBackgroundColor(g_clrBox/2);
	SetBorder(BT_SOME);

	strtok(sExtension, m_asExtensions, ";");
}

void CFileDialog::CreateControls(CResource<CBaseControl> pThis)
{
	m_hDirectoryLabel = AddControl(CreateControl(new CLabel(5, 5, 1, 1, "Folder:")));
	m_hDirectory = AddControl(CreateControl(new CTextField()));
	m_hDirectory->SetContentsChangedListener(this, NewDirectory);
	m_hDirectory->SetText(m_sDirectory);
	m_hOpenInExplorer = AddControl(CreateControl(new CButton(0, 0, 20, 20, "Open in explorer", false, "sans-serif", 8)));
	m_hOpenInExplorer->SetClickedListener(this, Explore);

	m_hFilesLabel = AddControl(CreateControl(new CLabel(5, 5, 1, 1, "Files:")));
	m_hFileList = AddControl(CreateControl(new CTree()));
	m_hFileList->SetSelectedListener(this, FileSelected);
	m_hFileList->SetConfirmedListener(this, FileConfirmed);
	m_hFileList->SetBackgroundColor(g_clrBox);

	m_hNewFile = AddControl(CreateControl(new CTextField()));
	m_hNewFile->SetContentsChangedListener(this, NewFileChanged);
	m_hNewFile->SetVisible(m_bSave);
	m_hSelect = AddControl(CreateControl(new CButton(0, 0, 20, 20, m_bSave?"Save":"Open")));
	m_hSelect->SetClickedListener(this, Select);
	m_hSelect->SetEnabled(false);
	m_hCancel = AddControl(CreateControl(new CButton(0, 0, 20, 20, "Cancel")));
	m_hCancel->SetClickedListener(this, Close);

	BaseClass::CreateControls(pThis);
}

CFileDialog::~CFileDialog()
{
}

void CFileDialog::Layout()
{
	m_hDirectory->SetText(m_sDirectory);

	SetSize(400, 300);
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_hDirectoryLabel->EnsureTextFits();
	m_hFilesLabel->EnsureTextFits();

	m_hDirectoryLabel->SetPos(5, 15);
	m_hDirectory->SetPos(5, m_hDirectoryLabel->GetBottom());
	m_hDirectory->SetSize(GetWidth()-55, m_hDirectory->GetHeight());

	m_hOpenInExplorer->SetSize(40, m_hDirectory->GetHeight());
	m_hOpenInExplorer->SetPos(GetWidth()-45, m_hDirectoryLabel->GetBottom());

	m_hFilesLabel->SetPos(5, m_hDirectory->GetBottom()+5);
	m_hFileList->SetPos(5, m_hFilesLabel->GetBottom());
	m_hFileList->SetSize(200, 170);

	m_hFileList->ClearTree();

	tvector<tstring> asFiles = ListDirectory(m_sDirectory, true);

	m_hFileList->AddNode("..");
	for (size_t i = 0; i < asFiles.size(); i++)
	{
		tstring sFile = asFiles[i];

		if (IsDirectory(m_sDirectory + DIR_SEP + sFile))
		{
			m_hFileList->AddNode(sFile + DIR_SEP);
			continue;
		}

		for (size_t j = 0; j < m_asExtensions.size(); j++)
		{
			if (!sFile.endswith(m_asExtensions[j]))
				continue;

			m_hFileList->AddNode(sFile);
			break;
		}
	}

	m_hSelect->SetSize(60, m_hNewFile->GetHeight());
	m_hSelect->SetPos(GetWidth() - 130, GetHeight() - m_hNewFile->GetHeight() - 5);
	m_hSelect->SetEnabled(false);

	m_hCancel->SetSize(60, m_hNewFile->GetHeight());
	m_hCancel->SetPos(GetWidth() - 65, GetHeight() - m_hNewFile->GetHeight() - 5);

	m_hNewFile->SetPos(5, GetHeight() - m_hNewFile->GetHeight() - 5);
	m_hNewFile->SetSize(GetWidth() - 140, m_hNewFile->GetHeight());

	BaseClass::Layout();
}

void CFileDialog::NewDirectoryCallback(const tstring& sArgs)
{
	m_sDirectory = m_hDirectory->GetText();
	Layout();

	m_hDirectory->SetAutoCompleteFiles("", m_asExtensions);

	if (IsFile(m_sDirectory))
	{
		for (size_t i = 0; i < m_asExtensions.size(); i++)
		{
			if (m_sDirectory.endswith(m_asExtensions[i]))
			{
				FileConfirmed(m_sDirectory);
				return;
			}
		}
	}
}

void CFileDialog::ExploreCallback(const tstring& sArgs)
{
	OpenExplorer(m_sDirectory);
}

void CFileDialog::FileSelectedCallback(const tstring& sArgs)
{
	m_hNewFile->SetText("");

	m_hSelect->SetEnabled(!!m_hFileList->GetSelectedNode());
}

void CFileDialog::NewFileChangedCallback(const tstring& sArgs)
{
	m_hFileList->Unselect();

	m_hSelect->SetEnabled(m_hNewFile->GetText().length() > 0);
}

void CFileDialog::SelectCallback(const tstring& sArgs)
{
	tstring sFile = GetFile();
	if (sFile.find(DIR_SEP"..") == sFile.length()-3)
	{
		m_sDirectory = FindAbsolutePath(sFile);
		Layout();
		return;
	}

	if (IsDirectory(sFile))
	{
		m_sDirectory = FindAbsolutePath(sFile);
		Layout();
		return;
	}

	FileConfirmed(sFile);
}

void CFileDialog::CloseCallback(const tstring& sArgs)
{
	SetVisible(false);
}

void CFileDialog::FileConfirmedCallback(const tstring& sArgs)
{
	FileSelectedCallback(sArgs);
	SelectCallback(sArgs);
}

void CFileDialog::FileConfirmed(const tstring& sFile)
{
	SetVisible(false);

	if (m_pSelectListener && m_pfnSelectCallback)
		m_pfnSelectCallback(m_pSelectListener, sFile);
}

void CFileDialog::ShowOpenDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	if (s_pDialog.get())
		s_pDialog.DowncastStatic<CFileDialog>()->Close();

	s_pDialog = CreateControl(new CFileDialog(sDirectory, sExtension, false));
	s_pDialog->Layout();

	CFileDialog* pDialog = s_pDialog.DowncastStatic<CFileDialog>();
	pDialog->m_pSelectListener = pListener;
	pDialog->m_pfnSelectCallback = pfnCallback;
}

void CFileDialog::ShowSaveDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	if (s_pDialog.get())
		s_pDialog.DowncastStatic<CFileDialog>()->Close();

	s_pDialog = CreateControl(new CFileDialog(sDirectory, sExtension, true));
	s_pDialog->Layout();

	CFileDialog* pDialog = s_pDialog.DowncastStatic<CFileDialog>();
	pDialog->m_pSelectListener = pListener;
	pDialog->m_pfnSelectCallback = pfnCallback;
}

tstring CFileDialog::GetFile()
{
	CFileDialog* pDialog = s_pDialog.DowncastStatic<CFileDialog>();

	if (!pDialog)
		return "";

	if (pDialog->m_bSave && pDialog->m_hNewFile->GetText().length())
	{
		tstring sName = pDialog->m_hNewFile->GetText();

		for (size_t j = 0; j < pDialog->m_asExtensions.size(); j++)
		{
			if (sName.length() <= pDialog->m_asExtensions[j].length())
				return FindAbsolutePath(pDialog->m_sDirectory + DIR_SEP + pDialog->m_hNewFile->GetText() + pDialog->m_asExtensions[j]);

			tstring sNameExtension = sName.substr(sName.length() - pDialog->m_asExtensions[j].length());
			if (sNameExtension == pDialog->m_asExtensions[j])
				return FindAbsolutePath(pDialog->m_sDirectory + DIR_SEP + pDialog->m_hNewFile->GetText());
		}

		return FindAbsolutePath(pDialog->m_sDirectory + DIR_SEP + pDialog->m_hNewFile->GetText() + pDialog->m_asExtensions[0]);
	}

	if (!pDialog->m_hFileList->GetSelectedNode())
		return "";

	return FindAbsolutePath(pDialog->m_sDirectory + DIR_SEP + pDialog->m_hFileList->GetSelectedNode()->m_hLabel->GetText());
}
