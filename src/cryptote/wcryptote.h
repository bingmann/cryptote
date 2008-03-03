// $Id$

#ifndef WCRYPTOTE_H
#define WCRYPTOTE_H

#include <wx/wx.h>
#include <wx/aui/aui.h>

#if wxCHECK_VERSION(2,8,0)
#include <wx/hyperlink.h>
#else
#include "common/hyperlink.h"
#endif

class WCryptoTE : public wxFrame
{
public:
    WCryptoTE(wxWindow* parent);
    ~WCryptoTE();

    // *** Identifiers ***

    enum ids {
	myID_FIRST = wxID_HIGHEST + 1,

	// Main Window Controls

	myID_AUINOTEBOOK,

	// Menu Items

	myID_MENU_SUBFILE_NEW,
	myID_MENU_SUBFILE_IMPORT,

	myID_MENU_EDIT_QUICKFIND,
	myID_MENU_EDIT_GOTO,
	myID_MENU_EDIT_SELECTLINE,

	myID_MENU_VIEW_LINEWRAP,
	myID_MENU_VIEW_LINENUMBER,
	myID_MENU_VIEW_WHITESPACE,	
	myID_MENU_VIEW_ENDOFLINE,
	myID_MENU_VIEW_INDENTGUIDE,
	myID_MENU_VIEW_LONGLINEGUIDE,

	// (Other) Accelerators

	myID_ACCEL_ESCAPE,

	// Quick-Find Bar

	myID_QUICKFIND_TEXT,
	myID_QUICKFIND_CLOSE,
	myID_QUICKFIND_NEXT,
	myID_QUICKFIND_PREV,

	// Quick-Goto Bar

	myID_QUICKGOTO_TEXT,
	myID_QUICKGOTO_GO,
	myID_QUICKGOTO_CLOSE,
    };

    // *** Operations ***

    /// Temporarily set status bar to show given text.
    void	UpdateStatusBar(const wxString& str);

    /// Callback from any subdialog to hide it's pane 
    void	HidePane(wxWindow* child);

    // *** Event Handlers ***

    // Menu Events

    void	OnMenuContainerOpen(wxCommandEvent& event);
    void	OnMenuContainerSave(wxCommandEvent& event);
    void	OnMenuContainerSaveAs(wxCommandEvent& event);
    void	OnMenuContainerRevert(wxCommandEvent& event);
    void	OnMenuContainerClose(wxCommandEvent& event);

    void	OnMenuContainerQuit(wxCommandEvent& event);

    void	OnMenuSubFileNew(wxCommandEvent& event);
    void	OnMenuSubFileImport(wxCommandEvent& event);

    void	OnMenuEditGeneric(wxCommandEvent& event);
    void	OnMenuEditQuickFind(wxCommandEvent& event);
    void	OnMenuEditGoto(wxCommandEvent& event);
    void	OnMenuEditFind(wxCommandEvent& event);
    void	OnMenuEditFindReplace(wxCommandEvent& event);

    void	OnMenuViewLineWrap(wxCommandEvent& event);
    void	OnMenuViewLineNumber(wxCommandEvent& event);
    void	OnMenuViewWhitespace(wxCommandEvent& event);
    void	OnMenuViewEndOfLine(wxCommandEvent& event);
    void	OnMenuViewIndentGuide(wxCommandEvent& event);
    void	OnMenuViewLonglineGuide(wxCommandEvent& event);

    void	OnMenuHelpAbout(wxCommandEvent& event);

    // Accelerator Events

    void	OnAccelEscape(wxCommandEvent& event);

    // wxAuiNotebook Callbacks

    void	OnNotebookPageChanged(wxAuiNotebookEvent& event);
    void	OnNotebookPageClose(wxAuiNotebookEvent& event);

    // Quick-Find Bar

    void	OnTextQuickFind(wxCommandEvent& event);

    void	OnButtonQuickFindNext(wxCommandEvent& event);
    void	OnButtonQuickFindPrev(wxCommandEvent& event);
    void	OnButtonQuickFindClose(wxCommandEvent& event);

    // Quick-Goto Bar

    void	OnButtonGotoGo(wxCommandEvent& event);
    void	OnButtonGotoClose(wxCommandEvent& event);

protected:

    // *** Menu, Tool and Status Bars of the Main Window ***

public:
    class wxMenuBar*	menubar;
    class wxToolBar*	toolbar;
    class WStatusBar*	statusbar;

    void 		CreateMenuBar();

protected:
    // *** wxAUI Window Manager ***

    wxAuiManager 	auimgr;

    // *** Displayed or Hidden Panes ***

    wxAuiNotebook*	auinotebook;

public:

    /// Currently selected notebook page (or NULL).
    class WNotePage*	cpage;

    /// Quick-Find Bar activated with Ctrl+F
    class WQuickFindBar* quickfindbar;
    bool		quickfindbar_visible;

    /// Quick-Goto Bar activated with Ctrl+G
    class WQuickGotoBar* quickgotobar;
    bool		quickgotobar_visible;

    /// (Slow) Find & Replace Dialog activated with Ctrl+Shift+F
    class WFindReplace*	findreplacedlg;

#if 0
/*****************************************************************************/

    /// Update the title bar with the currently loaded text file name
    void	UpdateTitle();

     /// Load a file into the editor, discard any current buffer.
    bool	FileOpen(const wxString& filename);

    /// Save the buffer into the associated file, query for a file name if none
    /// is associated.
    bool	FileSave();

    /// Query for a file name and save the buffer into the associated file.
    bool	FileSaveAs();

    /// Enable or Disable Save and SaveAs depending on if the buffer is modified.
    void	UpdateOnSavePoint();

    /// Ture if the user is allowed to close the window.
    bool	AllowCloseModified();

    // *** Event Handlers ***

    // Generic Events

    void	OnChar(wxKeyEvent& event);

    void	OnClose(wxCloseEvent& event);

    // Menu Events

    void	OnMenuFileOpen(wxCommandEvent& event);
    void	OnMenuFileSave(wxCommandEvent& event);
    void	OnMenuFileSaveAs(wxCommandEvent& event);
    void	OnMenuFileRevert(wxCommandEvent& event);
    void	OnMenuFileClose(wxCommandEvent& event);



/*****************************************************************************/
#endif

private:
    DECLARE_EVENT_TABLE()
};

class WStatusBar : public wxStatusBar
{
public:
    WStatusBar(wxWindow *parent);

    void		OnSize(wxSizeEvent& event);
    void		SetLock(bool on);

    wxStaticBitmap*	lockbitmap;

private:
    DECLARE_EVENT_TABLE()
};

class WAbout : public wxDialog
{
public:
    // begin wxGlade: WAbout::ids
    // end wxGlade

    WAbout(wxWindow* parent, int id=wxID_ANY, const wxString& title=wxEmptyString, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE);

private:
    // begin wxGlade: WAbout::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: WAbout::attributes
    wxStaticBitmap* bitmapIcon;
    wxStaticBitmap* bitmapWeb;
    wxHyperlinkCtrl* hyperlink1;
    wxButton* buttonOK;
    // end wxGlade
}; // wxGlade: end class

class WNotePage : public wxPanel
{
protected:

    WNotePage(class WCryptoTE* _wmain);

    /// Reference to parent window class
    class WCryptoTE*	wmain;

    /// Temporarily set status bar to show given text. Just forwards via wmain.
    void	UpdateStatusBar(const wxString& str);

public:

    /// Return the text to display in the notebook
    virtual wxString	GetCaption() = 0;

    /// Called when the notebook page is activated/focused.
    virtual void	PageFocused() = 0;

    /// Called when the notebook page is deactivated.
    virtual void	PageBlurred() = 0;

    /// Prepare for a Quick-Find by setting the search anchor point, backwards
    /// for searching backwards and reset for terminating incremental search
    /// and restarting.
    virtual void	PrepareQuickFind(bool backwards, bool reset) = 0;

    /// Execute Quick-Find for a search string.
    virtual void	DoQuickFind(bool backwards, const wxString& findtext) = 0;

    /// Execute Quick-Goto for a given string or set an error message. The goto
    /// window will close if the function returns true.
    virtual bool	DoQuickGoto(const wxString& gototext) = 0;

    DECLARE_ABSTRACT_CLASS(WNotePage);
};

#endif // WCRYPTOTE_H