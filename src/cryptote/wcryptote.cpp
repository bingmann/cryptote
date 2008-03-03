// $Id$

#include "wcryptote.h"
#include "wtextpage.h"
#include "wfind.h"

#include "common/tools.h"

WCryptoTE::WCryptoTE(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(750, 550),
	      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    cpage = NULL;

    {	// Program Icon
    
        #include "art/cryptote-16.h"
        #include "art/cryptote-32.h"
        #include "art/cryptote-48.h"

	wxIconBundle progicon;
	progicon.AddIcon( wxIconFromMemory(cryptote_16_png) );
	progicon.AddIcon( wxIconFromMemory(cryptote_32_png) );
	progicon.AddIcon( wxIconFromMemory(cryptote_48_png) );

	SetIcons(progicon);
    }

    SetTitle(_("CryptoTE v0.1"));

    CreateMenuBar();

    statusbar = new WStatusBar(this);
    SetStatusBar(statusbar);
    UpdateStatusBar(_("Welcome to CryptoTE..."));

    { // Accelerator to handle ESC key

	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, myID_ACCEL_ESCAPE);
	wxAcceleratorTable accel(1, entries);
	SetAcceleratorTable(accel);
    }

    // *** Set up Main Windows ***

    // Create Controls

    auinotebook = new wxAuiNotebook(this, myID_AUINOTEBOOK, wxDefaultPosition, wxDefaultSize,
				    wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    auinotebook->SetArtProvider(new wxAuiSimpleTabArt);

    quickfindbar = new WQuickFindBar(this);

    quickgotobar = new WQuickGotoBar(this);

    auinotebook->AddPage(new WTextPage(this), wxT("Test wxAUI"));
    auinotebook->AddPage(new WTextPage(this), wxT("Test2 wxAUI"));

    findreplacedlg = NULL;

    // *** wxAUI Layout ***

    // tell wxAuiManager to manage this frame
    auimgr.SetManagedWindow(this);

    // add panes to the manager

    auimgr.AddPane(toolbar, wxAuiPaneInfo().
		   Name(wxT("toolbar")).Caption(_("Toolbar")).
		   ToolbarPane().Top().
		   LeftDockable(false).RightDockable(false));

    auimgr.AddPane(auinotebook, wxAuiPaneInfo().
		   Name(wxT("notebook")).Caption(_("Notebook")).
		   CenterPane().PaneBorder(false));

    auimgr.AddPane(quickfindbar, wxAuiPaneInfo().Hide().
		   Name(wxT("quickfindbar")).Caption(_("Quick-Find")).
		   CaptionVisible(false).PaneBorder(false).
		   Bottom().DockFixed().Gripper().
		   LeftDockable(false).RightDockable(false));

    auimgr.AddPane(quickgotobar, wxAuiPaneInfo().Hide().
		   Name(wxT("quickgotobar")).Caption(_("Quick-Goto")).
		   CaptionVisible(false).PaneBorder(false).
		   Bottom().DockFixed().Gripper().
		   LeftDockable(false).RightDockable(false));

    // "commit" all changes made to wxAuiManager
    auimgr.Update();

    Centre();

    if (cpage) cpage->SetFocus();
}

WCryptoTE::~WCryptoTE()
{
    auimgr.UnInit();
}

void WCryptoTE::UpdateStatusBar(const wxString& str)
{
    statusbar->SetStatusText(str);
}

void WCryptoTE::HidePane(wxWindow* child)
{
    auimgr.GetPane(child).Hide();
    auimgr.Update();
}

static inline wxMenuItem* createMenuItem(class wxMenu* parentMenu, int id,
					 const wxString& text, const wxString& helpString,
					 const wxBitmap& bmp)
{
    wxMenuItem* mi = new wxMenuItem(parentMenu, id, text, helpString);
    mi->SetBitmap(bmp);
    return mi;
}

