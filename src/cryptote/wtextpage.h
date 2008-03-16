// $Id$

#ifndef WTEXTPAGE_H
#define WTEXTPAGE_H

#include <wx/wx.h>

#include "wcryptote.h"

class WTextPage : public WNotePage
{
public:

    WTextPage(class WCryptoTE* parent);

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

    /// Return the text to display in the notebook
    virtual wxString	GetCaption();

    /// Load a SubFile from the current container.
    bool		LoadSubFile(unsigned int sfid);

    /// Load View Settings from Metadata
    bool		LoadSubFileMetaSettings(unsigned int sfid);

    /// Load View Settings from Metadata
    void		SaveSubFileMetaSettings(unsigned int sfid);

    /// Clear buffer and load all data from a file
    virtual size_t	ImportFile(wxFile& file);

    /// Write buffer to the output stream
    virtual void	ExportBuffer(wxOutputStream& outstream);

    // *** Event Handlers ***

    void	OnContextMenu(wxContextMenuEvent& event);

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

    /// Called when the notebook page should save it's data.
    virtual void	PageSaveData();

    /// Called when the notebook page is closed.
    virtual void	PageClosed();

    virtual void	PrepareQuickFind(bool backwards, bool reset);
    virtual void	DoQuickFind(bool backwards, const wxString& findtext);
    virtual bool	DoQuickGoto(const wxString& gototext);

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
    int		quickfind_startpos;

    /// Settings of current View Options

    bool	view_linewrap;
    bool	view_linenumber;
    bool	view_whitespace;
    bool	view_endofline;
    bool	view_indentguide;
    bool	view_longlineguide;

    // *** Structures used to Save View Options ***

    struct MetaSettingsv00000001
    {
	uint32_t version;

	unsigned char	view_linewrap;
	unsigned char	view_linenumber;
	unsigned char	view_whitespace;
	unsigned char	view_endofline;
	unsigned char 	view_indentguide;
	unsigned char	view_longlineguide;
    }
	__attribute__((packed));

private:
    DECLARE_EVENT_TABLE();

    DECLARE_ABSTRACT_CLASS(WTextPage);
};

class WQuickFindBar : public wxPanel
{
public:

    WQuickFindBar(class WCryptoTE* wmain);

    void		set_bitmaps();

    class wxBoxSizer*	sizerQuickFind;

    wxBitmapButton*	buttonQuickFindClose;
    class wxTextCtrl*	textctrlQuickFind;
    wxBitmapButton*	buttonQuickFindNext;
    wxBitmapButton*	buttonQuickFindPrev;
};

class WQuickGotoBar : public wxPanel
{
public:

    WQuickGotoBar(class WCryptoTE* wmain);

    void		set_bitmaps();

    wxBitmapButton*	buttonGotoCancel;
    class wxBoxSizer*	sizerQuickGoto;
    class wxTextCtrl*	textctrlGoto;
    wxBitmapButton*	buttonGotoGo;
};

#endif // WTEXTPAGE_H
