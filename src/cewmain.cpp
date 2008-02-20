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

    editctrl = new CEWEdit(this, myID_EDITCTRL, wxDefaultPosition, wxDefaultSize,
			   wxBORDER_SUNKEN);
    editctrl->SetFocus();

    Centre();
}

void CEWMain::UpdateTitle()
{
    SetTitle( editctrl->GetFileBasename() + _(" - ") + _("Secretilla"));
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

    // *** Edit

    wxMenu *menuEdit = new wxMenu;

    #include "art/edit_undo.h"
    #include "art/edit_redo.h"
    #include "art/edit_cut.h"
    #include "art/edit_copy.h"
    #include "art/edit_paste.h"
    #include "art/edit_clear.h"

    menuEdit->Append( createMenuItem(menuEdit, wxID_UNDO,
				     _("&Undo\tCtrl+Z"),
				     _("Undo the last change."),
				     wxBitmapFromMemory(edit_undo_png)) );

    menuEdit->Append( createMenuItem(menuEdit, wxID_REDO,
				     _("&Redo\tCtrl+Shift+Z"),
				     _("Redo the previously undone change."),
				     wxBitmapFromMemory(edit_redo_png)) );

    menuEdit->AppendSeparator();

    toolbar->AddTool(wxID_UNDO,
		     _("Undo Operation"),
		     wxBitmapFromMemory(edit_undo_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Undo Operation"),
		     _("Undo the last change."));

    toolbar->AddTool(wxID_REDO,
		     _("Redo Operation"),
		     wxBitmapFromMemory(edit_redo_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Redo Operation"),
		     _("Redo the previously undone change."));

    toolbar->AddSeparator();

    menuEdit->Append( createMenuItem(menuEdit, wxID_CUT,
				     _("Cu&t\tCtrl+X"),
				     _("Cut selected text into clipboard."),
				     wxBitmapFromMemory(edit_cut_png)) );

    menuEdit->Append( createMenuItem(menuEdit, wxID_COPY,
				     _("&Copy\tCtrl+C"),
				     _("Copy selected text into clipboard."),
				     wxBitmapFromMemory(edit_copy_png)) );

    menuEdit->Append( createMenuItem(menuEdit, wxID_PASTE,
				     _("&Paste\tCtrl+V"),
				     _("Paste clipboard contents at the current text position."),
				     wxBitmapFromMemory(edit_paste_png)) );

    menuEdit->Append( createMenuItem(menuEdit, wxID_CLEAR,
				     _("&Delete\tDel"),
				     _("Delete selected text."),
				     wxBitmapFromMemory(edit_clear_png)) );

    menuEdit->AppendSeparator();

    toolbar->AddTool(wxID_CUT,
		     _("Cut Selection"),
		     wxBitmapFromMemory(edit_cut_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Cut Selection"),
		     _("Cut selected text into clipboard."));

    toolbar->AddTool(wxID_COPY,
		     _("Copy Selection"),
		     wxBitmapFromMemory(edit_copy_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Copy Selection"),
		     _("Copy selected text into clipboard."));

    toolbar->AddTool(wxID_PASTE,
		     _("Paste Clipboard"),
		     wxBitmapFromMemory(edit_paste_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Paste Clipboard"),
		     _("Paste clipboard contents at the current text position."));

    toolbar->AddSeparator();

    menuEdit->Append(wxID_SELECTALL,
		     _("&Select all\tCtrl+A"),
		     _("Select all text in the current buffer."));

    menuEdit->Append(myID_MENU_SELECTLINE,
		     _("Select &line\tCtrl+L"),
		     _("Select whole line at the current cursor position."));

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
    menubar->Append(menuEdit, _("&Edit"));
    menubar->Append(menuHelp, _("&Help"));

    SetMenuBar(menubar);
}


void CEWMain::OnMenuFileOpen(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dlg(this,
		     _("Open file"), wxEmptyString, wxEmptyString, _("Any file (*)|*"),
                     wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR);

    if (dlg.ShowModal() != wxID_OK) return;

    editctrl->FileOpen( dlg.GetPath() );

    UpdateTitle();
}

void CEWMain::OnMenuFileSave(wxCommandEvent& event)
{
    if (!editctrl->HasFilename()) {
	return OnMenuFileSaveAs(event);
    }

    editctrl->FileSave();
}

void CEWMain::OnMenuFileSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dlg(this,
		     _("Save file"), wxEmptyString, editctrl->GetFileBasename(), _("Any file (*)|*"),
		     wxSAVE | wxOVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK) return;

    editctrl->FileSaveAs( dlg.GetPath() );
}

void CEWMain::OnMenuFileRevert(wxCommandEvent& WXUNUSED(event))
{
    editctrl->FileRevert();
}

void CEWMain::OnMenuFileClose(wxCommandEvent& WXUNUSED(event))
{
    editctrl->FileNew();
}

void CEWMain::OnMenuFileQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void CEWMain::OnMenuEditGeneric(wxCommandEvent& event)
{
    editctrl->ProcessEvent(event);
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

    // Edit
    EVT_MENU	(wxID_UNDO,		CEWMain::OnMenuEditGeneric)
    EVT_MENU	(wxID_REDO,		CEWMain::OnMenuEditGeneric)

    EVT_MENU	(wxID_CUT,		CEWMain::OnMenuEditGeneric)
    EVT_MENU	(wxID_COPY,		CEWMain::OnMenuEditGeneric)
    EVT_MENU	(wxID_PASTE,		CEWMain::OnMenuEditGeneric)
    EVT_MENU	(wxID_CLEAR,		CEWMain::OnMenuEditGeneric)

    EVT_MENU	(wxID_SELECTALL,	CEWMain::OnMenuEditGeneric)
    EVT_MENU	(myID_MENU_SELECTLINE,	CEWMain::OnMenuEditGeneric)

    // Help
    EVT_MENU	(wxID_ABOUT,		CEWMain::OnMenuHelpAbout)

END_EVENT_TABLE()