void WCryptoTE::CreateMenuBar()
{
    toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
			    wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER);
    toolbar->SetToolBitmapSize(wxSize(16, 16));

    // *** Container

    wxMenu *menuContainer = new wxMenu;

    #include "art/document_open.h"
    #include "art/document_save.h"
    #include "art/document_saveas.h"
    #include "art/document_revert.h"
    #include "art/document_close.h"
    #include "art/application_exit.h"

    menuContainer->Append(
	createMenuItem(menuContainer, wxID_OPEN,
		       _("&Open ..\tCtrl+O"),
		       _("Open an existing encrypted container in the editor."),
		       wxBitmapFromMemory(document_open_png))
	);
    menuContainer->Append(
	createMenuItem(menuContainer, wxID_SAVE,
		       _("&Save\tCtrl+S"),
		       _("Save the current encrypted container to disk."),	
		       wxBitmapFromMemory(document_save_png))
	);
    menuContainer->Append(
	createMenuItem(menuContainer, wxID_SAVEAS,
		       _("Save &as ..\tCtrl+Shift+S"),
		       _("Choose a file name and save the current encrypted container to disk."),
		       wxBitmapFromMemory(document_saveas_png))
	);
    menuContainer->Append(
	createMenuItem(menuContainer, wxID_REVERT,
		       _("&Revert\tShift+Ctrl+W"),
		       _("Revert the current container by reloading it from disk and losing all unsaved changes."),
		       wxBitmapFromMemory(document_revert_png))
	);
    menuContainer->Append(
	createMenuItem(menuContainer, wxID_CLOSE,
		       _("&Close\tCtrl+W"),
		       _("Close the current encrypted container."),
		       wxBitmapFromMemory(document_close_png))
	);

    menuContainer->AppendSeparator();

    toolbar->AddTool(wxID_OPEN,
		     _("Open Container"),
		     wxBitmapFromMemory(document_open_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Open Container"),
		     _("Open an existing encrypted container in the editor."));

    toolbar->AddTool(wxID_SAVE,
		     _("Save Container"),
		     wxBitmapFromMemory(document_save_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Save Container"),
		     _("Save the current encrypted container to disk."));

    toolbar->AddTool(wxID_SAVEAS,
		     _("Save Container as..."),
		     wxBitmapFromMemory(document_saveas_png), wxNullBitmap, wxITEM_NORMAL,
		     _("Save Container as..."),
		     _("Choose a file name and save the current encrypted container to disk."));

    toolbar->AddSeparator();

    menuContainer->Append(
	createMenuItem(menuContainer, wxID_EXIT,
		       _("&Quit\tCtrl+Q"),
		       _("Exit CryptoTE"),
		       wxBitmapFromMemory(application_exit_png))
	);

    // *** SubFile

    wxMenu *menuSubFile = new wxMenu;

    menuSubFile->Append(
	createMenuItem(menuSubFile, myID_MENU_SUBFILE_NEW,
		       _("&New ...\tCtrl+Shift+N"),
		       _("Create new encrypted subfile in the current container."),
		       wxBitmapFromMemory(document_open_png))
	);
    menuSubFile->Append(
	createMenuItem(menuSubFile, myID_MENU_SUBFILE_IMPORT,
		       _("&Import ...\tCtrl+Shift+I"),
		       _("Import external text or data file into the current container."),
		       wxNullBitmap)
	);

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

    menuEdit->Append(
	createMenuItem(menuEdit, wxID_UNDO,
		       _("&Undo\tCtrl+Z"),
		       _("Undo the last change."),
		       wxBitmapFromMemory(edit_undo_png))
	);
    menuEdit->Append(
	createMenuItem(menuEdit, wxID_REDO,
		       _("&Redo\tCtrl+Shift+Z"),
		       _("Redo the previously undone change."),
		       wxBitmapFromMemory(edit_redo_png))
	);
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

    menuEdit->Append(
	createMenuItem(menuEdit, wxID_CUT,
		       _("Cu&t\tCtrl+X"),
		       _("Cut selected text into clipboard."),
		       wxBitmapFromMemory(edit_cut_png))
	);
    menuEdit->Append(
	createMenuItem(menuEdit, wxID_COPY,
		       _("&Copy\tCtrl+C"),
		       _("Copy selected text into clipboard."),
		       wxBitmapFromMemory(edit_copy_png))
	);
    menuEdit->Append(
	createMenuItem(menuEdit, wxID_PASTE,
		       _("&Paste\tCtrl+V"),
		       _("Paste clipboard contents at the current text position."),
		       wxBitmapFromMemory(edit_paste_png))
	);
    menuEdit->Append(
	createMenuItem(menuEdit, wxID_CLEAR,
		       _("&Delete\tDel"),
		       _("Delete selected text."),
		       wxBitmapFromMemory(edit_clear_png))
	);
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

    menuEdit->Append(
	createMenuItem(menuEdit, myID_MENU_EDIT_QUICKFIND,
		       _("&Quick-Find\tCtrl+F"),
		       _("Find a string in the buffer."),
		       wxBitmapFromMemory(edit_find_png))
	);
    menuEdit->Append(
	createMenuItem(menuEdit, wxID_FIND,
		       _("&Find...\tCtrl+Shift+F"),
		       _("Open find dialog to search for a string in the buffer."),
		       wxBitmapFromMemory(edit_find_png))
	);
    menuEdit->Append(
	createMenuItem(menuEdit, wxID_REPLACE,
		       _("&Replace\tCtrl+H"),
		       _("Open find and replace dialog to search for and replace a string in the buffer."),
		       wxBitmapFromMemory(edit_find_png))
	);
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

    menuEdit->Append(
	createMenuItem(menuEdit, myID_MENU_EDIT_GOTO,
		       _("&Go to Line...\tCtrl+G"),
		       _("Jump to the entered line number."),
		       wxBitmapFromMemory(go_next_png))
	);
    menuEdit->AppendSeparator();


    menuEdit->Append(wxID_SELECTALL,
		     _("&Select all\tCtrl+A"),
		     _("Select all text in the current buffer."));

    menuEdit->Append(myID_MENU_EDIT_SELECTLINE,
		     _("Select &line\tCtrl+L"),
		     _("Select whole line at the current cursor position."));

    // *** View

    wxMenu *menuView = new wxMenu;

    menuView->AppendCheckItem(myID_MENU_VIEW_LINEWRAP,
			      _("&Wrap long lines"),
			      _("Wrap long lines in editor."));

    menuView->AppendCheckItem(myID_MENU_VIEW_LINENUMBER,
			      _("Show line &numbers"),
			      _("Show line numbers on left margin."));

    menuView->AppendCheckItem(myID_MENU_VIEW_WHITESPACE,
			      _("Show white&space"),
			      _("Show white space (space and tab) in buffer."));

    menuView->AppendCheckItem(myID_MENU_VIEW_ENDOFLINE,
			      _("Show &end of line symbols"),
			      _("Show end of line symbols (new-line and carriage-return) in buffer."));

    menuView->AppendCheckItem(myID_MENU_VIEW_INDENTGUIDE,
			      _("Show &indent guide lines"),
			      _("Show guide lines following the indention depth."));

    menuView->AppendCheckItem(myID_MENU_VIEW_LONGLINEGUIDE,
			      _("Show guide line at &column 80"),
			      _("Show guide line at column 80 to display over-long lines."));

    // *** Help

    wxMenu *menuHelp = new wxMenu;

    #include "art/application_info.h"

    menuHelp->Append(
	createMenuItem(menuHelp, wxID_ABOUT,
		       _("&About\tShift+F1"),
		       _("Show some information about CryptoTE."),
		       wxBitmapFromMemory(application_info_png))
	);

    // construct menubar and add it to the window
    menubar = new wxMenuBar;

    menubar->Append(menuContainer, _("&Container"));
    menubar->Append(menuSubFile, _("&SubFile"));
    menubar->Append(menuEdit, _("&Edit"));
    menubar->Append(menuView, _("&View"));
    menubar->Append(menuHelp, _("&Help"));

    SetMenuBar(menubar);
    toolbar->Realize();
}

// *** Menu Events ***

void WCryptoTE::OnMenuContainerOpen(wxCommandEvent& WXUNUSED(event))
{
}

void WCryptoTE::OnMenuContainerSave(wxCommandEvent& WXUNUSED(event))
{
}

void WCryptoTE::OnMenuContainerSaveAs(wxCommandEvent& WXUNUSED(event))
{
}

void WCryptoTE::OnMenuContainerRevert(wxCommandEvent& WXUNUSED(event))
{
}

void WCryptoTE::OnMenuContainerClose(wxCommandEvent& WXUNUSED(event))
{
}

void WCryptoTE::OnMenuContainerQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void WCryptoTE::OnMenuSubFileNew(wxCommandEvent& WXUNUSED(event))
{
    auinotebook->AddPage(new WTextPage(this), wxT("Test wxAUI"), true);
}

void WCryptoTE::OnMenuSubFileImport(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dlg(this,
		     _("Import file"), wxEmptyString, wxEmptyString, _("Any file (*)|*"),
#if wxCHECK_VERSION(2,8,0)
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
#else
                     wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR);
#endif

    if (dlg.ShowModal() != wxID_OK) return;

}

void WCryptoTE::OnMenuEditGeneric(wxCommandEvent& event)
{
    // This is actually very dangerous: the page window MUST process the event
    // otherwise it will be passed up to the parent window, and will be caught
    // by this event handler and passed down again, creating an infinite loop.

    if (cpage) cpage->ProcessEvent(event);
}

void WCryptoTE::OnMenuEditQuickFind(wxCommandEvent& WXUNUSED(event))
{
    if (!cpage) return;

    if (quickfindbar_visible)
    {
	// pushing Ctrl+F again is equivalent to Search-Next

	quickfindbar->textctrlQuickFind->SetFocus();
	
	wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

	cpage->PrepareQuickFind(false, false);

	cpage->DoQuickFind(false, findtext);
    }
    else
    {
	// make Quick-Find bar visible

	auimgr.GetPane(quickfindbar).Show();
	auimgr.GetPane(quickgotobar).Hide();
	auimgr.Update();

	quickgotobar_visible = false;
	quickfindbar_visible = true;

	cpage->PrepareQuickFind(false, true);

	quickfindbar->textctrlQuickFind->SetFocus();
	quickfindbar->textctrlQuickFind->SetValue(wxT(""));
    }
}

void WCryptoTE::OnMenuEditGoto(wxCommandEvent& WXUNUSED(event))
{
    if (!quickgotobar_visible)
    {
	auimgr.GetPane(quickgotobar).Show();
	auimgr.GetPane(quickfindbar).Hide();
	auimgr.Update();

	quickfindbar_visible = false;
	quickgotobar_visible = true;

	quickgotobar->textctrlGoto->SetFocus();
	quickfindbar->textctrlQuickFind->SetValue(wxT(""));
    }
    else
    {
	quickgotobar->textctrlGoto->SetFocus();
    }
}

void WCryptoTE::OnMenuViewLineWrap(wxCommandEvent& event)
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetViewLineWrap(event.IsChecked());
}

