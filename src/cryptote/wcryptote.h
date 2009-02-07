// $Id$

#ifndef WCRYPTOTE_H
#define WCRYPTOTE_H

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/hyperlink.h>
#include <wx/wfstream.h>
#include <wx/aui/aui.h>
#include <wx/wxhtml.h>

#include "enctain.h"

// *** Global Identifiers used for Controls, Icons and Menu Items ***

enum {
    myID_FIRST = wxID_HIGHEST + 1,

    // Main Window Controls

    myID_AUINOTEBOOK,
    myID_TIMER_IDLECHECK,

    // Main Window Menu Items

    myID_MENU_CONTAINER_SHOWLIST,
    myID_MENU_CONTAINER_PASSLIST,

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
    myID_MENU_EDIT_INSERT_PASSWORD,
    myID_MENU_EDIT_INSERT_PASSWORD_DIALOG,
    myID_TOOL_EDIT_INSERT_PASSWORD,
    myID_MENU_EDIT_INSERT_DATETIME,
    myID_TOOL_EDIT_INSERT_DATETIME,
    myID_MENU_EDIT_INSERT_DATETIME_YYYYMMDD_HHMMSS,
    myID_MENU_EDIT_INSERT_DATETIME_YYYYMMDD,
    myID_MENU_EDIT_INSERT_DATETIME_HHMMSS,
    myID_MENU_EDIT_INSERT_DATETIME_LOCALE,
    myID_MENU_EDIT_INSERT_DATETIME_LOCALE_DATE,
    myID_MENU_EDIT_INSERT_DATETIME_LOCALE_TIME,
    myID_MENU_EDIT_INSERT_DATETIME_RFC822,

    myID_MENU_VIEW_LINEWRAP,
    myID_MENU_VIEW_LINENUMBER,
    myID_MENU_VIEW_WHITESPACE,	
    myID_MENU_VIEW_ENDOFLINE,
    myID_MENU_VIEW_INDENTGUIDE,
    myID_MENU_VIEW_LONGLINEGUIDE,
    myID_MENU_VIEW_ZOOM,
    myID_MENU_VIEW_ZOOM_INCREASE,
    myID_MENU_VIEW_ZOOM_DECREASE,
    myID_MENU_VIEW_ZOOM_RESET,

    myID_MENU_VIEW_BIGICONS,
    myID_MENU_VIEW_LIST,
    myID_MENU_VIEW_REPORT,

    myID_MENU_HELP_BROWSER,
    myID_MENU_HELP_WEBUPDATECHECK,

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

    // *** Other Icons / Identifiers

    myID_IMAGE_USERKEYSLOT,
    myID_IMAGE_USERKEYSLOT_ACTIVE,

    // *** Variable Menu Item Ranges

    myID_MENU_EDIT_INSERT_PASSWORD_FIRST = wxID_HIGHEST + 1000,

    myID_MENU_SHOW_COLUMN0 = wxID_HIGHEST + 2000,
};

class WCryptoTE : public wxFrame
{
public:
    WCryptoTE(wxWindow* parent);
    ~WCryptoTE();

    // *** Operations ***

    /// Return a localized string for the given error code.
    static const wxChar* EnctainErrorString(Enctain::error_type e);

    /// Return a localized string for the given exception object.
    static wxString EnctainExceptionString(Enctain::Exception& e);

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
    void	ImportSubFiles(const wxArrayString& filelist, const std::string& filetype, bool openpage);

    /// Export one of the SubFiles of the container, either the currently
    /// opened buffer or directly from the container.
    void	ExportSubFile(unsigned int sfid, wxOutputStream& os);

    /// Delete one of the existing subfiles, fixes up all current references.
    void	DeleteSubFile(unsigned int sfid, bool resetfilelist);

    /// Update the window title when a container is loaded.
    void	UpdateTitle();

    /// Return the current file name.
    wxString	GetSavedFilename();

    /// Toggle the filelist pane on or off and set the corresponding button
    void	ShowFilelistPane(bool on);

    /// Hide Quick-Find and Quick-Goto Bars
    void	HideQuickBars();

    /// Discard any current container and set up a default new one with one
    /// activated text file.
    void	ContainerNew();

    /// Load a container file into the editor, discard any current container.
    bool	ContainerOpen(const wxString& filename, const wxString& defpass=wxEmptyString);

    /// Save the current container to a file.
    bool	ContainerSaveAs(const wxString& filename);

    /// Save the list of currently opened subfiles in the global properties
    void	SaveOpenSubFilelist();

    /// Restore list of opened subfiles by calling OpenSubFile.
    void	RestoreOpenSubFilelist();

    /// Query the web page for the newest version and check if the current
    /// should be updated.
    void	WebUpdateCheck();

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
    
    /// Flag to keep the current file open on Win32 and thus locking it. On
    /// Linux the opened file is flock()-ed
    bool	prefs_sharelock;

    /// Flag is allow auto checking for updates online.
    bool	prefs_webupdatecheck;

    /// Time value when the last update check was performed successfully.
    time_t	prefs_webupdatecheck_time;

    /// Highest updated version confirmed by the user.
    wxString	prefs_webupdatecheck_version;

    /// Retrieve config settings from registry or config file
    void	LoadPreferences();

    // *** Container Option Variables ***

