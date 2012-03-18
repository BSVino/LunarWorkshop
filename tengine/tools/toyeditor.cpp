#include "toyeditor.h"

#include <iostream>
#include <fstream>

#include <tinker_platform.h>
#include <files.h>

#include <glgui/movablepanel.h>
#include <glgui/textfield.h>
#include <glgui/button.h>
#include <glgui/menu.h>
#include <tinker/application.h>

#include "workbench.h"

REGISTER_WORKBENCH_TOOL(ToyEditor);

CCreateToySourcePanel::CCreateToySourcePanel()
	: glgui::CMovablePanel("Create Toy Source Tool")
{
	SetBackgroundColor(Color(0, 0, 0, 255));
	SetHeaderColor(Color(100, 100, 100, 255));
	SetBorder(glgui::CPanel::BT_SOME);

	m_pToyFileLabel = new glgui::CLabel("Toy File:", "sans-serif", 10);
	m_pToyFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pToyFileLabel);
	m_pToyFileText = new glgui::CTextField();
	m_pToyFileText->SetContentsChangedListener(this, ToyChanged);
	AddControl(m_pToyFileText);

	m_pSourceFileLabel = new glgui::CLabel("Source File:", "sans-serif", 10);
	m_pSourceFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pSourceFileLabel);
	m_pSourceFileText = new glgui::CTextField();
	m_pSourceFileText->SetContentsChangedListener(this, SourceChanged);
	AddControl(m_pSourceFileText);

	m_pWarnings = new glgui::CLabel("");
	m_pWarnings->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pWarnings);

	m_pCreate = new glgui::CButton("Create");
	m_pCreate->SetClickedListener(this, Create);
	AddControl(m_pCreate);
}

void CCreateToySourcePanel::Layout()
{
	float flTop = 20;
	m_pToyFileLabel->SetLeft(15);
	m_pToyFileLabel->SetTop(flTop);
	m_pToyFileText->SetWidth(GetWidth()-30);
	m_pToyFileText->CenterX();
	m_pToyFileText->SetTop(flTop+12);

	flTop += 43;

	m_pSourceFileLabel->SetLeft(15);
	m_pSourceFileLabel->SetTop(flTop);
	m_pSourceFileText->SetWidth(GetWidth()-30);
	m_pSourceFileText->CenterX();
	m_pSourceFileText->SetTop(flTop+12);

	flTop += 43;

	m_pWarnings->SetLeft(15);
	m_pWarnings->SetTop(flTop);
	m_pWarnings->SetWidth(GetWidth()-30);
	m_pWarnings->CenterX();
	m_pWarnings->SetWrap(true);
	m_pWarnings->SetBottom(GetBottom() - 60);

	m_pCreate->SetTop(GetHeight() - 45);
	m_pCreate->CenterX();

	BaseClass::Layout();

	FileNamesChanged();
}

void CCreateToySourcePanel::ToyChangedCallback(const tstring& sArgs)
{
	FileNamesChanged();

	if (!m_pToyFileText->GetText().length())
		return;

	tstring sGameFolder = FindAbsolutePath(".");
	tstring sInputFolder = FindAbsolutePath(m_pToyFileText->GetText());

	if (sInputFolder.compare(0, sGameFolder.length(), sGameFolder) != 0)
		return;

	tstring sSearchDirectory = GetDirectory(sInputFolder);

	tstring sPrefix = ToForwardSlashes(sSearchDirectory.substr(sGameFolder.length()));
	while (sPrefix[0] == '/')
		sPrefix = sPrefix.substr(1);
	while (sPrefix.back() == '/')
		sPrefix = sPrefix.substr(0, sPrefix.length()-2);
	if (sPrefix.length())
		sPrefix = sPrefix + '/';

	eastl::vector<tstring> asFiles = ListDirectory(sSearchDirectory);
	eastl::vector<tstring> asCompletions;

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		if (!IsDirectory(sPrefix + asFiles[i]))
		{
			if (asFiles[i].length() <= 4)
				continue;

			if (asFiles[i].substr(asFiles[i].length()-4) != ".toy")
				continue;

			if (asFiles[i].length() > 9)
			{
				if (asFiles[i].substr(asFiles[i].length()-9) == ".mesh.toy")
					continue;

				if (asFiles[i].substr(asFiles[i].length()-9) == ".phys.toy")
					continue;

				if (asFiles[i].substr(asFiles[i].length()-9) == ".area.toy")
					continue;
			}
		}

		asCompletions.push_back(sPrefix + asFiles[i]);
	}

	m_pToyFileText->SetAutoCompleteCommands(asCompletions);
}