void WCryptoTE::OnMenuViewLineNumber(wxCommandEvent& event)
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetViewLineNumber(event.IsChecked());
}

void WCryptoTE::OnMenuViewWhitespace(wxCommandEvent& event)
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetViewWhitespace(event.IsChecked());
}

void WCryptoTE::OnMenuViewEndOfLine(wxCommandEvent& event)
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetViewEndOfLine(event.IsChecked());
}

void WCryptoTE::OnMenuViewIndentGuide(wxCommandEvent& event)
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetViewIndentGuide(event.IsChecked());
}

void WCryptoTE::OnMenuViewLonglineGuide(wxCommandEvent& event)
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetViewLonglineGuide(event.IsChecked());
}

void WCryptoTE::OnMenuHelpAbout(wxCommandEvent& WXUNUSED(event))
{
    WAbout dlg(this);
    dlg.ShowModal();
}

void WCryptoTE::OnAccelEscape(wxCommandEvent& WXUNUSED(event))
{
    // Hide Quick-Find Bar
    if (quickfindbar_visible) {
	auimgr.GetPane(quickfindbar).Hide();
	auimgr.Update();

	quickfindbar_visible = false;
    }

    // Hide Quick-Goto Bar
    if (quickgotobar_visible) {
	auimgr.GetPane(quickgotobar).Hide();
	auimgr.Update();

	quickgotobar_visible = false;
    }

    if (cpage) cpage->SetFocus();
}

