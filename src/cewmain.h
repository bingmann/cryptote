// $Id$

#ifndef CEWMAIN_H
#define CEWMAIN_H

#include <wx/wx.h>

#if wxCHECK_VERSION(2,8,0)
#include <wx/hyperlink.h>
#else
#include "common/hyperlink.h"
#endif

class CEWMain : public wxFrame
{
public:
    CEWMain(wxWindow* parent);

    // *** Identifiers ***

    enum {
	myID_EDITCTRL = wxID_HIGHEST + 1,
	myID_MENU_SELECTLINE,

	myID_QUICKFIND,
	myID_QUICKFIND_TEXT,
	myID_QUICKFIND_CLOSE,
	myID_QUICKFIND_NEXT,
	myID_QUICKFIND_PREV,

	myID_GOTO,
	myID_GOTOTEXT,
	myID_GOTO_GO,
	myID_GOTO_CLOSE,

	myID_MENU_LINEWRAP,
	myID_MENU_LINENUMBER,
	myID_MENU_WHITESPACE,	
	myID_MENU_ENDOFLINE,
	myID_MENU_INDENTGUIDE,
	myID_MENU_LONGLINEGUIDE
    };

    // *** Operations ***

    /// Update the title bar with the currently loaded text file name
    void	UpdateTitle();

    /// Update status bar to show given text.
    void	UpdateStatusBar(const wxString& str);

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

    void	OnMenuFileQuit(wxCommandEvent& event);

    void	OnMenuEditGeneric(wxCommandEvent& event);

    void	OnMenuEditQuickFind(wxCommandEvent& event);
    void	OnMenuEditFind(wxCommandEvent& event);
    void	OnMenuEditFindReplace(wxCommandEvent& event);

    void	OnMenuEditGoto(wxCommandEvent& event);

    void	OnMenuViewLineWrap(wxCommandEvent& event);
    void	OnMenuViewLineNumber(wxCommandEvent& event);
    void	OnMenuViewWhitespace(wxCommandEvent& event);
    void	OnMenuViewEndOfLine(wxCommandEvent& event);
    void	OnMenuViewIndentGuide(wxCommandEvent& event);
    void	OnMenuViewLonglineGuide(wxCommandEvent& event);

    void	OnMenuHelpAbout(wxCommandEvent& event);

    // Scintilla Callbacks

    void	OnScintillaUpdateUI(class wxStyledTextEvent& event);
    void	OnScintillaSavePointReached(class wxStyledTextEvent& event);
    void	OnScintillaSavePointLeft(class wxStyledTextEvent& event);

    // Quick-Find Bar

    void	OnTextQuickFind(wxCommandEvent& event);

    void	OnButtonQuickFindNext(wxCommandEvent& event);
    void	OnButtonQuickFindPrev(wxCommandEvent& event);
    void	OnButtonQuickFindClose(wxCommandEvent& event);

    // Quick-Goto Bar

    void	OnButtonGotoGo(wxCommandEvent& event);
    void	OnButtonGotoClose(wxCommandEvent& event);

protected:

    wxBoxSizer*	sizerMain;

    // *** Menu and Status Bars of the main window ***

    wxMenuBar*	menubar;
    wxToolBar*	toolbar;

    void 	CreateMenuBar();

    class CEWStatusBar* statusbar;

public:
    // *** Styled Text Edit control ***

    class CEWEditCtrl*	editctrl;

protected:
    // *** Quick-Find Popup Bar ***

    class wxBoxSizer*	sizerQuickFind;
    class wxTextCtrl*	textctrlQuickFind;

    bool		quickfind_visible;
    int			quickfind_startpos;

    void		QuickFind(bool forward);

    // *** Quick-Goto Popup Bar ***
    class wxBoxSizer*	sizerQuickGoto;
    class wxTextCtrl*	textctrlGoto;

    bool		quickgoto_visible;

    // *** Current Modeless Dialogs ***

    class CEWFind*	findreplace_dlg;

private:
    DECLARE_EVENT_TABLE()
};

class CEWStatusBar : public wxStatusBar
{
public:
    CEWStatusBar(wxWindow *parent);

    void		OnSize(wxSizeEvent& event);
    void		SetLock(bool on);

    wxStaticBitmap*	lockbitmap;

private:
    DECLARE_EVENT_TABLE()
};

class CEWAbout : public wxDialog
{
public:
    // begin wxGlade: CEWAbout::ids
    // end wxGlade

    CEWAbout(wxWindow* parent, int id=wxID_ANY, const wxString& title=wxEmptyString, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE);

private:
    // begin wxGlade: CEWAbout::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: CEWAbout::attributes
    wxStaticBitmap* bitmapIcon;
    wxStaticBitmap* bitmapWeb;
    wxHyperlinkCtrl* hyperlink1;
    wxButton* buttonOK;
    // end wxGlade
}; // wxGlade: end class

#endif // CEWMAIN_H
