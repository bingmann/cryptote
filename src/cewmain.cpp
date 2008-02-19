// $Id$

#include "cewmain.h"

CEWMain::CEWMain(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(750, 550),
	      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetTitle(_("CryptoTE v0.1"));
    CreateMenuBar();

    statusbar = CreateStatusBar(1, wxST_SIZEGRIP);
    statusbar->SetStatusText(_("Welcome to CryptoTE..."));

    Centre();
}

void CEWMain::CreateMenuBar()
{
    // *** File

    wxMenu *menuFile = new wxMenu;

    menuFile->Append(wxID_NEW,
		     _("&New ...\tCtrl+N"),
		     _("Clear the editor and start with a empty file."));
    menuFile->Append(wxID_OPEN,
		     _("&Open ..\tCtrl+O"),
		     _("Open an existing file in the editor."));
    menuFile->Append(wxID_SAVE,
		     _("&Save\tCtrl+S"),
		     _("Save the currently displayed buffer to the disk."));
    menuFile->Append(wxID_SAVEAS,
		     _("Save &as ..\tCtrl+Shift+S"),
		     _("Choose a file name and save the currently displayed buffer to the disk."));
    menuFile->Append(wxID_REVERT,
		     _("&Revert\tShift+Ctrl+W"),
		     _("Revert the currently displayed file losing all unsaved changes."));
    menuFile->Append(wxID_CLOSE,
		     _("&Close\tCtrl+W"),
		     _("Close the currently displayed file."));

    menuFile->AppendSeparator();

    menuFile->Append(wxID_EXIT,
		     _("&Quit\tCtrl+Q"),
		     _("Exit CryptoTE"));

    // *** Help

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT,
		     _("&About\tShift+F1"),
		     _("Show some information about CryptoTE"));

    // construct menubar and add it to the window
    menubar = new wxMenuBar;

    menubar->Append(menuFile, _("&File"));
    menubar->Append(menuHelp, _("&Help"));

    SetMenuBar(menubar);
}

void CEWMain::OnMenuFileNew(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuFileNew() called."));
}

void CEWMain::OnMenuFileOpen(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuFileOpen() called."));
}

void CEWMain::OnMenuFileSave(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuFileSave() called."));
}

void CEWMain::OnMenuFileSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuFileSaveAs() called."));
}

void CEWMain::OnMenuFileRevert(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuFileRevert() called."));
}

void CEWMain::OnMenuFileClose(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuFileClose() called."));
}

void CEWMain::OnMenuFileQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void CEWMain::OnMenuHelpAbout(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuHelpAbout() called."));
}

BEGIN_EVENT_TABLE(CEWMain, wxFrame)

    // *** Menu Items

    // File
    EVT_MENU	(wxID_NEW,		CEWMain::OnMenuFileNew)
    EVT_MENU	(wxID_OPEN,		CEWMain::OnMenuFileOpen)
    EVT_MENU	(wxID_SAVE,		CEWMain::OnMenuFileSave)
    EVT_MENU	(wxID_SAVEAS,		CEWMain::OnMenuFileSaveAs)
    EVT_MENU	(wxID_REVERT,		CEWMain::OnMenuFileRevert)
    EVT_MENU	(wxID_CLOSE,		CEWMain::OnMenuFileClose)

    EVT_MENU	(wxID_EXIT,		CEWMain::OnMenuFileQuit)

    // Help
    EVT_MENU	(wxID_ABOUT,		CEWMain::OnMenuHelpAbout)

END_EVENT_TABLE()
