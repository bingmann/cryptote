// $Id$

#include "cewmain.h"
#include "cewedit.h"

#include "tools.h"

CEWMain::CEWMain(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(750, 550),
	      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetTitle(_("CryptoTE v0.1"));
    CreateMenuBar();

    statusbar = CreateStatusBar(1, wxST_SIZEGRIP);
    statusbar->SetStatusText(_("Welcome to CryptoTE..."));

    edit = new CEWEdit(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		       wxBORDER_SUNKEN);
    edit->SetFocus();

    Centre();
}

void CEWMain::UpdateTitle()
{
    SetTitle( edit->GetFileBasename() + _(" - ") + _("Secretilla"));
}

void CEWMain::UpdateStatusBar(const wxString& str)
{
    statusbar->SetStatusText(str);
}

static inline wxMenuItem* createMenuItem(class wxMenu* parentMenu, int id,
					 const wxString& text, const wxString& helpString,
					 const wxBitmap& bmp)
{
    wxMenuItem* mi = new wxMenuItem(parentMenu, id, text, helpString);
    mi->SetBitmap(bmp);
    return mi;
}

void CEWMain::CreateMenuBar()
{
    toolbar = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
    toolbar->SetToolBitmapSize(wxSize(16, 16));

    // *** File

    wxMenu *menuFile = new wxMenu;

    #include "art/document_open.h"
    #include "art/document_save.h"
    #include "art/document_saveas.h"
    #include "art/document_revert.h"
    #include "art/document_close.h"
    #include "art/application_exit.h"

    menuFile->Append( createMenuItem(menuFile, wxID_OPEN,
				     _("&Open ..\tCtrl+O"),
				     _("Open an existing file in the editor."),
				     wxBitmapFromMemory(document_open_png)) );
    menuFile->Append( createMenuItem(menuFile, wxID_SAVE,
				     _("&Save\tCtrl+S"),
				     _("Save the currently displayed buffer to the disk."),	
				     wxBitmapFromMemory(document_save_png)) );
    menuFile->Append( createMenuItem(menuFile, wxID_SAVEAS,
				     _("Save &as ..\tCtrl+Shift+S"),
				     _("Choose a file name and save the currently displayed buffer to the disk."),
				     wxBitmapFromMemory(document_saveas_png)) );
    menuFile->Append( createMenuItem(menuFile, wxID_REVERT,
				     _("&Revert\tShift+Ctrl+W"),
				     _("Revert the currently displayed file losing all unsaved changes."),
				     wxBitmapFromMemory(document_revert_png)) );

    menuFile->Append( createMenuItem(menuFile, wxID_CLOSE,
				     _("&Close\tCtrl+W"),
				     _("Close the currently displayed file."),
				     wxBitmapFromMemory(document_close_png)) );

    menuFile->AppendSeparator();

    toolbar->AddTool(wxID_OPEN,
		     _("Open File"),
		     wxBitmapFromMemory(document_open_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Open File"),
		     _("Open an existing file in the editor."));

    toolbar->AddTool(wxID_SAVE,
		     _("Save File"),
		     wxBitmapFromMemory(document_save_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Save File"),
		     _("Save the currently displayed buffer to the disk."));

    toolbar->AddTool(wxID_SAVEAS,
		     _("Save File as..."),
		     wxBitmapFromMemory(document_saveas_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Save File as..."),
		     _("Choose a file name and save the currently displayed buffer to the disk."));

    toolbar->AddSeparator();

    menuFile->Append( createMenuItem(menuFile, wxID_EXIT,
				     _("&Quit\tCtrl+Q"),
				     _("Exit CryptoTE"),
				     wxBitmapFromMemory(application_exit_png)) );

    // *** Help

    wxMenu *menuHelp = new wxMenu;

    #include "art/application_info.h"

    menuHelp->Append( createMenuItem(menuFile, wxID_ABOUT,
				     _("&About\tShift+F1"),
				     _("Show some information about CryptoTE."),
				     wxBitmapFromMemory(application_info_png)) );

    // construct menubar and add it to the window
    menubar = new wxMenuBar;

    menubar->Append(menuFile, _("&File"));
    menubar->Append(menuHelp, _("&Help"));

    SetMenuBar(menubar);
}


void CEWMain::OnMenuFileOpen(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dlg(this,
		     _("Open file"), wxEmptyString, wxEmptyString, _("Any file (*)|*"),
                     wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR);

    if (dlg.ShowModal() != wxID_OK) return;

    edit->FileOpen( dlg.GetPath() );

    UpdateTitle();
}

void CEWMain::OnMenuFileSave(wxCommandEvent& event)
{
    if (!edit->HasFilename()) {
	return OnMenuFileSaveAs(event);
    }

    edit->FileSave();
}

void CEWMain::OnMenuFileSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dlg(this,
		     _("Save file"), wxEmptyString, edit->GetFileBasename(), _("Any file (*)|*"),
		     wxSAVE | wxOVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK) return;

    edit->FileSaveAs( dlg.GetPath() );
}

void CEWMain::OnMenuFileRevert(wxCommandEvent& WXUNUSED(event))
{
    edit->FileRevert();
}

void CEWMain::OnMenuFileClose(wxCommandEvent& WXUNUSED(event))
{
    edit->FileNew();
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
    EVT_MENU	(wxID_OPEN,		CEWMain::OnMenuFileOpen)
    EVT_MENU	(wxID_SAVE,		CEWMain::OnMenuFileSave)
    EVT_MENU	(wxID_SAVEAS,		CEWMain::OnMenuFileSaveAs)
    EVT_MENU	(wxID_REVERT,		CEWMain::OnMenuFileRevert)
    EVT_MENU	(wxID_CLOSE,		CEWMain::OnMenuFileClose)

    EVT_MENU	(wxID_EXIT,		CEWMain::OnMenuFileQuit)

    // Help
    EVT_MENU	(wxID_ABOUT,		CEWMain::OnMenuHelpAbout)

END_EVENT_TABLE()
