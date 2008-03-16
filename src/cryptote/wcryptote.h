// $Id$

#ifndef WCRYPTOTE_H
#define WCRYPTOTE_H

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/hyperlink.h>
#include <wx/aui/aui.h>

#include "enctain.h"

// *** Global Identifiers used for Controls, Icons and Menu Items ***

enum {
    myID_FIRST = wxID_HIGHEST + 1,

    // Main Window Controls

    myID_AUINOTEBOOK,
    myID_TIMER_IDLECHECK,

    // Main Window Menu Items

    myID_MENU_CONTAINER_SHOWLIST,
    myID_MENU_CONTAINER_SETPASS,

    myID_MENU_SUBFILE_NEW,
    myID_MENU_SUBFILE_OPEN,
    myID_MENU_SUBFILE_IMPORT,
    myID_MENU_SUBFILE_EXPORT,
    myID_MENU_SUBFILE_PROPERTIES,
    myID_MENU_SUBFILE_RENAME,
    myID_MENU_SUBFILE_CLOSE,
    myID_MENU_SUBFILE_DELETE,

    myID_MENU_EDIT_QUICKFIND,
    myID_MENU_EDIT_GOTO,
    myID_MENU_EDIT_SELECTLINE,

    myID_MENU_VIEW_LINEWRAP,
    myID_MENU_VIEW_LINENUMBER,
    myID_MENU_VIEW_WHITESPACE,	
    myID_MENU_VIEW_ENDOFLINE,
    myID_MENU_VIEW_INDENTGUIDE,
    myID_MENU_VIEW_LONGLINEGUIDE,

    myID_MENU_VIEW_BIGICONS,
    myID_MENU_VIEW_LIST,
    myID_MENU_VIEW_REPORT,

    // (Other) Main Window Accelerators

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

    // FileType Icons

    myID_FILETYPE_BINARY,
    myID_FILETYPE_TEXT,
    myID_FILETYPE_IMAGE,

    myID_FILETYPE_BINARY_LARGE,
    myID_FILETYPE_TEXT_LARGE,
    myID_FILETYPE_IMAGE_LARGE,
};

class WCryptoTE : public wxFrame
{
public:
    WCryptoTE(wxWindow* parent);
    ~WCryptoTE();

    // *** Operations ***

    /// Temporarily set status bar to show given text.
    void	UpdateStatusBar(const wxString& str);

    /// Callback from any subdialog to hide it's pane 
    void	HidePane(wxWindow* child);

    /// Checks if any subfile was modified or the main container modified.
    bool	IsModified();

    /// Set menu items and toolbar accordingly if the file was modified.
    void	UpdateModified();

    /// Helper to set the main modified flag and update the main window.
    void	SetModified();

    /// Attempt to find a notebook page showing the given subfileid.
    class WNotePage* FindSubFilePage(unsigned int sfid);

    /// Open one of the SubFiles in the Container
    void	OpenSubFile(unsigned int sfid);

    /// Update the modified page caption on the notebook
    void	UpdateSubFileCaption(int sfid);

    /// Set the modified page image on notebook pages
    void	UpdateSubFileModified(WNotePage* page, bool modified);

    /// Import a list of full file paths as new subfiles into the container.
    void	ImportSubFiles(const wxArrayString& filelist, int filetype, bool openpage);

    /// Export one of the SubFiles of the container, either the currently
    /// opened buffer or directly from the container.
    void	ExportSubFile(unsigned int sfid, wxOutputStream& os);

    /// Delete one of the existing subfiles, fixes up all current references.
    void	DeleteSubFile(unsigned int sfid, bool resetfilelist);

    /// Update the window title when a container is loaded.
    void	UpdateTitle();

    /// Toggle the filelist pane on or off and set the corresponding button
    void	ShowFilelistPane(bool on);

    /// Discard any current container and set up a default new one with one
    /// activated text file.
    void	ContainerNew();

    /// Load a container file into the editor, discard any current container.
    bool	ContainerOpen(const wxString& filename);

    /// Save the current container to a file.
    bool	ContainerSaveAs(const wxString& filename);

    // *** Preference Variables ***

    /// Theme set using BitmapCatalog
    int		prefs_bitmaptheme;

    /// Flag if backup files of the container should be created
    bool	prefs_makebackups;

    /// How many backups to keep.
    int		prefs_backupnum;
    
    /// Flag if the editor should automatically save and close container files
    /// after a period of inactivity.
    bool	prefs_autoclose;

    /// Number of minutes of inactivity until the container is saved and closed.
    int		prefs_autoclosetime;

    /// Flag if to also close CryptoTE after the idle timeout.
    bool	prefs_autocloseexit;
    
    /// Retrieve config settings from registry or config file
    void	LoadPreferences();

    // *** Event Handlers ***

    // Generic Events

    void	OnClose(wxCloseEvent& event);

    /// Helper Function for OnClose() show "Save container" dialog if it is
    /// modified.
    bool	AllowCloseModified();