    /// Restore positions
    bool	copt_restoreview;

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
    void	OnMenuContainerPasswordList(wxCommandEvent& event);

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
    void	OnMenuEditInsertPassword(wxCommandEvent& event);
    void	OnToolEditInsertPassword(wxCommandEvent& event);
    void	OnMenuEditInsertPasswordPreset(wxCommandEvent& event);
    void	OnMenuEditInsertDateTime(wxCommandEvent& event);
    void	OnToolEditInsertDateTime(wxCommandEvent& event);

    void	OnMenuViewLineWrap(wxCommandEvent& event);
    void	OnMenuViewLineNumber(wxCommandEvent& event);
    void	OnMenuViewWhitespace(wxCommandEvent& event);
    void	OnMenuViewEndOfLine(wxCommandEvent& event);
    void	OnMenuViewIndentGuide(wxCommandEvent& event);
    void	OnMenuViewLonglineGuide(wxCommandEvent& event);
    void	OnMenuViewZoomIncrease(wxCommandEvent& event);
    void	OnMenuViewZoomDecrease(wxCommandEvent& event);
    void	OnMenuViewZoomReset(wxCommandEvent& event);

    void	OnMenuHelpBrowser(wxCommandEvent& event);
    void	OnMenuHelpWebUpdateCheck(wxCommandEvent& event);
    void	OnMenuHelpAbout(wxCommandEvent& event);

    // Accelerator Events

    void	OnAccelEscape(wxCommandEvent& event);

    // wxAuiManager Callbacks
    
    void	OnAuiManagerPaneClose(wxAuiManagerEvent& event);

    // wxAuiNotebook Callbacks

    void	OnNotebookPageChanged(wxAuiNotebookEvent& event);
    void	OnNotebookPageClose(wxAuiNotebookEvent& event);
    void	OnNotebookPageRightDown(wxAuiNotebookEvent& event);

    /// Update the window's menus and toolbars when a notebook page is
    /// activated.
    void	UpdateNotebookPageChanged(int pageid, WNotePage* page);

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

    void		UpdateMenuInsertPassword();

protected:
    // *** wxAUI Window Manager ***

    wxAuiManager 	auimgr;

public:
    // *** Displayed or Hidden Panes ***

    wxAuiNotebook*	auinotebook;

    // *** Container Loaded ***

    /// Container object used. This class is a reference-counter pimpl;
    Enctain::Container	container;

    /// Associated file name
    wxFileName		container_filename;

    /// File Object which stays open to keep an exclusive share lock on Win32.
    wxFile*		container_filehandle;

    /// Whether the container's or a subfile's metadata was modified
    bool		main_modified;

public:

    /// Currently selected notebook page (or NULL).
    class WNotePage*	cpage;

    /// Index in notebook of the current page
    int			cpageid;

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

    /// Password Generator Dialog Box
    class WPassGen*	wpassgen;

    /// Reentry flag for WebUpdateCheck
    bool		webupdatecheck_running;

    /// wxHtmlHelp displayer
    wxHtmlHelpController* m_htmlhelp;

    /// Return initialized help displayer
    wxHtmlHelpController* GetHtmlHelpController();
    
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
    void		SetModified(bool on);

    wxWindow *parent;

    wxStaticBitmap*	lockbitmap;

    wxWindow*		panelProgress;
    wxBoxSizer* 	sizerProgress;
    wxStaticText*	labelProgress;
    wxGauge*		gaugeProgress;

    /// Pure virtual function called when the progress indicator should
    /// start. The range is given in this call.
    virtual void	ProgressStart(const char* text, Enctain::progress_indicator_type pitype,
				      size_t value, size_t limit);

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

class WWebUpdateCheck : public wxDialog
{
public:
    // begin wxGlade: WWebUpdateCheck::ids
    // end wxGlade

    WWebUpdateCheck(wxWindow* parent, const wxString& newversion, const wxString& changes, int id=wxID_ANY, const wxString& title=wxEmptyString, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE);

private:
    // begin wxGlade: WWebUpdateCheck::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: WWebUpdateCheck::attributes
    wxStaticText* labelNewVersion;
    wxHyperlinkCtrl* hyperlink1;
    wxTextCtrl* textctrlChanges;
    wxButton* buttonOK;
    wxButton* buttonDisable;
    wxButton* buttonClose;
    // end wxGlade

    virtual void OnButtonDisableWebUpdateCheck(wxCommandEvent &event); // wxGlade: <event_handler>

protected:
    DECLARE_EVENT_TABLE()
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

    /// Terminate a quick find sequence
    virtual void        StopQuickFind() = 0;

    /// Execute Quick-Goto for a given string or set an error message. The goto
    /// window will close if the function returns true.
    virtual bool	DoQuickGoto(const wxString& gototext) = 0;

    DECLARE_ABSTRACT_CLASS(WNotePage);
};

struct DataInputStream : public Enctain::DataInput
{
    wxInputStream&	is;

    DataInputStream(wxInputStream& s)
	: is(s)
    {
    }

    virtual unsigned int Input(void* data, size_t maxlen)
    {
	return is.Read(data, maxlen).LastRead();
    }
};

/** Write the incoming file data into the output file. */
class DataOutputStream : public Enctain::DataOutput
{
public:
    class wxOutputStream&	outstream;

    /// Constructor using a reference to an already open stream.
    DataOutputStream(wxOutputStream& os)
	: outstream(os)
    {
    }

    /// Virtual callback function to save data.
    virtual bool Output(const void* data, size_t datalen)
    {
	return outstream.Write(data, datalen).IsOk();
    }
};

#endif // WCRYPTOTE_H