// *** wxAuiNotebook Callbacks ***

void WCryptoTE::OnNotebookPageChanged(wxAuiNotebookEvent& event)
{
    wxWindow* sel = auinotebook->GetPage( event.GetSelection() );

    if (sel && sel->IsKindOf(CLASSINFO(WNotePage)))
    {
	if (cpage) cpage->PageBlurred();

	printf("page changed: %d\n", event.GetSelection());
	cpage = (WNotePage*)sel;

	cpage->PageFocused();
    }
    else {
	wxLogError(_T("Invalid notebook page activated."));
    }
}

void WCryptoTE::OnNotebookPageClose(wxAuiNotebookEvent& event)
{
    printf("page close: %d - %d\n", event.GetSelection(), auinotebook->GetPageCount());

    if (auinotebook->GetPageCount() == 1) {
	// will be empty after the last page is closed
	cpage = NULL;
    }
}

// *** WQuickFindBar Callbacks ***

void WCryptoTE::OnButtonQuickFindClose(wxCommandEvent& WXUNUSED(event))
{
    if (quickfindbar_visible)
    {
	auimgr.GetPane(quickfindbar).Hide();
	auimgr.Update();

	quickfindbar_visible = false;

	if (cpage) cpage->SetFocus();
    }
}

void WCryptoTE::OnTextQuickFind(wxCommandEvent& WXUNUSED(event))
{
    if (cpage)
    {
	wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

	cpage->DoQuickFind(false, findtext);
    }
}