    // Menu Events

    void	OnMenuContainerOpen(wxCommandEvent& event);
    void	OnMenuContainerSave(wxCommandEvent& event);
    void	OnMenuContainerSaveAs(wxCommandEvent& event);
    void	OnMenuContainerRevert(wxCommandEvent& event);
    void	OnMenuContainerClose(wxCommandEvent& event);

    bool	UserContainerOpen();
    bool	UserContainerSave();
    bool	UserContainerSaveAs();

    void	OnMenuContainerShowList(wxCommandEvent& event);
    void	OnMenuContainerProperties(wxCommandEvent& event);
    void	OnMenuContainerSetPassword(wxCommandEvent& event);

    void	OnMenuContainerPreferences(wxCommandEvent& event);

    void	OnMenuContainerQuit(wxCommandEvent& event);

    void	OnMenuSubFileNew(wxCommandEvent& event);
    void	OnMenuSubFileImport(wxCommandEvent& event);
    void	OnMenuSubFileExport(wxCommandEvent& event);
    void	OnMenuSubFileProperties(wxCommandEvent& event);
    void	OnMenuSubFileClose(wxCommandEvent& event);

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

    // wxAuiManager Callbacks
    
    void	OnAuiManagerPaneClose(wxAuiManagerEvent& event);

    // wxAuiNotebook Callbacks

    void	OnNotebookPageChanged(wxAuiNotebookEvent& event);
    void	OnNotebookPageClose(wxAuiNotebookEvent& event);
    void	OnNotebookPageRightDown(wxAuiNotebookEvent& event);

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
    class wxMenuBar*	menubar_plain;
    class wxMenuBar*	menubar_textpage;
    class wxMenuBar*	menubar_binarypage;

    class wxMenuBar*	menubar_active;

    class wxToolBar*	toolbar;
    class WStatusBar*	statusbar;

    wxMenuBar* 		CreateMenuBar(const wxClassInfo* page);

    void 		CreateToolBar();

protected:
    // *** wxAUI Window Manager ***

    wxAuiManager 	auimgr;

    // *** Displayed or Hidden Panes ***

    wxAuiNotebook*	auinotebook;

public:

    // *** Container Loaded ***

    /// Container object used.
    Enctain::Container*	container;

    /// Associated file name
    wxFileName		container_filename;

    /// Whether the container's or a subfile's metadata was modified
    bool		main_modified;

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

    /// Container File List Pane
    class WFileList*	filelistpane;

public:

    // *** Idle-Timer ***

    /// wxEvent::GetTimestamp() value of the last user event.
    long		lastuserevent;

    /// wxTimer calling the idle event handler
    wxTimer		idlechecktimer;

    /// Old text from status bar before replacing it with idle time counter.
    wxString		idletimestatusbar;
    
    /// Timer event handler called by the wxTimer object every 5 seconds
    void		OnIdleTimerCheck(wxTimerEvent& event);

    /// Called by the main event loop FilterEvent() in wxApp
    void		ResetIdleTimer();

private:
    DECLARE_EVENT_TABLE()
};

class WStatusBar : public wxStatusBar, public Enctain::ProgressIndicator
{
public:
    WStatusBar(wxWindow *parent);

    void		OnSize(wxSizeEvent& event);
    void		SetLock(bool on);

    wxWindow *parent;

    wxStaticBitmap*	lockbitmap;

    wxWindow*		panelProgress;
    wxBoxSizer* 	sizerProgress;
    wxStaticText*	labelProgress;
    wxGauge*		gaugeProgress;

    /// Pure virtual function called when the progress indicator should
    /// start. The range is given in this call.
    virtual void	ProgressStart(const char* text, size_t value, size_t limit);

    /// Pure virtual function called when the progress indicator should be
    /// updated.
    virtual void	ProgressUpdate(size_t value);

    /// Pure virtual function called when the progress indicator should be
    /// hidden.
    virtual void	ProgressStop();

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

    /// Helper to set the modified flag of this page and also update the main
    /// window.
    void	SetModified(bool modified);

public:
    /// associated container subfile identifier of page data
    int		subfileid;

    /// modified flag
    bool	page_modified;

public:

    /// Return the text to display in the notebook
    virtual wxString	GetCaption() = 0;

    /// Called when the notebook page is activated/focused.
    virtual void	PageFocused() = 0;

    /// Called when the notebook page is deactivated.
    virtual void	PageBlurred() = 0;

    /// Called when the notebook page should save it's data.
    virtual void	PageSaveData() = 0;

    /// Called when the notebook page is closed.
    virtual void	PageClosed() = 0;

    /// Clear buffer and load all data from a file
    virtual size_t	ImportFile(wxFile& file) = 0;

    /// Write current data buffer to the output stream. Used by SubFile->Export.
    virtual void	ExportBuffer(wxOutputStream& outstream) = 0;

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
