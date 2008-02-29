// $Id$

#ifndef WTEXTPAGE_H
#define WTEXTPAGE_H

#include <wx/wx.h>
#include <wx/filename.h>

#include "wcryptote.h"

class WTextPage : public WNotePage
{
public:

    WTextPage(class WCryptoTE* parent);

    /// Return the text to display in the notebook
    virtual wxString	GetCaption();

    // *** Variables ***

    /// True if buffer modified in editor
    bool		modified;

    // *** Identifiers ***

    enum {
	myID_FIRST = wxID_HIGHEST + 1,
	myID_EDITCTRL,
    };

    /// Margins Enumeration
    enum margin_num {
	MARGIN_LINENUMBER = 0
    };

    // *** Operations ***

    /// Enable or Disable Save and SaveAs depending on if the buffer is modified.
    void	UpdateOnSavePoint();

    // *** Event Handlers ***

    // Edit Menu
    void	OnMenuEditUndo(wxCommandEvent& event);
    void	OnMenuEditRedo(wxCommandEvent& event);

    void	OnMenuEditCut(wxCommandEvent& event);
    void	OnMenuEditCopy(wxCommandEvent& event);
    void	OnMenuEditPaste(wxCommandEvent& event);
    void	OnMenuEditDelete(wxCommandEvent& event);

    void	OnMenuEditSelectAll(wxCommandEvent &event);
    void	OnMenuEditSelectLine(wxCommandEvent &event);

    // *** Virtual Callbacks via WNotePage ***

    /// Called when the notebook page is activated/focused.
    virtual void	PageFocused();

    /// Called when the notebook page is deactivated.
    virtual void	PageBlurred();

    virtual void PrepareQuickFind(bool backwards, bool reset);
    virtual void DoQuickFind(bool backwards, const wxString& findtext);
    virtual bool DoQuickGoto(const wxString& gototext);

    // *** Scintilla Callbacks ***

    void	OnScintillaUpdateUI(class wxStyledTextEvent& event);
    void	OnScintillaSavePointReached(class wxStyledTextEvent& event);
    void	OnScintillaSavePointLeft(class wxStyledTextEvent& event);

    // *** Control ***

    /// The Scintilla edit control
    class wxStyledTextCtrl*	editctrl;

    // *** Set/Get View Options ***

    void	SetViewLineWrap(bool on);
    bool	GetViewLineWrap();

    void	SetViewLineNumber(bool on);
    bool	GetViewLineNumber();

    void	SetViewWhitespace(bool on);
    bool	GetViewWhitespace();

    void	SetViewEndOfLine(bool on);
    bool	GetViewEndOfLine();

    void	SetViewIndentGuide(bool on);
    bool	GetViewIndentGuide();
    
    void	SetViewLonglineGuide(bool on);
    bool	GetViewLonglineGuide();

protected:

    /// Current starting position for incremental Quick-Find
    int			quickfind_startpos;

    /// Settings of current View Options

    bool	view_linewrap;
    bool	view_linenumber;
    bool	view_whitespace;
    bool	view_endofline;
    bool	view_indentguide;
    bool	view_longlineguide;

#if 0
/*****************************************************************************/

    /// Currently opened file name
    class wxFileName	currentfilename;


public:
    // *** File operations ***

    /// Clear the currently loaded file and start a new one
    void	FileNew();

    /// Attempt to open and load an existing file
    bool	FileOpen(const wxString& filename);

    /// Save currently opened buffer into the associated file name
    bool	FileSave();

    /// Save buffer into new file name
    bool	FileSaveAs(const wxString& filename);

    /// Reload the associated file
    bool	FileRevert();

    /// Load a file from the given wxInputStream
    bool	LoadInputStream(wxInputStream& stream);

    /// Has an associated file, used by FileSave()
    bool	HasFilename() const;

    /// Return full path the associated file
    wxString	GetFileFullpath() const;

    /// Return only the file name component with extension.
    wxString	GetFileBasename() const;

    // _Updateable_ reference to modification flag of current file.
    bool&	ModifiedFlag();

/*****************************************************************************/
#endif

private:
    DECLARE_EVENT_TABLE()
};

class WQuickFindBar : public wxPanel
{
public:

    WQuickFindBar(class WCryptoTE* wmain);

    class wxBoxSizer*	sizerQuickFind;
    class wxTextCtrl*	textctrlQuickFind;
};

class WQuickGotoBar : public wxPanel
{
public:

    WQuickGotoBar(class WCryptoTE* wmain);

    class wxBoxSizer*	sizerQuickGoto;
    class wxTextCtrl*	textctrlGoto;
};

#endif // WTEXTPAGE_H