void WCryptoTE::OnButtonQuickFindNext(wxCommandEvent& WXUNUSED(event))
{
    if (cpage)
    {
	wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

	cpage->PrepareQuickFind(false, false);

	cpage->DoQuickFind(false, findtext);
    }
}

void WCryptoTE::OnButtonQuickFindPrev(wxCommandEvent& WXUNUSED(event))
{
    if (cpage)
    {
	wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

	cpage->PrepareQuickFind(true, false);

	cpage->DoQuickFind(true, findtext);
    }
}
// *** WQuickGotoBar Callbacks ***

void WCryptoTE::OnButtonGotoClose(wxCommandEvent& WXUNUSED(event))
{
    if (quickgotobar_visible)
    {
	auimgr.GetPane(quickgotobar).Hide();
	auimgr.Update();

	quickgotobar_visible = false;

	if (cpage) cpage->SetFocus();
    }
}

void WCryptoTE::OnButtonGotoGo(wxCommandEvent& WXUNUSED(event))
{
    if (cpage)
    {
	bool r = cpage->DoQuickGoto( quickgotobar->textctrlGoto->GetValue() );

	if (!r) {
	    quickgotobar->textctrlGoto->SetFocus();
	}
	else {
	    auimgr.GetPane(quickgotobar).Hide();
	    auimgr.Update();

	    quickgotobar_visible = false;
	    
	    cpage->SetFocus();
	}
    }
}

void WCryptoTE::OnMenuEditFind(wxCommandEvent& WXUNUSED(event))
{
    if (!findreplacedlg)
    {
	findreplacedlg = new WFindReplace(this);

	auimgr.AddPane(findreplacedlg, wxAuiPaneInfo().
		       Name(wxT("findreplacedlg")).
		       Dockable(false).Float());
    }

    findreplacedlg->ShowReplace(false);

    auimgr.GetPane(findreplacedlg).Show().Caption(_("Find"));
    auimgr.Update();
}