void CCreateToySourcePanel::SourceChangedCallback(const tstring& sArgs)
{
	FileNamesChanged();

	if (!m_pSourceFileText->GetText().length())
		return;

	tstring sSourceFolder = FindAbsolutePath("../sources");
	tstring sInputFolder = FindAbsolutePath("../sources/" + m_pSourceFileText->GetText());

	if (sInputFolder.compare(0, sSourceFolder.length(), sSourceFolder) != 0)
		return;

	tstring sSearchDirectory = GetDirectory(sInputFolder);

	tstring sPrefix = ToForwardSlashes(sSearchDirectory.substr(sSourceFolder.length()));
	while (sPrefix[0] == '/')
		sPrefix = sPrefix.substr(1);
	while (sPrefix.back() == '/')
		sPrefix = sPrefix.substr(0, sPrefix.length()-2);
	if (sPrefix.length())
		sPrefix = sPrefix + '/';

	eastl::vector<tstring> asFiles = ListDirectory(sSearchDirectory);
	eastl::vector<tstring> asCompletions;

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		if (!IsDirectory(sSearchDirectory + '/' + asFiles[i]))
		{
			if (asFiles[i].length() <= 4)
				continue;

			if (asFiles[i].substr(asFiles[i].length()-4) != ".txt")
				continue;
		}

		asCompletions.push_back(sPrefix + asFiles[i]);
	}

	m_pSourceFileText->SetAutoCompleteCommands(asCompletions);
}

tstring CCreateToySourcePanel::GetToyFileName()
{
	tstring sToyFile = m_pToyFileText->GetText();

	if (sToyFile.length() <= 4 || sToyFile.substr(sToyFile.length()-4) != ".toy")
		sToyFile.append(".toy");

	return sToyFile;
}

tstring CCreateToySourcePanel::GetSourceFileName()
{
	tstring sSourceFile = m_pSourceFileText->GetText();

	if (sSourceFile.length() <= 4 || sSourceFile.substr(sSourceFile.length()-4) != ".txt")
		sSourceFile.append(".txt");

	return "../sources/" + sSourceFile;
}

void CCreateToySourcePanel::FileNamesChanged()
{
	m_pWarnings->SetText("");

	tstring sToyFile = GetToyFileName();
	if (IsFile(sToyFile))
		m_pWarnings->SetText("WARNING: This toy file already exists. It will be overwritten when the new source file is built.");

	tstring sSourceFile = GetSourceFileName();
	if (IsFile(sSourceFile))
	{
		if (m_pWarnings->GetText().length())
			m_pWarnings->AppendText("\n\n");
		m_pWarnings->AppendText("WARNING: This source file already exists. It will be overwritten.");
	}

	m_pCreate->SetVisible(false);

	if (m_pSourceFileText->GetText().length() && m_pToyFileText->GetText().length())
		m_pCreate->SetVisible(true);
}

void CCreateToySourcePanel::CreateCallback(const tstring& sArgs)
{
	if (!m_pSourceFileText->GetText().length() || !m_pToyFileText->GetText().length())
		return;

	ToyEditor()->NewToy();
	ToyEditor()->GetToy().m_sFilename = GetSourceFileName();
	ToyEditor()->GetToy().m_sToyFile = GetToyFileName();

	SetVisible(false);

	ToyEditor()->SetupMenu();
}

CToyEditor* CToyEditor::s_pToyEditor = nullptr;

CToyEditor::CToyEditor()
{
	s_pToyEditor = this;

	m_pCreateToySourcePanel = new CCreateToySourcePanel();
	m_pCreateToySourcePanel->Layout();
	m_pCreateToySourcePanel->Center();
	m_pCreateToySourcePanel->SetVisible(false);
}

CToyEditor::~CToyEditor()
{
	delete m_pCreateToySourcePanel;
}

void CToyEditor::Activate()
{
	if (!m_oToySource.m_sFilename.length())
		m_pCreateToySourcePanel->SetVisible(true);

	SetupMenu();
}

void CToyEditor::Deactivate()
{
	m_pCreateToySourcePanel->SetVisible(false);
}

void CToyEditor::SetupMenu()
{
	GetFileMenu()->ClearSubmenus();

	GetFileMenu()->AddSubmenu("New", this, NewToy);

	if (GetToy().m_sFilename.length())
		GetFileMenu()->AddSubmenu("Save", this, SaveToy);
}

void CToyEditor::NewToy()
{
	m_oToySource = CToySource();
}

void CToyEditor::NewToyCallback(const tstring& sArgs)
{
	m_pCreateToySourcePanel->SetVisible(true);
}

void CToyEditor::SaveToyCallback(const tstring& sArgs)
{
	m_oToySource.Save();
}

bool CToyEditor::KeyPress(int c)
{
	// ; because my dvorak to qwerty key mapper works against me when the game is open, oh well.
	if ((c == 'S' || c == ';') && Application()->IsCtrlDown())
	{
		m_oToySource.Save();

		return true;
	}

	return false;
}

void CToySource::Save()
{
	if (!m_sFilename.length())
		return;

	CreateDirectory(GetDirectory(m_sFilename));

	std::basic_ofstream<tchar> f(m_sFilename.c_str());

	tstring sMessage = "// Generated by the Tinker Engine\n// Feel free to modify\n\n";
	f.write(sMessage.data(), sMessage.length());

	tstring sGame = "Game: " + GetRelativePath(".", GetDirectory(m_sFilename)) + "\n";
	f.write(sGame.data(), sGame.length());

	tstring sOutput = "Output: " + m_sToyFile + "\n";
	f.write(sOutput.data(), sOutput.length());
}
