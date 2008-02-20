// $Id$

#include "cewmain.h"
#include "cewedit.h"
#include "cewfind.h"
#include "wmsgdlg.h"

#include "tools.h"

CEWMain::CEWMain(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(750, 550),
	      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetTitle(_("CryptoTE v0.1"));
    CreateMenuBar();

    SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE) );

    statusbar = new CEWStatusBar(this);
    SetStatusBar(statusbar);
    statusbar->SetStatusText(_("Welcome to CryptoTE..."));

    // *** Create Controls ***

    editctrl = new CEWEdit(this, myID_EDITCTRL, wxDefaultPosition, wxDefaultSize,
			   wxBORDER_SUNKEN);
    editctrl->SetFocus();

    // Quick-Find Bar

    #include "art/window_close.h"
    #include "art/go_up.h"
    #include "art/go_down.h"

    wxBitmapButton* buttonQuickFindClose
	= new wxBitmapButton(this, myID_QUICKFIND_CLOSE, wxBitmapFromMemory(window_close_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindClose->SetLabel(_("Close"));
    buttonQuickFindClose->SetToolTip(_("Close Quick-Find bar"));

    wxStaticText* labelQuickFind = new wxStaticText(this, wxID_ANY, _("Find: "));

    textctrlQuickFind = new wxTextCtrl(this, myID_QUICKFIND_TEXT, wxEmptyString);

    wxBitmapButton* buttonQuickFindNext
	= new wxBitmapButton(this, myID_QUICKFIND_NEXT, wxBitmapFromMemory(go_down_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindNext->SetLabel(_("Next"));
    buttonQuickFindNext->SetToolTip(_("Search for next occurance"));

    wxBitmapButton* buttonQuickFindPrev
	= new wxBitmapButton(this, myID_QUICKFIND_PREV, wxBitmapFromMemory(go_up_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindPrev->SetLabel(_("Previous"));
    buttonQuickFindPrev->SetToolTip(_("Search for previous occurance"));

    // Quick-Goto Bar

    #include "art/go_next.h"

    wxBitmapButton* buttonGotoCancel
	= new wxBitmapButton(this, myID_GOTO_CLOSE, wxBitmapFromMemory(window_close_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonGotoCancel->SetLabel(_("Cancel"));
    buttonGotoCancel->SetToolTip(_("Cancel Go to Line"));

    wxStaticText* labelGoto = new wxStaticText(this, wxID_ANY, _("Goto: "));

    textctrlGoto = new wxTextCtrl(this, myID_GOTOTEXT, wxEmptyString,
				   wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    wxBitmapButton* buttonGotoGo
	= new wxBitmapButton(this, myID_GOTO_GO, wxBitmapFromMemory(go_next_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonGotoGo->SetLabel(_("Go"));
    buttonGotoGo->SetToolTip(_("Go to Line"));

    // *** Frame Layout ***

    // Quick-Find Bar

    sizerQuickFind = new wxBoxSizer(wxHORIZONTAL);
    sizerQuickFind->Add(buttonQuickFindClose, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickFind->Add(labelQuickFind, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickFind->Add(textctrlQuickFind, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickFind->Add(buttonQuickFindNext, 0, wxALL, 2);
    sizerQuickFind->Add(buttonQuickFindPrev, 0, wxALL, 2);

    // Quick-Goto Bar

    sizerQuickGoto = new wxBoxSizer(wxHORIZONTAL);
    sizerQuickGoto->Add(buttonGotoCancel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickGoto->Add(labelGoto, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickGoto->Add(textctrlGoto, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickGoto->Add(buttonGotoGo, 0, wxALL, 2);

    // Main Frame

    sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(editctrl, 1, wxEXPAND, 0);

    sizerMain->Add(sizerQuickFind, 0, wxLEFT | wxRIGHT | wxEXPAND, 2);
    sizerMain->Hide(sizerQuickFind);
    quickfind_visible = false;

    sizerMain->Add(sizerQuickGoto, 0, wxLEFT | wxRIGHT | wxEXPAND, 2);
    sizerMain->Hide(sizerQuickGoto);
    quickgoto_visible = false;

    SetSizer(sizerMain);
    Layout();
    Centre();

    // Default Settings

    UpdateTitle();
    findreplace_dlg = NULL;
}

void CEWMain::UpdateTitle()
{
    wxString title = editctrl->GetFileBasename();

    if (editctrl->ModifiedFlag()) {
	title += _(" (Modified)");
    }

    title += _(" - ");
    title += _("CryptoTE");

    SetTitle(title);
}

void CEWMain::UpdateStatusBar(const wxString& str)
{
    statusbar->SetStatusText(str);
}

bool CEWMain::FileOpen(const wxString& filename)
{
    bool b = editctrl->FileOpen(filename);

    UpdateTitle();
    return b;
}

bool CEWMain::FileSave()
{
    if (!editctrl->HasFilename()) {
	return FileSaveAs();
    }

    return editctrl->FileSave();
}

bool CEWMain::FileSaveAs()
{
    wxFileDialog dlg(this,
		     _("Save file"), wxEmptyString, editctrl->GetFileBasename(), _("Any file (*)|*"),
		     wxSAVE | wxOVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK) return false;

    editctrl->FileSaveAs( dlg.GetPath() );

    return true;
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
    #include "art/edit_find.h"
    #include "art/go_next.h"

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

    menuEdit->Append( createMenuItem(menuEdit, myID_QUICKFIND,
				     _("&Quick-Find\tCtrl+F"),
				     _("Find a string in the buffer."),
				     wxBitmapFromMemory(edit_find_png)) );

    menuEdit->Append( createMenuItem(menuEdit, wxID_FIND,
				     _("&Find...\tCtrl+Shift+F"),
				     _("Open find dialog to search for a string in the buffer."),
				     wxBitmapFromMemory(edit_find_png)) );

    menuEdit->Append( createMenuItem(menuEdit, wxID_REPLACE,
				     _("&Replace\tCtrl+H"),
				     _("Open find and replace dialog to search for and replace a string in the buffer."),
				     wxBitmapFromMemory(edit_find_png)) );

    menuEdit->AppendSeparator();

    toolbar->AddTool(wxID_FIND,
		     _("Find Text"),
		     wxBitmapFromMemory(edit_find_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Find Text"),
		     _("Open find dialog to search for a string in the buffer."));

    toolbar->AddTool(wxID_REPLACE,
		     _("Find and Replace Text"),
		     wxBitmapFromMemory(edit_find_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Find and Replace Text"),
		     _("Open find and replace dialog to search for and replace a string in the buffer."));

    menuEdit->Append( createMenuItem(menuEdit, myID_GOTO,
				     _("&Go to Line...\tCtrl+G"),
				     _("Jump to the entered line number."),
				     wxBitmapFromMemory(go_next_png)) );

    menuEdit->AppendSeparator();


    menuEdit->Append(wxID_SELECTALL,
		     _("&Select all\tCtrl+A"),
		     _("Select all text in the current buffer."));

    menuEdit->Append(myID_MENU_SELECTLINE,
		     _("Select &line\tCtrl+L"),
		     _("Select whole line at the current cursor position."));

    // *** View

    wxMenu *menuView = new wxMenu;

    menuView->AppendCheckItem(myID_MENU_LINEWRAP,
			      _("&Wrap long lines"),
			      _("Wrap long lines in editor."));

    menuView->AppendCheckItem(myID_MENU_LINENUMBER,
			      _("Show line &numbers"),
			      _("Show line numbers on left margin."));

    menuView->AppendCheckItem(myID_MENU_WHITESPACE,
			      _("Show white&space"),
			      _("Show white space (space and tab) in buffer."));

    menuView->AppendCheckItem(myID_MENU_ENDOFLINE,
			      _("Show &end of line symbols"),
			      _("Show end of line symbols (new-line and carriage-return) in buffer."));

    menuView->AppendCheckItem(myID_MENU_INDENTGUIDE,
			      _("Show &indent guide lines"),
			      _("Show guide lines following the indention depth."));

    menuView->AppendCheckItem(myID_MENU_LONGLINEGUIDE,
			      _("Show guide line at &column 80"),
			      _("Show guide line at column 80 to display over-long lines."));

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
    menubar->Append(menuView, _("&View"));
    menubar->Append(menuHelp, _("&Help"));

    SetMenuBar(menubar);
    toolbar->Realize();
}

void CEWMain::OnClose(wxCloseEvent& event)
{
    if (!event.CanVeto()) {
	Destroy();
	return;
    }

    if (AllowCloseModified()) {
	Destroy();
    }
    else {
	event.Veto();
    }
}

void CEWMain::OnChar(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_ESCAPE)
    {
	// Hide Quick-Find Bar
	if (quickfind_visible) {
	    sizerMain->Hide(sizerQuickFind);
	    sizerMain->Layout();

	    quickfind_visible = false;
	}

	// Hide Quick-Goto Bar
	if (quickgoto_visible) {
	    sizerMain->Hide(sizerQuickGoto);
	    sizerMain->Layout();

	    quickgoto_visible = false;
	}
    }
}

bool CEWMain::AllowCloseModified()
{
    if (editctrl->ModifiedFlag())
    {
	while(1)
	{
	    WMessageDialog dlg(this,
			       wxString::Format(_("Save modified text file \"%s\"?"), editctrl->GetFileBasename().c_str()),
			       _("Close Application"),
			       wxICON_WARNING,
			       wxID_SAVE, wxID_NO, wxID_CANCEL);

	    int id = dlg.ShowModal();

	    if (id == wxID_SAVE)
	    {
		if (FileSave()) return true;
	    }
	    if (id == wxID_NO)
	    {
		return true;
	    }
	    if (id == wxID_CANCEL)
	    {
		return false;
	    }
	}
    }

    return true;
}

void CEWMain::OnMenuFileOpen(wxCommandEvent& WXUNUSED(event))
{
    if (!AllowCloseModified()) return;

    wxFileDialog dlg(this,
		     _("Open file"), wxEmptyString, wxEmptyString, _("Any file (*)|*"),
                     wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR);

    if (dlg.ShowModal() != wxID_OK) return;

    editctrl->FileOpen( dlg.GetPath() );

    UpdateTitle();
}

void CEWMain::OnMenuFileSave(wxCommandEvent& event)
{
    FileSave();
}

void CEWMain::OnMenuFileSaveAs(wxCommandEvent& WXUNUSED(event))
{
    FileSaveAs();
}

void CEWMain::OnMenuFileRevert(wxCommandEvent& WXUNUSED(event))
{
    editctrl->FileRevert();
}

void CEWMain::OnMenuFileClose(wxCommandEvent& WXUNUSED(event))
{
    if (!AllowCloseModified()) return;

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

void CEWMain::OnMenuEditQuickFind(wxCommandEvent& WXUNUSED(event))
{
    if (quickfind_visible)
    {
	// pushing Ctrl+F again is equivalent to Search-Next

	textctrlQuickFind->SetFocus();

	quickfind_startpos = editctrl->GetSelectionEnd();

	QuickFind(true);
    }
    else
    {
	// make quick find bar visible

	sizerMain->Show(sizerQuickFind);
	sizerMain->Hide(sizerQuickGoto);
	sizerMain->Layout();

	textctrlQuickFind->SetFocus();
	textctrlQuickFind->SetValue(wxT(""));

	quickgoto_visible = false;
	quickfind_visible = true;
	quickfind_startpos = editctrl->GetCurrentPos();
    }
}

void CEWMain::OnMenuEditFind(wxCommandEvent& WXUNUSED(event))
{
    if (!findreplace_dlg) {
	findreplace_dlg = new CEWFind(this);
    }

    findreplace_dlg->ShowReplace(false);
    findreplace_dlg->Show();
}

void CEWMain::OnMenuEditFindReplace(wxCommandEvent& WXUNUSED(event))
{
    if (!findreplace_dlg) {
	findreplace_dlg = new CEWFind(this);
    }

    findreplace_dlg->ShowReplace(true);
    findreplace_dlg->Show();
}

void CEWMain::OnMenuEditGoto(wxCommandEvent& WXUNUSED(event))
{
    sizerMain->Show(sizerQuickGoto);
    sizerMain->Hide(sizerQuickFind);
    sizerMain->Layout();

    quickfind_visible = false;
    quickgoto_visible = true;

    textctrlGoto->SetFocus();
}

void CEWMain::OnMenuViewLineWrap(wxCommandEvent& event)
{
    editctrl->SetWrapMode(event.IsChecked() ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
}

void CEWMain::OnMenuViewLineNumber(wxCommandEvent& event)
{
    editctrl->ShowLineNumber(event.IsChecked());
}

void CEWMain::OnMenuViewWhitespace(wxCommandEvent& event)
{
    editctrl->SetViewWhiteSpace(event.IsChecked() ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
}

void CEWMain::OnMenuViewEndOfLine(wxCommandEvent& event)
{
    editctrl->SetViewEOL(event.IsChecked());
}

void CEWMain::OnMenuViewIndentGuide(wxCommandEvent& event)
{
    editctrl->SetIndentationGuides(event.IsChecked());
}

void CEWMain::OnMenuViewLonglineGuide(wxCommandEvent& event)
{
    editctrl->SetEdgeColumn(80);
    editctrl->SetEdgeMode(event.IsChecked() ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
}

void CEWMain::OnMenuHelpAbout(wxCommandEvent& WXUNUSED(event))
{
    wxLogMessage(_T("OnMenuHelpAbout() called."));
}

// *** Scintilla Callbacks ***

void CEWMain::OnScintillaUpdateUI(wxStyledTextEvent& WXUNUSED(event))
{
    // Enable or Disable Menu Items and Tool Bar Items

    menubar->Enable(wxID_UNDO, editctrl->CanUndo());
    menubar->Enable(wxID_REDO, editctrl->CanRedo());

    menubar->Enable(wxID_PASTE, editctrl->CanPaste());

    toolbar->EnableTool(wxID_UNDO, editctrl->CanUndo());
    toolbar->EnableTool(wxID_REDO, editctrl->CanRedo());

    toolbar->EnableTool(wxID_PASTE, editctrl->CanPaste());

    bool HasSelection = editctrl->GetSelectionEnd() > editctrl->GetSelectionStart();

    menubar->Enable(wxID_CUT, HasSelection);
    menubar->Enable(wxID_COPY, HasSelection);
    menubar->Enable(wxID_CLEAR, HasSelection);

    toolbar->EnableTool(wxID_CUT, HasSelection);
    toolbar->EnableTool(wxID_COPY, HasSelection);
    toolbar->EnableTool(wxID_CLEAR, HasSelection);

    // Update status bar field
    {
	int pos = editctrl->GetCurrentPos();
	int row = editctrl->LineFromPosition(pos);
	int col = editctrl->GetColumn(pos);
	int sel = editctrl->GetSelectionEnd () - editctrl->GetSelectionStart();

	wxString sb;
	sb.Printf( _("Ln %d Col %d Sel %d"), row, col, sel);

	statusbar->SetStatusText(sb, 1);
    }
}

void CEWMain::UpdateOnSavePoint()
{
    menubar->Enable(wxID_SAVE, editctrl->ModifiedFlag());
    menubar->Enable(wxID_REVERT, editctrl->ModifiedFlag());

    toolbar->EnableTool(wxID_SAVE, editctrl->ModifiedFlag());

    // statusbar->SetLock( editctrl->IsEncrypted() );

    UpdateTitle();
}

void CEWMain::OnScintillaSavePointReached(wxStyledTextEvent& WXUNUSED(event))
{
    // Document is un-modified
    
    editctrl->ModifiedFlag() = false;

    UpdateOnSavePoint();
}

void CEWMain::OnScintillaSavePointLeft(wxStyledTextEvent& WXUNUSED(event))
{
    // Document is modified

    editctrl->ModifiedFlag() = true;

    UpdateOnSavePoint();
}

// *** Quick-Find Bar ***

void CEWMain::QuickFind(bool forward)
{
    wxString findtext = textctrlQuickFind->GetValue();

    if (findtext.IsEmpty())
    {
	// move cursor and screen back to search start position
	
	editctrl->SetSelection(quickfind_startpos, quickfind_startpos);
	
	UpdateStatusBar( wxT("") );
	
	textctrlQuickFind->SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW) );
	textctrlQuickFind->Refresh();

	return;
    }

    if (forward)
    {
	editctrl->SetTargetStart(quickfind_startpos);
	editctrl->SetTargetEnd(editctrl->GetLength());
    }
    else
    {
	editctrl->SetTargetStart(quickfind_startpos);
	editctrl->SetTargetEnd(0);
    }

    editctrl->SetSearchFlags(0);

    int respos = editctrl->SearchInTarget(findtext);

    bool wrapped = false;

    if (respos < 0)
    {
	// wrap-around search
	wrapped = true;

	if (forward)
	{
	    editctrl->SetTargetStart(0);
	    editctrl->SetTargetEnd(quickfind_startpos);
	}
	else
	{
	    editctrl->SetTargetStart(editctrl->GetLength());
	    editctrl->SetTargetEnd(quickfind_startpos);
	}

	respos = editctrl->SearchInTarget(findtext);
    }

    bool found = false;
    if (respos >= 0)
    {
	found = true;
	int start = editctrl->GetTargetStart();
	int end = editctrl->GetTargetEnd();

	editctrl->EnsureVisible( editctrl->LineFromPosition(start) );
	editctrl->SetSelection(start, end);
    }

    if (found && !wrapped) {
	UpdateStatusBar( wxT("") );
    }
    else if (found && wrapped) {
	if (forward)
	    UpdateStatusBar( _("Search wrapped to beginning of document.") );
	else
	    UpdateStatusBar( _("Search wrapped to end of document.") );
    }
    else if (!found) {
	UpdateStatusBar( _("Search string not found in document.") );
    }

    wxColor clr;
    if (found) clr = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    else clr = wxColor(255, 102, 102);

    textctrlQuickFind->SetBackgroundColour(clr);
    textctrlQuickFind->Refresh();
}

void CEWMain::OnTextQuickFind(wxCommandEvent& WXUNUSED(event))
{
    QuickFind(true);
}

void CEWMain::OnButtonQuickFindNext(wxCommandEvent& WXUNUSED(event))
{
    quickfind_startpos = editctrl->GetSelectionEnd();

    QuickFind(true);
}

void CEWMain::OnButtonQuickFindPrev(wxCommandEvent& WXUNUSED(event))
{
    quickfind_startpos = editctrl->GetSelectionStart();

    QuickFind(false);
}

void CEWMain::OnButtonQuickFindClose(wxCommandEvent& WXUNUSED(event))
{
    sizerMain->Hide(sizerQuickFind);
    sizerMain->Layout();

    quickfind_visible = false;

    editctrl->SetFocus();
}

// *** Quick-Goto Bar ***

void CEWMain::OnButtonGotoGo(wxCommandEvent& WXUNUSED(event))
{
    long linenum;

    if (! textctrlGoto->GetValue().ToLong(&linenum) ) {
	UpdateStatusBar(_("Yeah right. Enter a number smarty."));
	textctrlGoto->SetFocus();
	return;
    }

    editctrl->GotoLine(linenum);
    UpdateStatusBar(wxString::Format(_("Jumped to line %d."), editctrl->GetCurrentLine()));

    sizerMain->Hide(sizerQuickGoto);
    sizerMain->Layout();

    editctrl->SetFocus();
}

void CEWMain::OnButtonGotoClose(wxCommandEvent& WXUNUSED(event))
{
    sizerMain->Hide(sizerQuickGoto);
    sizerMain->Layout();

    editctrl->SetFocus();
}


BEGIN_EVENT_TABLE(CEWMain, wxFrame)

    // *** Generic Events

    EVT_CLOSE	(CEWMain::OnClose)

    EVT_CHAR	(CEWMain::OnChar)

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

    EVT_MENU	(myID_QUICKFIND,	CEWMain::OnMenuEditQuickFind)
    EVT_MENU	(wxID_FIND,		CEWMain::OnMenuEditFind)
    EVT_MENU	(wxID_REPLACE,		CEWMain::OnMenuEditFindReplace)

    EVT_MENU	(myID_GOTO,		CEWMain::OnMenuEditGoto)

    EVT_MENU	(wxID_SELECTALL,	CEWMain::OnMenuEditGeneric)
    EVT_MENU	(myID_MENU_SELECTLINE,	CEWMain::OnMenuEditGeneric)

    // View
    EVT_MENU	(myID_MENU_LINEWRAP,	CEWMain::OnMenuViewLineWrap)
    EVT_MENU	(myID_MENU_LINENUMBER,	CEWMain::OnMenuViewLineNumber)
    EVT_MENU	(myID_MENU_WHITESPACE,	CEWMain::OnMenuViewWhitespace)
    EVT_MENU	(myID_MENU_ENDOFLINE,	CEWMain::OnMenuViewEndOfLine)
    EVT_MENU	(myID_MENU_INDENTGUIDE,	CEWMain::OnMenuViewIndentGuide)
    EVT_MENU	(myID_MENU_LONGLINEGUIDE, CEWMain::OnMenuViewLonglineGuide)

    // Help
    EVT_MENU	(wxID_ABOUT,		CEWMain::OnMenuHelpAbout)

    // *** Scintilla Edit Callbacks

    EVT_STC_UPDATEUI(myID_EDITCTRL,		CEWMain::OnScintillaUpdateUI)
    EVT_STC_SAVEPOINTREACHED(myID_EDITCTRL,	CEWMain::OnScintillaSavePointReached)
    EVT_STC_SAVEPOINTLEFT(myID_EDITCTRL,	CEWMain::OnScintillaSavePointLeft)

    // *** Quick-Find Bar

    EVT_TEXT	(myID_QUICKFIND_TEXT,	CEWMain::OnTextQuickFind)

    EVT_BUTTON	(myID_QUICKFIND_NEXT,	CEWMain::OnButtonQuickFindNext)
    EVT_BUTTON	(myID_QUICKFIND_PREV,	CEWMain::OnButtonQuickFindPrev)
    EVT_BUTTON	(myID_QUICKFIND_CLOSE,	CEWMain::OnButtonQuickFindClose)

    // *** Quick-Goto Bar

    EVT_TEXT_ENTER(myID_GOTOTEXT,	CEWMain::OnButtonGotoGo)
    EVT_BUTTON	(myID_GOTO_GO,		CEWMain::OnButtonGotoGo)
    EVT_BUTTON	(myID_GOTO_CLOSE,	CEWMain::OnButtonGotoClose)

END_EVENT_TABLE()

CEWStatusBar::CEWStatusBar(wxWindow *parent)
    : wxStatusBar(parent, wxID_ANY, 0)
{

    static const int statusbar_widths[3] = { -1, 150, 28 };

    SetFieldsCount(3);
    SetStatusWidths(3, statusbar_widths);

    #include "art/stock_lock.h"

    lockbitmap = new wxStaticBitmap(this, wxID_ANY, wxIconFromMemory(stock_lock_png));
}

void CEWStatusBar::OnSize(wxSizeEvent& event)
{
    // move bitmap to position
    wxRect rect;
    GetFieldRect(2, rect);
    wxSize size = lockbitmap->GetSize();

    lockbitmap->Move(rect.x + (rect.width - size.x) / 2,
		     rect.y + (rect.height - size.y) / 2);

    event.Skip();
}

void CEWStatusBar::SetLock(bool on)
{
    #include "art/stock_lock.h"
    #include "art/stock_unlock.h"

    if (on) {
	lockbitmap->SetBitmap(wxIconFromMemory(stock_lock_png));
	lockbitmap->SetToolTip(_("Text file is encrypted."));
    }
    else {
	lockbitmap->SetBitmap(wxIconFromMemory(stock_unlock_png));
	lockbitmap->SetToolTip(_("Text file is NOT encrypted."));
    }
}

BEGIN_EVENT_TABLE(CEWStatusBar, wxStatusBar)

    EVT_SIZE	(CEWStatusBar::OnSize)

END_EVENT_TABLE();