void WCryptoTE::OnMenuEditFindReplace(wxCommandEvent& WXUNUSED(event))
{
    if (!findreplacedlg)
    {
	findreplacedlg = new WFindReplace(this);

	auimgr.AddPane(findreplacedlg, wxAuiPaneInfo().
		       Name(wxT("findreplacedlg")).
		       Dockable(false).Float());
    }

    findreplacedlg->ShowReplace(true);

    auimgr.GetPane(findreplacedlg).Show().Caption(_("Find & Replace"));
    auimgr.Update();
}

BEGIN_EVENT_TABLE(WCryptoTE, wxFrame)

    // *** Menu Items

    // Container
    EVT_MENU	(wxID_OPEN,		WCryptoTE::OnMenuContainerOpen)
    EVT_MENU	(wxID_SAVE,		WCryptoTE::OnMenuContainerSave)
    EVT_MENU	(wxID_SAVEAS,		WCryptoTE::OnMenuContainerSaveAs)
    EVT_MENU	(wxID_REVERT,		WCryptoTE::OnMenuContainerRevert)
    EVT_MENU	(wxID_CLOSE,		WCryptoTE::OnMenuContainerClose)

    EVT_MENU	(wxID_EXIT,		WCryptoTE::OnMenuContainerQuit)

    // SubFile
    EVT_MENU	(myID_MENU_SUBFILE_NEW, WCryptoTE::OnMenuSubFileNew)
    EVT_MENU	(myID_MENU_SUBFILE_IMPORT, WCryptoTE::OnMenuSubFileImport)

    // Edit
    EVT_MENU	(wxID_UNDO,		WCryptoTE::OnMenuEditGeneric)
    EVT_MENU	(wxID_REDO,		WCryptoTE::OnMenuEditGeneric)

    EVT_MENU	(wxID_CUT,		WCryptoTE::OnMenuEditGeneric)
    EVT_MENU	(wxID_COPY,		WCryptoTE::OnMenuEditGeneric)
    EVT_MENU	(wxID_PASTE,		WCryptoTE::OnMenuEditGeneric)
    EVT_MENU	(wxID_CLEAR,		WCryptoTE::OnMenuEditGeneric)

    EVT_MENU	(myID_MENU_EDIT_QUICKFIND, WCryptoTE::OnMenuEditQuickFind)
    EVT_MENU	(wxID_FIND,		WCryptoTE::OnMenuEditFind)
    EVT_MENU	(wxID_REPLACE,		WCryptoTE::OnMenuEditFindReplace)

    EVT_MENU	(myID_MENU_EDIT_GOTO,	WCryptoTE::OnMenuEditGoto)

    EVT_MENU	(wxID_SELECTALL,	WCryptoTE::OnMenuEditGeneric)
    EVT_MENU	(myID_MENU_EDIT_SELECTLINE, WCryptoTE::OnMenuEditGeneric)

    // View
    EVT_MENU	(myID_MENU_VIEW_LINEWRAP,	WCryptoTE::OnMenuViewLineWrap)
    EVT_MENU	(myID_MENU_VIEW_LINENUMBER,	WCryptoTE::OnMenuViewLineNumber)
    EVT_MENU	(myID_MENU_VIEW_WHITESPACE,	WCryptoTE::OnMenuViewWhitespace)
    EVT_MENU	(myID_MENU_VIEW_ENDOFLINE,	WCryptoTE::OnMenuViewEndOfLine)
    EVT_MENU	(myID_MENU_VIEW_INDENTGUIDE,	WCryptoTE::OnMenuViewIndentGuide)
    EVT_MENU	(myID_MENU_VIEW_LONGLINEGUIDE,	WCryptoTE::OnMenuViewLonglineGuide)

    // Help
    EVT_MENU	(wxID_ABOUT,		WCryptoTE::OnMenuHelpAbout)

    // *** Accelerators

    EVT_MENU	(myID_ACCEL_ESCAPE,	WCryptoTE::OnAccelEscape)

    // *** wxAuiNotebook Callbacks

    EVT_AUINOTEBOOK_PAGE_CHANGED(myID_AUINOTEBOOK, WCryptoTE::OnNotebookPageChanged)
    EVT_AUINOTEBOOK_PAGE_CLOSE(myID_AUINOTEBOOK, WCryptoTE::OnNotebookPageClose)

    // *** Quick-Find Bar

    EVT_TEXT	(myID_QUICKFIND_TEXT,	WCryptoTE::OnTextQuickFind)

    EVT_BUTTON	(myID_QUICKFIND_NEXT,	WCryptoTE::OnButtonQuickFindNext)
    EVT_BUTTON	(myID_QUICKFIND_PREV,	WCryptoTE::OnButtonQuickFindPrev)
    EVT_BUTTON	(myID_QUICKFIND_CLOSE,	WCryptoTE::OnButtonQuickFindClose)

    // *** Quick-Goto Bar

    EVT_TEXT_ENTER(myID_QUICKGOTO_TEXT,	WCryptoTE::OnButtonGotoGo)
    EVT_BUTTON	(myID_QUICKGOTO_GO,	WCryptoTE::OnButtonGotoGo)
    EVT_BUTTON	(myID_QUICKGOTO_CLOSE,	WCryptoTE::OnButtonGotoClose)

