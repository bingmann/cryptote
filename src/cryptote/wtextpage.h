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

    /// Styles Enumeration
    enum StyleNum {
        STYLE_FINDHIGHLIGHT = 16,
    };

    /// Margins Enumeration
    enum MarginNum {
	MARGIN_LINENUMBER = 0
    };

    // *** Operations ***

    /// Return the text to display in the notebook
    virtual wxString	GetCaption();

    /// Load a SubFile from the current container.
    bool		LoadSubFile(unsigned int sfid);

    /// Load View Settings from Metadata
    bool		LoadSubFileMetaSettings();

    /// Load View Settings from Metadata
    void		SaveSubFileMetaSettings();

    /// Clear buffer and load all data from a file
    virtual size_t	ImportFile(wxFile& file);

    /// Write buffer to the output stream
    virtual void	ExportBuffer(wxOutputStream& outstream);

    /// Insert a string at current position.
    void		AddText(const wxString& text);

    /// Prepare for a Quick-Find by setting the search anchor point, backwards
    /// for searching backwards and reset for terminating incremental search
    /// and restarting.
    void		PrepareQuickFind(bool backwards, bool reset);

    /// Execute Quick-Find for a search string.
    void	        DoQuickFind(bool backwards, const wxString& findtext);

    /// Terminate a quick find sequence.
    virtual void        StopQuickFind();

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

    /// Execute Quick-Goto for a given string or set an error message. The goto
    /// window will close if the function returns true.
    virtual bool	DoQuickGoto(const wxString& gototext);

    // *** Scintilla Callbacks ***

    void	OnScintillaUpdateUI(class wxStyledTextEvent& event);
    void	OnScintillaSavePointReached(class wxStyledTextEvent& event);
    void	OnScintillaSavePointLeft(class wxStyledTextEvent& event);
    void	OnScintillaZoom(class wxStyledTextEvent& event);
    void	OnScintillaPainted(class wxStyledTextEvent& event);

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

    void	SetZoom(int level);
    int		GetZoom();

protected:

    /// Current starting position and highlighted for incremental Quick-Find
    int		quickfind_startpos;
    int		quickfind_length;

    /// Settings of current View Options

    bool	view_linewrap;
    bool	view_linenumber;
    bool	view_whitespace;
    bool	view_endofline;
    bool	view_indentguide;
    bool	view_longlineguide;

    int		cursor_firstvisibleline;
    int		cursor_xoffset;
    int		cursor_currentpos;

    // *** Structures used to Save View Options ***

    struct MetaSettingsv00000001
    {
	uint32_t	version;

	unsigned char	view_linewrap;
	unsigned char	view_linenumber;
	unsigned char	view_whitespace;
	unsigned char	view_endofline;
	unsigned char 	view_indentguide;
	unsigned char	view_longlineguide;

	int		view_zoom;

	int		cursor_firstvisibleline;
	int		cursor_xoffset;
	int		cursor_currentpos;
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