END_EVENT_TABLE()

#if 0
/*****************************************************************************/


    SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE) );

    // Default Settings

    editctrl->FileNew();

    UpdateTitle();
}

void WCryptoTE::UpdateTitle()
{
    wxString title = editctrl->GetFileBasename();

    if (editctrl->ModifiedFlag()) {
	title += _(" (Modified)");
    }

    title += _(" - ");
    title += _("CryptoTE");

    SetTitle(title);
}

bool WCryptoTE::FileOpen(const wxString& filename)
{
    bool b = editctrl->FileOpen(filename);

    UpdateTitle();
    return b;
}

bool WCryptoTE::FileSave()
{
    if (!editctrl->HasFilename()) {
	return FileSaveAs();
    }

    return editctrl->FileSave();
}

bool WCryptoTE::FileSaveAs()
{
    wxFileDialog dlg(this,
		     _("Save file"), wxEmptyString, editctrl->GetFileBasename(), _("Any file (*)|*"),
#if wxCHECK_VERSION(2,8,0)
		     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
#else
		     wxSAVE | wxOVERWRITE_PROMPT);
#endif

    if (dlg.ShowModal() != wxID_OK) return false;

    editctrl->FileSaveAs( dlg.GetPath() );

    return true;
}

void WCryptoTE::CreateMenuBar()
{
}

void WCryptoTE::OnClose(wxCloseEvent& event)
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

bool WCryptoTE::AllowCloseModified()
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

void WCryptoTE::OnMenuFileOpen(wxCommandEvent& WXUNUSED(event))
{
    if (!AllowCloseModified()) return;

    wxFileDialog dlg(this,
		     _("Open file"), wxEmptyString, wxEmptyString, _("Any file (*)|*"),
#if wxCHECK_VERSION(2,8,0)
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
#else
                     wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR);
#endif

    if (dlg.ShowModal() != wxID_OK) return;

    editctrl->FileOpen( dlg.GetPath() );

    UpdateTitle();
}

void WCryptoTE::OnMenuFileSave(wxCommandEvent& WXUNUSED(event))
{
    FileSave();
}

void WCryptoTE::OnMenuFileSaveAs(wxCommandEvent& WXUNUSED(event))
{
    FileSaveAs();
}

void WCryptoTE::OnMenuFileRevert(wxCommandEvent& WXUNUSED(event))
{
    editctrl->FileRevert();
}

void WCryptoTE::OnMenuFileClose(wxCommandEvent& WXUNUSED(event))
{
    if (!AllowCloseModified()) return;

    editctrl->FileNew();
}

BEGIN_EVENT_TABLE(WCryptoTE, wxFrame)

    // *** Generic Events

    EVT_CLOSE	(WCryptoTE::OnClose)

END_EVENT_TABLE()

/*****************************************************************************/
#endif

// *** WStatusBar ***

WStatusBar::WStatusBar(wxWindow *parent)
    : wxStatusBar(parent, wxID_ANY, 0)
{

    static const int statusbar_widths[3] = { -1, 150, 28 };

    SetFieldsCount(3);
    SetStatusWidths(3, statusbar_widths);

    #include "art/stock_lock.h"

    lockbitmap = new wxStaticBitmap(this, wxID_ANY, wxIconFromMemory(stock_lock_png));
}

void WStatusBar::OnSize(wxSizeEvent& event)
{
    // move bitmap to position
    wxRect rect;
    GetFieldRect(2, rect);
    wxSize size = lockbitmap->GetSize();

    lockbitmap->Move(rect.x + (rect.width - size.x) / 2,
		     rect.y + (rect.height - size.y) / 2);

    event.Skip();
}

void WStatusBar::SetLock(bool on)
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

BEGIN_EVENT_TABLE(WStatusBar, wxStatusBar)

    EVT_SIZE	(WStatusBar::OnSize)

END_EVENT_TABLE();

// *** WAbout ***

WAbout::WAbout(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long WXUNUSED(style))
    : wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
{
    // begin wxGlade: WAbout::WAbout
    bitmapIcon = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    bitmapWeb = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    hyperlink1 = new wxHyperlinkCtrl(this, wxID_ANY, _("Visit http://idlebox.net/2008/cryptote/"), _("http://idlebox.net/2008/cryptote/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT);
    buttonOK = new wxButton(this, wxID_OK, wxEmptyString);

    set_properties();
    do_layout();
    // end wxGlade

    #include "art/cryptote-48.h"
    bitmapIcon->SetBitmap( wxBitmapFromMemory(cryptote_48_png) );

    #include "art/web-16.h"
    bitmapWeb->SetBitmap( wxBitmapFromMemory(web_16_png) );

    Layout();
    GetSizer()->Fit(this);
    Centre();
}

void WAbout::set_properties()
{
    // begin wxGlade: WAbout::set_properties
    SetTitle(_("About CryptoTE"));
    // end wxGlade
}

void WAbout::do_layout()
{
    // begin wxGlade: WAbout::do_layout
    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizer3 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizer5 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(bitmapIcon, 0, wxALL, 8);
    wxStaticText* label1 = new wxStaticText(this, wxID_ANY, _("CryptoTE v0.1"));
    label1->SetFont(wxFont(18, wxDEFAULT, wxNORMAL, wxBOLD, 0, wxT("")));
    sizer3->Add(label1, 0, wxALL, 6);
    wxStaticText* label2 = new wxStaticText(this, wxID_ANY, _("CryptoTE is a text editor built upon the popular\nScintilla editing widget. Text is saved encrypted\nand compressed to secure sensitive data."));
    sizer3->Add(label2, 0, wxALL, 6);
    wxStaticText* label4 = new wxStaticText(this, wxID_ANY, _("Copyright 2008 Timo Bingmann\nReleased under the GNU General Public License v2"));
    sizer3->Add(label4, 0, wxALL, 6);
    sizer4->Add(bitmapWeb, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    sizer5->Add(hyperlink1, 1, wxEXPAND, 0);
    wxStaticText* label5 = new wxStaticText(this, wxID_ANY, _("for updates and more."));
    sizer5->Add(label5, 0, wxALL, 0);
    sizer4->Add(sizer5, 1, wxEXPAND, 0);
    sizer3->Add(sizer4, 0, wxALL|wxEXPAND, 6);
    sizer2->Add(sizer3, 1, wxEXPAND, 0);
    sizer1->Add(sizer2, 1, wxALL|wxEXPAND, 6);
    sizer1->Add(buttonOK, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 6);
    SetSizer(sizer1);
    sizer1->Fit(this);
    Layout();
    // end wxGlade
}

// *** WNotePage ***

WNotePage::WNotePage(class WCryptoTE* _wmain)
    : wxPanel(_wmain),
      wmain(_wmain)
{
}

void WNotePage::UpdateStatusBar(const wxString& str)
{
    wmain->UpdateStatusBar(str);
}

IMPLEMENT_ABSTRACT_CLASS(WNotePage, wxPanel);
