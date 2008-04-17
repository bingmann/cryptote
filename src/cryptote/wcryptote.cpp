// $Id$

#include "wcryptote.h"
#include "bmpcat.h"
#include "wtextpage.h"
#include "wfind.h"
#include "wfilelist.h"
#include "wfileprop.h"
#include "wcntprop.h"
#include "wmsgdlg.h"
#include "wbinpage.h"
#include "wpass.h"
#include "wprefs.h"
#include "pwgen/wpassgen.h"

#include <wx/config.h>
#include "common/tools.h"

#if defined(__WINDOWS__)
#include <share.h>
#endif

WCryptoTE::WCryptoTE(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(750, 550),
	      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    cpage = NULL;
    cpageid = -1;
    container = NULL;
    container_filehandle = NULL;
    main_modified = false;
    lastuserevent = ::wxGetLocalTime();
    wpassgen = NULL;

    copt_restoreview = true;

    LoadPreferences();
    BitmapCatalog::GetSingleton()->SetTheme(prefs_bitmaptheme);

    Enctain::Container::SetSignature("CryptoTE");

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

    menubar_plain = CreateMenuBar(NULL);
    menubar_textpage = CreateMenuBar(CLASSINFO(WTextPage));
    menubar_binarypage = CreateMenuBar(CLASSINFO(WBinaryPage));
    menubar_active = menubar_plain;
    toolbar = NULL;

    SetMenuBar(menubar_active);
    CreateToolBar();

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
				    wxAUI_NB_DEFAULT_STYLE | wxNO_BORDER);

    quickfindbar = new WQuickFindBar(this);

    quickgotobar = new WQuickGotoBar(this);

    findreplacedlg = NULL;

    filelistpane = new WFileList(this);

    wpassgen = new WPassGen(this, false);
    UpdateMenuInsertPassword();

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
		   CaptionVisible(false).PaneBorder(false).Row(10).
		   Bottom().Gripper().
#if wxCHECK_VERSION(2,8,7)
		   DockFixed().
#else
		   Dock().
#endif
		   LeftDockable(false).RightDockable(false));

    auimgr.AddPane(quickgotobar, wxAuiPaneInfo().Hide().
		   Name(wxT("quickgotobar")).Caption(_("Quick-Goto")).
		   CaptionVisible(false).PaneBorder(false).Row(10).
		   Bottom().Gripper().
#if wxCHECK_VERSION(2,8,7)
		   DockFixed().
#else
		   Dock().
#endif
		   LeftDockable(false).RightDockable(false));

    auimgr.AddPane(filelistpane, wxAuiPaneInfo().
		   Name(wxT("filelist")).Caption(_("Container")).
		   Bottom());

    // "commit" all changes made to wxAuiManager
    auimgr.Update();

    quickfindbar_visible = false;
    quickgotobar_visible = false;

    // Create and start idle-timer check function
    idlechecktimer.SetOwner(this, myID_TIMER_IDLECHECK);
    idlechecktimer.Start(1000, wxTIMER_CONTINUOUS);

    Centre();

    ContainerNew();
}

WCryptoTE::~WCryptoTE()
{
    auimgr.UnInit();

    if (container) {
	delete container;
	container = NULL;
    }
    if (container_filehandle) {
	delete container_filehandle;
	container_filehandle = NULL;
    }
}

const wxChar* WCryptoTE::EnctainErrorString(Enctain::error_t e)
{
    using namespace Enctain;

    switch(e)
    {
    case ETE_SUCCESS:
	return _("Success.");

    case ETE_SAVE_NO_PASSWORD:
	return _("Error saving container: no encryption password set!");

    case ETE_LOAD_HEADER1:
	return _("Error loading container: could not read header.");

    case ETE_LOAD_HEADER1_SIGNATURE:
	return _("Error loading container: could not read header, invalid signature.");

    case ETE_LOAD_HEADER1_VERSION:
	return _("Error loading container: could not read header, invalid version.");

    case ETE_LOAD_HEADER1_METADATA:
	return _("Error loading container: could not read header, invalid metadata.");

    case ETE_LOAD_HEADER1_METADATA_PARSE:
	return _("Error loading container: could not read header, metadata parse failed.");

    case ETE_LOAD_HEADER2:
	return _("Error loading container: could not read secondary header.");

    case ETE_LOAD_HEADER2_ENCRYPTION:
	return _("Error loading container: could not read secondary header, check encryption key.");

    case ETE_LOAD_HEADER2_METADATA:
	return _("Error loading container: could not read secondary header, invalid metadata.");

    case ETE_LOAD_HEADER2_METADATA_CRC32:
	return _("Error loading container: could not read secondary header, metadata crc32 mismatch.");

    case ETE_LOAD_HEADER2_METADATA_PARSE:
	return _("Error loading container: could not read secondary header, metadata parse failed.");

    case ETE_LOAD_SUBFILE:
	return _("Error loading container: could not read encrypted subfile data.");

    case ETE_SUBFILE_COMPRESSION_INVALID:
	return _("Error in subfile: unknown compression algorithm.");

    case ETE_SUBFILE_ENCRYPTION_INVALID:
	return _("Error in subfile: unknown encryption cipher.");

    case ETE_SUBFILE_ENCRYPTION_LENGTH:
	return _("Error in subfile: invalid encrypted data length.");

    case ETE_SUBFILE_UNEXPECTED_EOF:
	return _("Error in subfile: read beyond end of stream.");

    case ETE_SUBFILE_CRC32:
	return _("Error in subfile: crc32 mismatch, data possibly corrupt.");

    case ETE_Z_UNKNOWN:
	return _("Error in zlib: unknown error.");

    case ETE_Z_OK:
	return _("Error in zlib: success.");

    case ETE_Z_NEED_DICT:
	return _("Error in zlib: need dictionary.");
	
    case ETE_Z_STREAM_END:
	return _("Error in zlib: stream end.");

    case ETE_Z_ERRNO:
	return _("Error in zlib: file error.");

    case ETE_Z_STREAM_ERROR:
	return _("Error in zlib: stream error.");

    case ETE_Z_DATA_ERROR:
	return _("Error in zlib: data error.");

    case ETE_Z_MEM_ERROR:
	return _("Error in zlib: insufficient memory.");

    case ETE_Z_BUF_ERROR:
	return _("Error in zlib: buffer error.");

    case ETE_Z_VERSION_ERROR:
	return _("Error in zlib: incompatible version.");

    case ETE_BZ_UNKNOWN:
	return _("Error in bzip2: unknown error.");

    case ETE_BZ_OK:
	return _("Error in bzip2: success.");

    case ETE_BZ_RUN_OK:
	return _("Error in bzip2: successful run.");

    case ETE_BZ_FLUSH_OK:
	return _("Error in bzip2: successful flush.");

    case ETE_BZ_FINISH_OK:
	return _("Error in bzip2: successful finish.");

    case ETE_BZ_STREAM_END:
	return _("Error in bzip2: stream end.");

    case ETE_BZ_SEQUENCE_ERROR:
	return _("Error in bzip2: sequence error.");

    case ETE_BZ_PARAM_ERROR:
	return _("Error in bzip2: parameter error.");

    case ETE_BZ_MEM_ERROR:
	return _("Error in bzip2: insufficient memory.");

    case ETE_BZ_DATA_ERROR:
	return _("Error in bzip2: data error.");

    case ETE_BZ_DATA_ERROR_MAGIC:
	return _("Error in bzip2: magic header error.");

    case ETE_BZ_IO_ERROR:
	return _("Error in bzip2: file system error.");

    case ETE_BZ_UNEXPECTED_EOF:
	return _("Error in bzip2: unexpected end of file.");

    case ETE_BZ_OUTBUFF_FULL:
	return _("Error in bzip2: output buffer full.");

    case ETE_BZ_CONFIG_ERROR:
	return _("Error in bzip2: platform config error.");

    default:
	return _("Unknown error code.");
    }
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

bool WCryptoTE::IsModified()
{
    // check metadata
    if (main_modified) return true;

    for(unsigned int pi = 0; pi < auinotebook->GetPageCount(); ++pi)
    {
	WNotePage* page = wxDynamicCast(auinotebook->GetPage(pi), WNotePage);
	if (page)
	{
	    if (page->page_modified) return true;
	}
	else
	{
	    wxLogError(_T("Invalid notebook page found."));
	}
    }

    return false;
}

void WCryptoTE::UpdateModified()
{
    bool modified = IsModified();

    menubar_active->Enable(wxID_SAVE, modified);
    menubar_active->Enable(wxID_REVERT, modified);

    toolbar->EnableTool(wxID_SAVE, modified);

    statusbar->SetModified(modified);
}

void WCryptoTE::SetModified()
{
    main_modified = true;
    UpdateModified();
}

WNotePage* WCryptoTE::FindSubFilePage(unsigned int sfid)
{
    for(unsigned int pi = 0; pi < auinotebook->GetPageCount(); ++pi)
    {
	WNotePage* page = wxDynamicCast(auinotebook->GetPage(pi), WNotePage);
	if (page)
	{
	    if (page->subfileid == (int)sfid)
		return page;
	}
	else
	{
	    wxLogError(_T("Invalid notebook page found."));
	}
    }

    return NULL;
}

void WCryptoTE::OpenSubFile(unsigned int sfid)
{
    WNotePage* findpage = FindSubFilePage(sfid);
    if (findpage != NULL)
    {
	// subfile with specified id is already open. change notebook tab to
	// the selected subfile.
	auinotebook->SetSelection( auinotebook->GetPageIndex(findpage) );
	return;
    }

    const std::string& filetype = container->GetSubFileProperty(sfid, "Filetype");

    if (filetype == "text")
    {
	WTextPage* textpage = new WTextPage(this);

	if (!textpage->LoadSubFile(sfid))
	{
	    wxLogError(_T("Error loading subfile into text page."));
	    delete textpage;
	}
	else
	{
	    auinotebook->AddPage(textpage, textpage->GetCaption(), true);

	    UpdateNotebookPageChanged(auinotebook->GetPageIndex(textpage), textpage);
	}
    }
    else
    {
	WBinaryPage* binarypage = new WBinaryPage(this);

	if (!binarypage->LoadSubFile(sfid))
	{
	    wxLogError(_T("Error loading subfile into binary page."));
	    delete binarypage;
	}
	else
	{
	    auinotebook->AddPage(binarypage, binarypage->GetCaption(), true);

	    UpdateNotebookPageChanged(auinotebook->GetPageIndex(binarypage), binarypage);
	}
    }
}

void WCryptoTE::UpdateSubFileCaption(int sfid)
{
    WNotePage* page = FindSubFilePage(sfid);
    if (!page) return;

    int pi = auinotebook->GetPageIndex(page);
    if (pi == wxNOT_FOUND) return;
  
    auinotebook->SetPageText(pi, page->GetCaption());
}

void WCryptoTE::UpdateSubFileModified(WNotePage* page, bool modified)
{
    #include "art/modified-12.h"

    int pi = auinotebook->GetPageIndex(page);
    if (pi == wxNOT_FOUND) return;

    auinotebook->SetPageBitmap(pi, modified ? wxBitmapFromMemory(modified_12_png) : wxNullBitmap);

    UpdateModified();
}

static inline bool CheckTextASCII(char c)
{
    static const unsigned char ok[256] = {
	// 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
	0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, // 00-0F: NUL - SI
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, // 10-1F: DLE - US
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 20-2F: " !"#$%&'()*+,-./"
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 30-3F: "0123456789:;<=>?"
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 40-4F: "@ABCDEFGHIJKLMNO"
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 50-5F: "PQRSTUVWXYZ[\]^_"
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 60-6F: "`abcdefghijklmno"
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 70-7F: "pqrstuvwxyz{|}~ "
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80-8F: depends on codepage
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 90-9F: depends on codepage
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A0-AF: depends on codepage
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B0-BF: depends on codepage
	0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // C0-CF: depends on codepage
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // D0-DF: depends on codepage
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // E0-EF: depends on codepage
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // F0-FF: depends on codepage
    };
    return ok[(unsigned char)(c) & 0xFF];
}

void WCryptoTE::ImportSubFiles(const wxArrayString& importlist, const std::string& importtype, bool openpage)
{
    size_t importsize = 0;
    size_t importnum = 0;

    for (unsigned int fi = 0; fi < importlist.GetCount(); ++fi)
    {
	wxFile filehandle(importlist[fi], wxFile::read);
	if (!filehandle.IsOpened()) continue;

	// Create new file in the container
	unsigned int sfnew = container->AppendSubFile();
    
	// use defaults from global properties
	long defcomp = 0;
	if (!strSTL2WX(container->GetGlobalEncryptedProperty("DefaultCompression")).ToLong(&defcomp))
	    defcomp = Enctain::COMPRESSION_ZLIB;

	long defencr = 0;
	if (!strSTL2WX(container->GetGlobalEncryptedProperty("DefaultEncryption")).ToLong(&defencr))
	    defencr = Enctain::ENCRYPTION_SERPENT256;

	container->SetSubFileCompression(sfnew, (Enctain::compression_t)defcomp);
	container->SetSubFileEncryption(sfnew, (Enctain::encryption_t)defencr);

	wxFileName fname (importlist[fi]);
	container->SetSubFileProperty(sfnew, "Name", strWX2STL(fname.GetFullName()));
	container->SetSubFileProperty(sfnew, "Author", strWX2STL(wxGetUserId()));
	container->SetSubFileProperty(sfnew, "CTime", strTimeStampNow());

	std::string filetype = importtype;
	if (filetype.empty())
	{
	    // try to detect file type
	    if (fname.GetExt().Lower() == _T("txt"))
		filetype = "text";
	    else
		filetype = "";
	}
	container->SetSubFileProperty(sfnew, "Filetype", filetype);

	if (openpage)
	{
	    // Open empty subfileid in text editor or binary viewer page and
	    // load the file via the page's method

	    OpenSubFile(sfnew);
	    
	    WNotePage* page = FindSubFilePage(sfnew);

	    importnum++;
	    importsize += page->ImportFile(filehandle);
	}
	else
	{
	    // Load complete file into wxMemoryBuffer and save data via SetSubFileData()
	    wxMemoryBuffer filedata;
	    bool istextfile = false;
	    if (importtype.empty()) istextfile = true;

	    {
		wxFileOffset filesize = filehandle.Length();
		statusbar->ProgressStart("Importing", 0, filesize);

		filedata.SetBufSize(filesize);
		size_t offset = 0;

		for (int i = 0; !filehandle.Eof(); i++)
		{
		    filedata.SetBufSize(offset + 65536);
		
		    size_t readmax = wxMin(65536, filedata.GetBufSize() - offset);
		    size_t rb = filehandle.Read((char*)filedata.GetData() + offset, readmax);
		    if (rb <= 0) break;

		    if (istextfile) // check if any non-printable ascii character is found
		    {
			char* cdata = (char*)filedata.GetData();
			for(unsigned int bo = offset; bo < offset + rb; ++bo)
			{
			    if (!CheckTextASCII(cdata[bo])) {
				istextfile = false;
				break;
			    }
			}
		    }

		    offset += rb;

		    statusbar->ProgressUpdate(offset);
		}

		filehandle.Close();
		filedata.SetDataLen(offset);

		statusbar->ProgressStop();
	    }

	    if (istextfile && importtype.empty())
	    {
		container->SetSubFileProperty(sfnew, "Filetype", "text");
	    }

	    Enctain::error_t e = container->SetSubFileData(sfnew, filedata.GetData(), filedata.GetDataLen());
	    if (e != Enctain::ETE_SUCCESS)
	    {
		wxMessageDialogErrorOK(this, WCryptoTE::EnctainErrorString(e));
	    }

	    container->SetSubFileProperty(sfnew, "MTime", strTimeStampNow());

	    importnum++;
	    importsize += filedata.GetDataLen();
	}
    }

    // update window
    filelistpane->ResetItems();

    UpdateStatusBar(wxString::Format(_("Imported %u bytes into %u new subfiles in container."),
				     importsize, importnum));
    SetModified();

    if (openpage)
    {
	if (cpage) cpage->SetFocus();
    }
}

void WCryptoTE::ExportSubFile(unsigned int sfid, wxOutputStream& outstream)
{
    WNotePage* page = FindSubFilePage(sfid);

    if (page)
    {
	// write currently opened buffer to output stream

	page->ExportBuffer(outstream);
    }
    else
    {
	// export directly from the container storage

	DataOutputStream dataout(outstream);
	Enctain::error_t e = container->GetSubFileData(sfid, dataout);
	if (e != Enctain::ETE_SUCCESS)
	{
	    wxMessageDialogErrorOK(this, EnctainErrorString(e));
	}
    }
}

void WCryptoTE::DeleteSubFile(unsigned int sfid, bool resetfilelist)
{
    if (!container) return;

    WNotePage* page = FindSubFilePage(sfid);
    if (page)
    {
	int pi = auinotebook->GetPageIndex(page);
	if (pi == wxNOT_FOUND) return;

	// PageClose event is not generated by wxAuiNotebook

	if (auinotebook->GetPageCount() == 1)
	{
	    // will be empty after the last page is closed
	    UpdateNotebookPageChanged(-1, NULL);
	}

	auinotebook->DeletePage(pi);
    }

    container->DeleteSubFile(sfid);

    // fix-up subfileid's of all notepages

    for(unsigned int pi = 0; pi < auinotebook->GetPageCount(); ++pi)
    {
	WNotePage* page = wxDynamicCast(auinotebook->GetPage(pi), WNotePage);
	if (page)
	{
	    if (page->subfileid >= (int)sfid)
	    {
		page->subfileid--;
	    }
	}
	else
	{
	    wxLogError(_T("Invalid notebook page found."));
	}
    }
    
    if (resetfilelist)
	filelistpane->ResetItems();

    SetModified();
}

void WCryptoTE::ShowFilelistPane(bool on)
{
    if (on)
    {
	auimgr.GetPane(filelistpane).Show();
	auimgr.Update();

	toolbar->ToggleTool(myID_MENU_CONTAINER_SHOWLIST, true);
    }
    else
    {
	auimgr.GetPane(filelistpane).Hide();
	auimgr.Update();

	toolbar->ToggleTool(myID_MENU_CONTAINER_SHOWLIST, false);
    }
}

void WCryptoTE::HideQuickBars()
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
}

void WCryptoTE::UpdateTitle()
{
    wxString title;

    if (container_filename.IsOk())
    {
	title += container_filename.GetFullName();
	title += _T(" - ");
    }

    title += _("CryptoTE v0.1");

    SetTitle(title);
}

void WCryptoTE::ContainerNew()
{
    if (container) {
	delete container;
	container = NULL;
    }
    if (container_filehandle) {
	delete container_filehandle;
	container_filehandle = NULL;
    }

    HideQuickBars();

    // close all notebook pages
    while( auinotebook->GetPageCount() > 0 )
    {
	wxWindow* w = auinotebook->GetPage(0);
	auinotebook->RemovePage(0);
	w->Destroy();
    }
    cpage = NULL;
    cpageid = -1;

    container = new Enctain::Container();
    container->SetProgressIndicator(statusbar);
    container_filename.Clear();
    main_modified = false;

    // Set up new container properties
    container->SetGlobalUnencryptedProperty("Author", strWX2STL(wxGetUserId()));

    container->SetGlobalEncryptedProperty("CTime", strTimeStampNow());

    // Set up one empty text file in the container
    unsigned int sf1 = container->AppendSubFile();

    container->SetSubFileProperty(sf1, "Name", strWX2STL(_("Untitled.txt")));
    container->SetSubFileProperty(sf1, "Filetype", "text");
    container->SetSubFileProperty(sf1, "Author", strWX2STL(wxGetUserId()));
    container->SetSubFileProperty(sf1, "CTime", strTimeStampNow());

    // use defaults from global properties
    long defcomp = 0;
    if (!strSTL2WX(container->GetGlobalEncryptedProperty("DefaultCompression")).ToLong(&defcomp))
	defcomp = Enctain::COMPRESSION_ZLIB;

    long defencr = 0;
    if (!strSTL2WX(container->GetGlobalEncryptedProperty("DefaultEncryption")).ToLong(&defencr))
	defencr = Enctain::ENCRYPTION_SERPENT256;

    container->SetSubFileCompression(sf1, (Enctain::compression_t)defcomp);
    container->SetSubFileEncryption(sf1, (Enctain::encryption_t)defencr);

    copt_restoreview = true;

    filelistpane->ResetItems();

    OpenSubFile(sf1);
    ShowFilelistPane(false);

    UpdateStatusBar(_("New container initialized."));
    UpdateTitle();
    UpdateModified();

    if (cpage) cpage->SetFocus();
}

bool WCryptoTE::ContainerOpen(const wxString& filename)
{
    std::auto_ptr<wxFile> fh (new wxFile);

#if defined(__WINDOWS__)
    if (prefs_sharelock)
    {
#if wxUSE_UNICODE
	int fd = _wsopen(filename.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYRW);
#else
	int fd = _sopen(filename.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYRW);
#endif
	if (fd < 0) {
	    wxLogSysError(_("Cannot open file '%s'"), filename.c_str());
	    return false;
	}
	fh->Attach(fd);
    }
    else
    {
	if (!fh->Open(filename.c_str(), wxFile::read)) return false;
    }
#elif defined(__UNIX__)
    if (prefs_sharelock)
    {
	// read_write is required to make a write-lock
	if (!fh->Open(filename.c_str(), wxFile::read_write)) return false;

	int lf = lockf(fh->fd(), F_TLOCK, 0);
	if (lf != 0) {
	    wxLogSysError(_("Cannot open file '%s'"), filename.c_str());
	    return false;
	}
    }
    else
    {
	if (!fh->Open(filename.c_str(), wxFile::read)) return false;
    }
#else
    if (!fh->Open(filename.c_str(), wxFile::read)) return false;
#endif

    std::auto_ptr<Enctain::Container> nc (new Enctain::Container);
    nc->SetProgressIndicator(statusbar);
    
    Enctain::error_t e;
    do
    {
	WGetPassword passdlg(this, filename);
	if (passdlg.ShowModal() != wxID_OK) return false;

	fh->Seek(0, wxFromStart);
	wxFileInputStream stream(*fh.get());
	if (!stream.IsOk()) return false;

	DataInputStream datain(stream);
	e = nc->Load(datain, strWX2STL(passdlg.GetPass()));

	if (e != Enctain::ETE_SUCCESS)
	{
	    if (e == Enctain::ETE_LOAD_HEADER2_ENCRYPTION)
	    {
		WMessageDialog dlg(this,
				   _("Error loading container: could not read encrypted header.\nEncryption key probably invalid. Retry?"),
				   _("CryptoTE"),
				   wxICON_ERROR,
				   wxID_YES, wxID_CANCEL);

		int id = dlg.ShowModal();

		if (id != wxID_YES)
		    return false;
	    }
	    else
	    {
		wxMessageDialogErrorOK(this, EnctainErrorString(e));
		return false;
	    }
	}
    } while(e != Enctain::ETE_SUCCESS);

    // Loading was successful

    if (container) {
	delete container;
    }
    container = nc.release();

    if (container_filehandle) {
	delete container_filehandle;
    }
    container_filehandle = fh.release();

    container_filename.Assign(filename);
    main_modified = false;

    HideQuickBars();

    // close all notebook pages
    while( auinotebook->GetPageCount() > 0 )
    {
	wxWindow* w = auinotebook->GetPage(0);
	auinotebook->RemovePage(0);
	w->Destroy();
    }
    cpage = NULL;
    cpageid = -1;

    filelistpane->LoadProperties();
    filelistpane->ResetItems();

    RestoreOpenSubFilelist();

    if (container->CountSubFile() == 1)
    {
	ShowFilelistPane(false);
    }
    else
    {
	ShowFilelistPane(true);
    }

    UpdateStatusBar(wxString::Format(_("Loaded container with %u subfiles from %s"),
				     container->CountSubFile(), container_filename.GetFullPath().c_str()));
    UpdateTitle();
    UpdateModified();

    return true;
}

bool WCryptoTE::ContainerSaveAs(const wxString& filename)
{
    if (!container) return false;

    // save all notebook pages
    for(unsigned int pi = 0; pi < auinotebook->GetPageCount(); ++pi)
    {
	WNotePage* page = wxDynamicCast(auinotebook->GetPage(pi), WNotePage);
	if (page)
	    page->PageSaveData();
	else
	    wxLogError(_T("Invalid notebook page found."));
    }

    // check that an encryption key is set
    if (!container->IsKeySet())
    {
	WSetPassword passdlg(this, filename);
	if (passdlg.ShowModal() != wxID_OK) return false;

	container->SetKey( strWX2STL(passdlg.GetPass()) );
    }

    // release share lock
    if (container_filehandle) {
	delete container_filehandle;
	container_filehandle = NULL;
    }

    if (prefs_makebackups)
    {
	int havebacks = 0;

	// Look how many backup files are in the directory. backup = 0 is the
	// current file in the directory.
	for(havebacks = 0; ; ++havebacks)
	{
	    wxFileName backname(filename);
	    if (havebacks != 0) {
		backname.SetName(backname.GetName() + wxString::Format(_("-backup%d"), havebacks));
	    }

	    if (!backname.FileExists()) break;
	}

	havebacks--;
	// havebacks == last available backup or real file.

	// Move backups one slot forwards
	while(havebacks >= 0)
	{
	    wxFileName backname(filename);
	    if (havebacks != 0) {
		backname.SetName(backname.GetName() + wxString::Format(_("-backup%d"), havebacks));
	    }

	    if (havebacks >= prefs_backupnum && havebacks != 0)
	    {
		// superfluous backup file. delete it.
		wxRemoveFile(backname.GetFullPath());
	    }
	    else
	    {
		// move current backup file one slot forwards
		wxFileName nextbackname(filename);
		nextbackname.SetName(nextbackname.GetName() + wxString::Format(_("-backup%d"), havebacks+1));
		
		wxRenameFile(backname.GetFullPath(), nextbackname.GetFullPath(), false);
	    }

	    havebacks--;
	}
    }

    std::auto_ptr<wxFile> fh (new wxFile);
#if defined(__WINDOWS__)
    if (prefs_sharelock)
    {
#if wxUSE_UNICODE
	int fd = _wsopen(filename.c_str(), _O_WRONLY | _O_CREAT | _O_BINARY, _SH_DENYRW);
#else
	int fd = _sopen(filename.c_str(), _O_WRONLY | _O_CREAT | _O_BINARY, _SH_DENYRW);
#endif
	if (fd < 0) {
	    wxLogSysError(_("Cannot create file '%s'"), filename.c_str());
	    return false;
	}
	fh->Attach(fd);
    }
    else
    {
	if (!fh->Create(filename.c_str(), true, wxS_DEFAULT)) return false;
    }
#elif defined(__UNIX__)
    if (prefs_sharelock)
    {
	if (!fh->Create(filename.c_str(), true, 02640)) return false;

	lockf(fh->fd(), F_LOCK, 0);
    }
    else
    {
	if (!fh->Create(filename.c_str(), true, wxS_DEFAULT)) return false;
    }
#else
    if (!fh->Create(filename.c_str(), true, wxS_DEFAULT)) return false;
#endif

    wxFileOutputStream stream(*fh.get());
    if (!stream.IsOk()) return false;

    container->SetGlobalEncryptedProperty("MTime", strTimeStampNow());
    filelistpane->SaveProperties();
    SaveOpenSubFilelist();

    DataOutputStream dataout(stream);
    Enctain::error_t e = container->Save(dataout);
    if (e != Enctain::ETE_SUCCESS)
    {
	UpdateStatusBar(EnctainErrorString(e));
	wxMessageDialogErrorOK(this, EnctainErrorString(e));
	return false;
    }

    container_filehandle = fh.release();
    container_filename.Assign(filename);
    main_modified = false;

    UpdateStatusBar(wxString::Format(_("Saved container with %u subfiles to %s"),
				     container->CountSubFile(), container_filename.GetFullPath().c_str()));
    UpdateTitle();
    UpdateModified();

    if (cpage) cpage->SetFocus();

    return true;
}

void WCryptoTE::SaveOpenSubFilelist()
{
    if (!copt_restoreview)
    {
	container->EraseGlobalEncryptedProperty("SubFilesOpened");
	return;
    }

    const size_t pagenum = auinotebook->GetPageCount();

    int sflist[pagenum];

    for (unsigned int pi = 0; pi < pagenum; ++pi)
    {
	WNotePage* sel = wxDynamicCast(auinotebook->GetPage(pi), WNotePage);

	if (sel)
	{
	    sflist[pi] = sel->subfileid;
	}
	else
	{
	    wxLogError(_T("Invalid notebook page found."));
	    sflist[pi] = 0;
	}
    }
    
    container->SetGlobalEncryptedProperty("SubFilesOpened", std::string((char*)&sflist, pagenum * sizeof(int)));
}

void WCryptoTE::RestoreOpenSubFilelist()
{
    // retrieve value of restore view option
    unsigned long restoreview;
    if ( !strSTL2WX(container->GetGlobalEncryptedProperty("RestoreView")).ToULong(&restoreview) ) {
	restoreview = 1;
    }
    copt_restoreview = restoreview;

    if (!copt_restoreview) return;

    std::string str = container->GetGlobalEncryptedProperty("SubFilesOpened");

    if (str.size() % 4 != 0) {
	wxLogError(_T("Invalid list of open subfiles."));
	return;
    }

    const int* sflist = reinterpret_cast<const int*>(str.data());
    
    for (unsigned int pi = 0; pi < str.size() / 4; ++pi)
    {
	OpenSubFile(sflist[pi]);
    }
}

static inline wxMenuItem* appendMenuItem(class wxMenu* parentMenu, int id,
					 const wxString& text, const wxString& helpString)
{
    wxMenuItem* mi = new wxMenuItem(parentMenu, id, text, helpString);
    mi->SetBitmap( BitmapCatalog::GetMenuBitmap(id) );
    parentMenu->Append(mi);
    return mi;
}

wxMenuBar* WCryptoTE::CreateMenuBar(const wxClassInfo* page)
{
    // Construct MenuBar

    wxMenuBar* menubar = new wxMenuBar;

    // *** Container

    wxMenu *menuContainer = new wxMenu;

    appendMenuItem(menuContainer, wxID_OPEN,
		   _("&Open ...\tCtrl+O"),
		   _("Open an existing encrypted container in the editor."));

    appendMenuItem(menuContainer, wxID_SAVE,
		   _("&Save\tCtrl+S"),
		   _("Save the current encrypted container to disk."));

    appendMenuItem(menuContainer, wxID_SAVEAS,
		   _("Save &as ...\tCtrl+Shift+S"),
		   _("Choose a file name and save the current encrypted container to disk."));

    appendMenuItem(menuContainer, wxID_REVERT,
		   _("&Revert\tCtrl+Shift+W"),
		   _("Revert the current container by reloading it from disk and losing all unsaved changes."));

    appendMenuItem(menuContainer, wxID_CLOSE,
		   _("&Close\tCtrl+Shift+N"),
		   _("Close the current encrypted container."));

    menuContainer->AppendSeparator();

    appendMenuItem(menuContainer, myID_MENU_CONTAINER_SHOWLIST,
		   _("Show &SubFile List"),
		   _("Show list of subfiles contained in current encrypted container."));

    appendMenuItem(menuContainer, wxID_PROPERTIES,
		   _("&Properties ...\tAlt+Enter"),
		   _("Show metadata properties of the encrypted container."));

    appendMenuItem(menuContainer, myID_MENU_CONTAINER_SETPASS,
		   _("&Change Password ..."),
		   _("Change the encryption password of the current container."));

    menuContainer->AppendSeparator();

    appendMenuItem(menuContainer, wxID_PREFERENCES,
		   _("Preferences ..."),
		   _("Show CryptoTE preferences."));

    menuContainer->AppendSeparator();

    appendMenuItem(menuContainer, wxID_EXIT,
		   _("&Quit\tAlt+F4"),
		   _("Exit CryptoTE."));

    menubar->Append(menuContainer, _("&Container"));

    // *** SubFile

    wxMenu *menuSubFile = new wxMenu;

    appendMenuItem(menuSubFile, myID_MENU_SUBFILE_NEW,
		   _("&New Text SubFile\tCtrl+N"),
		   _("Create a new empty text subfile in encrypted container."));

    appendMenuItem(menuSubFile, myID_MENU_SUBFILE_IMPORT,
		   _("&Import SubFile ...\tCtrl+I"),
		   _("Import any file from disk into encrypted container."));

    appendMenuItem(menuSubFile, myID_MENU_SUBFILE_EXPORT,
		   _("&Export SubFile ...\tCtrl+E"),
		   _("Export current subfile to disk."));
 
    appendMenuItem(menuSubFile, myID_MENU_SUBFILE_PROPERTIES,
		   _("&Properties ...\tAlt+Shift+Enter"),
		   _("Show metadata properties of current subfile."));

    appendMenuItem(menuSubFile, myID_MENU_SUBFILE_CLOSE,
		   _("&Close SubFile\tCtrl+W"),
		   _("Close the currently view subfile."));

    menubar->Append(menuSubFile, _("&SubFile"));

    if (page == CLASSINFO(WTextPage))
    {
	// *** Edit

	wxMenu *menuEdit = new wxMenu;

	appendMenuItem(menuEdit, wxID_UNDO,
		       _("&Undo\tCtrl+Z"),
		       _("Undo the last change."));

	appendMenuItem(menuEdit, wxID_REDO,
		       _("&Redo\tCtrl+Shift+Z"),
		       _("Redo the previously undone change."));

	menuEdit->AppendSeparator();

	appendMenuItem(menuEdit, wxID_CUT,
		       _("Cu&t\tCtrl+X"),
		       _("Cut selected text into clipboard."));

	appendMenuItem(menuEdit, wxID_COPY,
		       _("&Copy\tCtrl+C"),
		       _("Copy selected text into clipboard."));

	appendMenuItem(menuEdit, wxID_PASTE,
		       _("&Paste\tCtrl+V"),
		       _("Paste clipboard contents at the current text position."));

	appendMenuItem(menuEdit, wxID_CLEAR,
		       _("&Delete\tDel"),
		       _("Delete selected text."));

	menuEdit->AppendSeparator();

	appendMenuItem(menuEdit, myID_MENU_EDIT_QUICKFIND,
		       _("&Quick-Find ...\tCtrl+F"),
		       _("Find a string in the buffer."));

	appendMenuItem(menuEdit, wxID_FIND,
		       _("&Find ...\tCtrl+Shift+F"),
		       _("Open find dialog to search for a string in the buffer."));

	appendMenuItem(menuEdit, wxID_REPLACE,
		       _("&Replace ...\tCtrl+H"),
		       _("Open find and replace dialog to search for and replace a string in the buffer."));

	menuEdit->AppendSeparator();

	appendMenuItem(menuEdit, myID_MENU_EDIT_GOTO,
		       _("&Go to line ...\tCtrl+G"),
		       _("Jump to the entered line number."));

	menuEdit->AppendSeparator();

	appendMenuItem(menuEdit, wxID_SELECTALL,
		       _("&Select all\tCtrl+A"),
		       _("Select all text in the current buffer."));

	appendMenuItem(menuEdit, myID_MENU_EDIT_SELECTLINE,
		       _("Select &line\tCtrl+L"),
		       _("Select whole line at the current cursor position."));

	menuEdit->AppendSeparator();
	
	// Create Password Generator Submenu
	wxMenu* passgenmenu = new wxMenu;
 
	passgenmenu->Append(myID_MENU_EDIT_INSERT_PASSWORD_DIALOG,
			    _("Open Generator Dialog ...\tCtrl+P"),
			    _("Open random generator dialog box and insert the generated password."));

	passgenmenu->AppendSeparator();

	// Create wxMenuItem for PassGen Submenu
	wxMenuItem* passgenitem
	    = new wxMenuItem(menuEdit, myID_MENU_EDIT_INSERT_PASSWORD,
			     _("Insert &Password"),
			     _("Insert random password from generator preset or open generator dialog box."),
			     wxITEM_NORMAL, passgenmenu);
	
	passgenitem->SetBitmap( BitmapCatalog::GetMenuBitmap(myID_MENU_EDIT_INSERT_PASSWORD) );
	menuEdit->Append(passgenitem);

	menubar->Append(menuEdit, _("&Edit"));

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

	menuView->AppendSeparator();

	{
	    // *** View -> Zoom Level

	    wxMenu *menuZoom = new wxMenu;

	    appendMenuItem(menuZoom, myID_MENU_VIEW_ZOOM_INCREASE,
			   _("&Increase Zoom +1pt\tCtrl++"),
			   _("Increase the font zoom size by one point."));

	    appendMenuItem(menuZoom, myID_MENU_VIEW_ZOOM_DECREASE,
			   _("&Decrease Zoom -1pt\tCtrl+-"),
			   _("Decrease the font zoom size by one point."));

	    appendMenuItem(menuZoom, myID_MENU_VIEW_ZOOM_RESET,
			   _("&Reset Zoom\tCtrl+0"),
			   _("Reset the font zoom size to the normal pointsize."));

	    wxMenuItem* zoomitem
		= new wxMenuItem(menuEdit, myID_MENU_VIEW_ZOOM,
				 _("&Zoom Level ..."),
				 _("Change zoom level of text. Added or substracted to font point size."),
				 wxITEM_NORMAL, menuZoom);
	    zoomitem->SetBitmap( BitmapCatalog::GetMenuBitmap(myID_MENU_VIEW_ZOOM) );
	    menuView->Append(zoomitem);
	}

	menubar->Append(menuView, _("&View"));
    }

    if (page == CLASSINFO(WBinaryPage))
    {
	// *** Edit

	wxMenu *menuEdit = new wxMenu;

	appendMenuItem(menuEdit, myID_MENU_EDIT_GOTO,
		       _("&Go to Offset ...\tCtrl+G"),
		       _("Jump to the entered offset."));

	menubar->Append(menuEdit, _("&Edit"));
    }

    // *** Help

    wxMenu *menuHelp = new wxMenu;

    appendMenuItem(menuHelp, wxID_ABOUT,
		   _("&About ...\tShift+F1"),
		   _("Show some information about CryptoTE."));

    menubar->Append(menuHelp, _("&Help"));

    UpdateMenuInsertPassword();

    return menubar;
}

static inline void appendTool(wxToolBar* toolbar, int toolId, const wxString& label, wxItemKind kind = wxITEM_NORMAL, const wxString& longHelpString = wxEmptyString)
{
    toolbar->AddTool(toolId, label,
		     BitmapCatalog::GetToolbarBitmap(toolId), wxNullBitmap,
		     kind, label, longHelpString);
}

void WCryptoTE::CreateToolBar()
{
    if (!toolbar)
    {
	toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
				wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER);
	toolbar->SetToolBitmapSize(wxSize(22, 22));
    }
    else {
	toolbar->ClearTools();
    }

    // *** Container

    appendTool(toolbar, wxID_OPEN, _("Open Container"), wxITEM_NORMAL,
	       _("Open an existing encrypted container in the editor."));

    appendTool(toolbar, wxID_SAVE, _("Save Container"), wxITEM_NORMAL,
	       _("Save the current encrypted container to disk."));

    appendTool(toolbar, wxID_SAVEAS, _("Save Container as ..."), wxITEM_NORMAL,
	       _("Choose a file name and save the current encrypted container to disk."));

    appendTool(toolbar, myID_MENU_CONTAINER_SHOWLIST, _("Show SubFile List"), wxITEM_CHECK,
	       _("Show list of subfiles contained in current encrypted container."));

    // *** SubFile

    if (cpage && cpage->IsKindOf(CLASSINFO(WTextPage)))
    {
	toolbar->AddSeparator();

	// *** Edit

	appendTool(toolbar, wxID_UNDO, _("Undo Operation"), wxITEM_NORMAL,
		   _("Undo the last change."));

	appendTool(toolbar, wxID_REDO, _("Redo Operation"), wxITEM_NORMAL,
		   _("Redo the previously undone change."));

	toolbar->AddSeparator();

	appendTool(toolbar, wxID_CUT, _("Cut Selection"), wxITEM_NORMAL,
		   _("Cut selected text into clipboard."));

	appendTool(toolbar, wxID_COPY, _("Copy Selection"), wxITEM_NORMAL,
		   _("Copy selected text into clipboard."));

	appendTool(toolbar, wxID_PASTE, _("Paste Clipboard"), wxITEM_NORMAL,
		   _("Paste clipboard contents at the current text position."));

	toolbar->AddSeparator();

	appendTool(toolbar, wxID_FIND, _("Find Text ..."), wxITEM_NORMAL,
		   _("Open find dialog to search for a string in the buffer."));

	appendTool(toolbar, wxID_REPLACE, _("Find and Replace Text ..."), wxITEM_NORMAL,
		   _("Open find and replace dialog to search for and replace a string in the buffer."));

	appendTool(toolbar, myID_MENU_EDIT_GOTO, _("Goto to Line ..."), wxITEM_NORMAL,
		   _("Jump to the entered line number."));

	appendTool(toolbar, myID_TOOL_EDIT_INSERT_PASSWORD, _("Insert Password ..."), wxITEM_NORMAL,
		   _("Insert random password from generator preset or open generator dialog box."));
    }

    if (cpage && cpage->IsKindOf(CLASSINFO(WBinaryPage)))
    {
	toolbar->AddSeparator();

	// *** Edit

	appendTool(toolbar, myID_MENU_EDIT_GOTO, _("Goto to Offset ..."), wxITEM_NORMAL,
		   _("Jump to the entered offset."));
    }

    // *** Help

    toolbar->Realize();

    wxAuiPaneInfo& toolpane = auimgr.GetPane(toolbar);
    if (toolpane.IsOk())
    {
	toolpane.BestSize(toolbar->GetBestSize());
	auimgr.Update();
    }
}

// *** Preference Variables ***

void WCryptoTE::LoadPreferences()
{
    wxConfigBase* cfg = wxConfigBase::Get();

    cfg->SetPath(_T("/cryptote"));
    
    cfg->Read(_T("bitmaptheme"), &prefs_bitmaptheme, 0);

    cfg->Read(_T("backups"), &prefs_makebackups, true);
    cfg->Read(_T("backupnum"), &prefs_backupnum, 5);

    cfg->Read(_T("autoclose"), &prefs_autoclose, false);
    cfg->Read(_T("autoclosetime"), &prefs_autoclosetime, 15);
    cfg->Read(_T("autocloseexit"), &prefs_autocloseexit, true);

#if defined(__WINDOWS__)
    bool default_sharelock = true;
#else
    bool default_sharelock = false;
#endif

    cfg->Read(_T("sharelock"), &prefs_sharelock, default_sharelock);
}

// *** Generic Events ***

bool WCryptoTE::AllowCloseModified()
{
    if (!IsModified()) return true;
    if (!container) return true;

    while(1)
    {
	wxString closestr;
	if (container_filename.IsOk())
	    closestr = wxString::Format(_("Save modified container \"%s\"?"), container_filename.GetFullName().c_str());
	else
	    closestr = _("Save untitled modified container?");

	WMessageDialog dlg(this, closestr, _("Close CryptoTE"),
			   wxICON_WARNING,
			   wxID_SAVE, wxID_NO, wxID_CANCEL);

	int id = dlg.ShowModal();

	if (id == wxID_SAVE)
	{
	    if (UserContainerSave()) return true;
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

// *** Menu Events ***

void WCryptoTE::OnMenuContainerOpen(wxCommandEvent& WXUNUSED(event))
{
    UserContainerOpen();
}

bool WCryptoTE::UserContainerOpen()
{
    if (!AllowCloseModified()) return false;

    wxFileDialog dlg(this,
		     _("Open Container"), wxEmptyString, wxEmptyString,
		     _("Encrypted Container (*.ect)|*.ect"),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() != wxID_OK) return false;

    return ContainerOpen( dlg.GetPath() );
}

void WCryptoTE::OnMenuContainerSave(wxCommandEvent& WXUNUSED(event))
{
    UserContainerSave();
}

bool WCryptoTE::UserContainerSave()
{
    if (!container_filename.IsOk()) {
	return UserContainerSaveAs();
    }

    return ContainerSaveAs( container_filename.GetFullPath() );
}

void WCryptoTE::OnMenuContainerSaveAs(wxCommandEvent& WXUNUSED(event))
{
    UserContainerSaveAs();
}

bool WCryptoTE::UserContainerSaveAs()
{
    if (!container) return false;

    wxFileDialog dlg(this,
		     _("Save Container"), wxEmptyString, container_filename.GetFullName(),
		     _("Encrypted Container (*.ect)|*.ect"),
		     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK) return false;

    wxFileName fname(dlg.GetPath());

    // set extension .ect if none was entered and the file dialog filter is set.
    if (fname.GetExt().IsEmpty() && dlg.GetFilterIndex() == 0) {
	fname.SetExt(wxT("ect"));
    }

    return ContainerSaveAs( fname.GetFullPath() );
}

void WCryptoTE::OnMenuContainerRevert(wxCommandEvent& WXUNUSED(event))
{
    if (!AllowCloseModified()) return;

    if (!container_filename.IsOk()) {
	ContainerNew();
    }
    else {
	ContainerOpen( container_filename.GetFullPath() );
    }
}

void WCryptoTE::OnMenuContainerClose(wxCommandEvent& WXUNUSED(event))
{
    if (!AllowCloseModified()) return;

    ContainerNew();
}

void WCryptoTE::OnMenuContainerShowList(wxCommandEvent& WXUNUSED(event))
{
    ShowFilelistPane( !auimgr.GetPane(filelistpane).IsShown() );
}

void WCryptoTE::OnMenuContainerProperties(wxCommandEvent& WXUNUSED(event))
{
    if (!container) return;

    WContainerProperties dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
	SetModified();
    }
}

void WCryptoTE::OnMenuContainerSetPassword(wxCommandEvent& WXUNUSED(event))
{
    wxString filename = container_filename.IsOk() ? container_filename.GetFullName() : wxString(_("Untitled.ect"));

    WSetPassword passdlg(this, filename);
    if (passdlg.ShowModal() != wxID_OK) return;

    container->SetKey( strWX2STL(passdlg.GetPass()) );

    SetModified();
}

void WCryptoTE::OnMenuContainerPreferences(wxCommandEvent& WXUNUSED(event))
{
    WPreferences dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
	LoadPreferences();

	Freeze();

	if (prefs_bitmaptheme != BitmapCatalog::GetSingleton()->GetCurrentTheme())
	{
	    // reload all images
	    BitmapCatalog::GetSingleton()->SetTheme(prefs_bitmaptheme);

	    // Rebuild menus
	    SetMenuBar(NULL);

	    menubar_plain = CreateMenuBar(NULL);
	    menubar_textpage = CreateMenuBar(CLASSINFO(WTextPage));
	    menubar_binarypage = CreateMenuBar(CLASSINFO(WBinaryPage));

	    if (cpage->IsKindOf(CLASSINFO(WTextPage)))
	    {
		menubar_active = menubar_textpage;
	    }
	    else if (cpage->IsKindOf(CLASSINFO(WBinaryPage)))
	    {
		menubar_active = menubar_binarypage;
	    }
	    else
	    {
		menubar_active = menubar_plain;
	    }

	    SetMenuBar(menubar_active);

	    CreateToolBar();

	    // Force page and others to update their menu item status

	    menubar_active->Enable(myID_MENU_SUBFILE_EXPORT, true);
	    menubar_active->Enable(myID_MENU_SUBFILE_PROPERTIES, true);
	    menubar_active->Enable(myID_MENU_SUBFILE_CLOSE, true);

	    UpdateModified();
	    if (cpage) cpage->PageFocused();

	    // QuickFind and Goto Panes

	    quickfindbar->set_bitmaps();
	    quickgotobar->set_bitmaps();

	    filelistpane->BuildImageList();
	}

	// no need to check if idle timer changed, the user's mouse click will
	// have reset the timer anyway.

	if (!prefs_sharelock)
	{
	    // release file handle and thus the lock if it exists.
	    if (container_filehandle) {
		delete container_filehandle;
		container_filehandle = NULL;
	    }
	}

	Thaw();
    }
}

void WCryptoTE::OnMenuContainerQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void WCryptoTE::OnMenuSubFileNew(wxCommandEvent& WXUNUSED(event))
{
    if (!container) return;

    // Set up an empty text file in the container
    unsigned int sfnew = container->AppendSubFile();
    
    container->SetSubFileProperty(sfnew, "Name", strWX2STL(_("Untitled.txt")));
    container->SetSubFileProperty(sfnew, "Filetype", "text");
    container->SetSubFileProperty(sfnew, "Author", strWX2STL(wxGetUserId()));
    container->SetSubFileProperty(sfnew, "CTime", strTimeStampNow());

    // use defaults from global properties

    long defcomp = 0;
    if (!strSTL2WX(container->GetGlobalEncryptedProperty("DefaultCompression")).ToLong(&defcomp))
	defcomp = Enctain::COMPRESSION_ZLIB;

    long defencr = 0;
    if (!strSTL2WX(container->GetGlobalEncryptedProperty("DefaultEncryption")).ToLong(&defencr))
	defencr = Enctain::ENCRYPTION_SERPENT256;

    container->SetSubFileCompression(sfnew, (Enctain::compression_t)defcomp);
    container->SetSubFileEncryption(sfnew, (Enctain::encryption_t)defencr);

    filelistpane->ResetItems();

    OpenSubFile(sfnew);

    UpdateStatusBar(_("Created new empty text subfile in container."));
    SetModified();

    if (cpage) cpage->SetFocus();
}

void WCryptoTE::OnMenuSubFileImport(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dlg(this,
		     _("Import File(s)"), wxEmptyString, wxEmptyString,
		     _("Text File (*.txt)|*.txt|Any Text File (*.txt;*)|*.txt;*|Any Binary File (*)|*"),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (dlg.ShowModal() != wxID_OK) return;

    wxArrayString importlist;
    dlg.GetPaths(importlist);

    std::string filetype;
    if (dlg.GetFilterIndex() == 0 || dlg.GetFilterIndex() == 1) {
	filetype = "text";
    }

    ImportSubFiles(importlist, filetype, true);
}

void WCryptoTE::OnMenuSubFileExport(wxCommandEvent& WXUNUSED(event))
{
    if (!container) return;
    if (!cpage || cpage->subfileid < 0) return;

    wxString suggestname = strSTL2WX(container->GetSubFileProperty(cpage->subfileid, "Name"));

    wxFileDialog dlg(this,
		     _("Save SubFile"), wxEmptyString, suggestname,
		     _("Any file (*)|*"),
		     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK) return;

    wxFile outfile(dlg.GetPath(), wxFile::write);
    if (!outfile.IsOpened()) return;

    {
	wxFileOutputStream outstream(outfile);
	ExportSubFile(cpage->subfileid, outstream);
    }

    UpdateStatusBar(wxString::Format(_("Wrote %u bytes from subfile \"%s\" to %s"),
				     (unsigned int)(outfile.Tell()),
				     suggestname.c_str(),
				     dlg.GetPath().c_str()));
}

void WCryptoTE::OnMenuSubFileProperties(wxCommandEvent& WXUNUSED(event))
{
    if (!cpage || cpage->subfileid < 0) return;

    WFileProperties dlg(this, cpage->subfileid);
    if (dlg.ShowModal() == wxID_OK)
    {
	UpdateSubFileCaption(cpage->subfileid);
	SetModified();

	filelistpane->UpdateItem(cpage->subfileid);
    }
}

void WCryptoTE::OnMenuSubFileClose(wxCommandEvent& WXUNUSED(event))
{
    if (!cpage) return;

    int pi = auinotebook->GetPageIndex(cpage);
    if (pi == wxNOT_FOUND) return;

    // PageClose event is not generated by wxAuiNotebook

    if (auinotebook->GetPageCount() == 1)
    {
	// will be empty after the last page is closed
	UpdateNotebookPageChanged(-1, NULL);
    }

    auinotebook->DeletePage(pi);
}

void WCryptoTE::OnMenuEditGeneric(wxCommandEvent& event)
{
    // This is actually very dangerous: the page window MUST process the event
    // otherwise it will be passed up to the parent window, and will be caught
    // by this event handler and passed down again, creating an infinite loop.

    if (cpage && cpage->IsKindOf(CLASSINFO(WTextPage)))
    {
	cpage->ProcessEvent(event);
    }
}

void WCryptoTE::OnMenuEditQuickFind(wxCommandEvent& WXUNUSED(event))
{
    WTextPage* page = wxDynamicCast(cpage, WTextPage);
    if (!page) return;

    if (quickfindbar_visible)
    {
	// pushing Ctrl+F again is equivalent to Search-Next

	quickfindbar->textctrlQuickFind->SetFocus();
	
	wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

	page->PrepareQuickFind(false, false);

	page->DoQuickFind(false, findtext);
    }
    else
    {
	// make Quick-Find bar visible

	auimgr.GetPane(quickfindbar).Show();
	auimgr.GetPane(quickgotobar).Hide();
	auimgr.Update();

	quickgotobar_visible = false;
	quickfindbar_visible = true;

	page->PrepareQuickFind(false, true);

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

void WCryptoTE::OnMenuEditInsertPassword(wxCommandEvent& WXUNUSED(event))
{
    WTextPage* page = wxDynamicCast(cpage, WTextPage);
    if (!page) return;

    wpassgen->Centre();
    if (wpassgen->ShowModal() == wxID_OK)
    {
	page->AddText(wpassgen->GetSelectedPassword());
    }

    UpdateMenuInsertPassword();
}

void WCryptoTE::UpdateMenuInsertPassword()
{
    if (!menubar_textpage) return;
    if (!wpassgen) return;

    wxMenuItem* pgitem = menubar_textpage->FindItem(myID_MENU_EDIT_INSERT_PASSWORD);
    wxMenu* pgsubmenu = pgitem ? pgitem->GetSubMenu() : NULL;
    
    if (!pgitem || !pgsubmenu) {
	wxLogError(_T("Internal Menu Error: could not find insert-password submenu"));
	return;
    }

    // Delete all preset items
    {
	unsigned int pi = myID_MENU_EDIT_INSERT_PASSWORD_FIRST;
	wxMenuItem* presetitem = pgsubmenu->FindItem(pi);
	
	while(presetitem != NULL)
	{
	    pgsubmenu->Remove(presetitem);
	    delete presetitem;
	    presetitem = pgsubmenu->FindItem(++pi);
	}
    }

    // Append all presets
    for (unsigned int pi = 0; pi < wpassgen->presetlist.size(); ++pi)
    {
	const WPassGen::Preset& preset = wpassgen->presetlist[pi];
    
	pgsubmenu->Append(myID_MENU_EDIT_INSERT_PASSWORD_FIRST + pi,
			  preset.name,
			  wxString::Format(_("Insert a password generated with the preset %s"), preset.name.c_str()));
    }
}

void WCryptoTE::OnToolEditInsertPassword(wxCommandEvent& WXUNUSED(event))
{
    wxMenu* menu = new wxMenu(_("Password Presets"));
 
    for (unsigned int pi = 0; pi < wpassgen->presetlist.size(); ++pi)
    {
	const WPassGen::Preset& preset = wpassgen->presetlist[pi];
    
	menu->Append(myID_MENU_EDIT_INSERT_PASSWORD_FIRST + pi,
		     preset.name,
		     wxString::Format(_("Insert a password generated with the preset %s"), preset.name.c_str()));
    }

    menu->AppendSeparator();

    menu->Append(myID_MENU_EDIT_INSERT_PASSWORD_DIALOG,
		 _("Open Generator Dialog ..."),
		 _("Open random generator dialog box and insert the generated password."));

    PopupMenu(menu);
}

void WCryptoTE::OnMenuEditInsertPasswordPreset(wxCommandEvent& event)
{
    WTextPage* page = wxDynamicCast(cpage, WTextPage);
    if (!page) return;

    int id = event.GetId() - myID_MENU_EDIT_INSERT_PASSWORD_FIRST;

    if (id < 0 || id >= (int)wpassgen->presetlist.size()) return;

    const WPassGen::Preset& preset = wpassgen->presetlist[id];
    
    page->AddText(wpassgen->MakePassword(preset));
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

void WCryptoTE::OnMenuViewZoomIncrease(wxCommandEvent& WXUNUSED(event))
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetZoom( ctext->GetZoom() + 1 );
}

void WCryptoTE::OnMenuViewZoomDecrease(wxCommandEvent& WXUNUSED(event))
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetZoom( ctext->GetZoom() - 1 );
}

void WCryptoTE::OnMenuViewZoomReset(wxCommandEvent& WXUNUSED(event))
{
    if (!cpage || !cpage->IsKindOf(CLASSINFO(WTextPage))) return;
    WTextPage* ctext = (WTextPage*)cpage;

    ctext->SetZoom(0);
}

void WCryptoTE::OnMenuHelpAbout(wxCommandEvent& WXUNUSED(event))
{
    WAbout dlg(this);
    dlg.ShowModal();
}

void WCryptoTE::OnAccelEscape(wxCommandEvent& WXUNUSED(event))
{
    HideQuickBars();

    if (cpage) cpage->SetFocus();
}

// *** wxAuiManager Callbacks ***

void WCryptoTE::OnAuiManagerPaneClose(wxAuiManagerEvent& event)
{
    if (event.GetPane() && event.GetPane()->window == filelistpane)
    {
        toolbar->ToggleTool(myID_MENU_CONTAINER_SHOWLIST, false);
    }
}

// *** wxAuiNotebook Callbacks ***

void WCryptoTE::OnNotebookPageChanged(wxAuiNotebookEvent& event)
{
    // printf("Debug: OnNotebookPageChanged() to %d event\n", event.GetSelection());

    WNotePage* sel = wxDynamicCast(auinotebook->GetPage( event.GetSelection() ), WNotePage);

    if (sel)
    {
	UpdateNotebookPageChanged(event.GetSelection(), sel);
    }
    else
    {
	wxLogError(_T("Invalid notebook page activated."));
    }
}

void WCryptoTE::OnNotebookPageClose(wxAuiNotebookEvent& event)
{
    printf("Debug: OnNotebookPageClose() event\n");

    WNotePage* sel = wxDynamicCast(auinotebook->GetPage( event.GetSelection() ), WNotePage);

    if (sel)
    {
	sel->PageClosed();
    }

    if (auinotebook->GetPageCount() == 1)
    {
	// will be empty after the last page is closed
	UpdateNotebookPageChanged(-1, NULL);
    }
}

void WCryptoTE::UpdateNotebookPageChanged(int pageid, WNotePage* page)
{
    printf("Debug: UpdateNotebookPageChanged() event for %d page %p\n", pageid, page);

    if (cpageid == pageid && cpage == page) return;

    if (cpage) cpage->PageBlurred();

    cpage = page;
    cpageid = pageid;

    Freeze();

    if (cpage)
    {
	cpage->PageFocused();

	if (cpage->IsKindOf(CLASSINFO(WTextPage)))
	{
	    menubar_active = menubar_textpage;
	}
	else if (cpage->IsKindOf(CLASSINFO(WBinaryPage)))
	{
	    menubar_active = menubar_binarypage;
	}
	else
	{
	    menubar_active = menubar_plain;
	}

	SetMenuBar(menubar_active);
	CreateToolBar();

	menubar_active->Enable(myID_MENU_SUBFILE_EXPORT, true);
	menubar_active->Enable(myID_MENU_SUBFILE_PROPERTIES, true);
	menubar_active->Enable(myID_MENU_SUBFILE_CLOSE, true);
    }
    else
    {
	menubar_active = menubar_plain;
	SetMenuBar(menubar_active);
	CreateToolBar();

	menubar_active->Enable(myID_MENU_SUBFILE_EXPORT, false);
	menubar_active->Enable(myID_MENU_SUBFILE_PROPERTIES, false);
	menubar_active->Enable(myID_MENU_SUBFILE_CLOSE, false);

	// always show the file list pane if no file is open
	ShowFilelistPane(true);
    }

    Thaw();
}

void WCryptoTE::OnNotebookPageRightDown(wxAuiNotebookEvent& WXUNUSED(event))
{
    wxMenu* menu = new wxMenu;
 
    appendMenuItem(menu, myID_MENU_SUBFILE_EXPORT,
		   _("&Export SubFile ..."),
		   _("Export current subfile to disk."));
 
    appendMenuItem(menu, myID_MENU_SUBFILE_PROPERTIES,
		   _("&Properties ..."),
		   _("Show metadata properties of current subfile."));

    appendMenuItem(menu, myID_MENU_SUBFILE_CLOSE,
		   _("&Close SubFile"),
		   _("Close current subfile."));

    PopupMenu(menu);
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
    WTextPage* page = wxDynamicCast(cpage, WTextPage);
    if (!page) return;

    wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

    page->DoQuickFind(false, findtext);
}

void WCryptoTE::OnButtonQuickFindNext(wxCommandEvent& WXUNUSED(event))
{
    WTextPage* page = wxDynamicCast(cpage, WTextPage);
    if (!page) return;

    wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

    page->PrepareQuickFind(false, false);

    page->DoQuickFind(false, findtext);
}

void WCryptoTE::OnButtonQuickFindPrev(wxCommandEvent& WXUNUSED(event))
{
    WTextPage* page = wxDynamicCast(cpage, WTextPage);
    if (!page) return;

    wxString findtext = quickfindbar->textctrlQuickFind->GetValue();

    page->PrepareQuickFind(true, false);

    page->DoQuickFind(true, findtext);
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

void WCryptoTE::OnIdleTimerCheck(wxTimerEvent& WXUNUSED(event))
{
    if (lastuserevent == 0) return;

    long timenow = ::wxGetLocalTime();

    if (prefs_autoclose)
    {
	long timedelta = timenow - lastuserevent;

	if (timedelta >= prefs_autoclosetime * 60)
	{
	    lastuserevent = 0; // disable timer before processing, only one
			       // idle-timeout per user-event.

	    // cannot auto-close if the filename is unset.
	    if (!container_filename.IsOk()) {
		UpdateStatusBar(_("Inactivity time elapsed. But could not auto-save container: no default file name set."));
		return;
	    }

	    if (IsModified()) {
		// error during save?
		if (!UserContainerSave()) return;
	    }
	    else {
		UpdateStatusBar(_("Inactivity time elapsed. No modifications to save."));
	    }

	    if (prefs_autocloseexit) {
		Close();
	    }
	    else {
		ContainerNew();
	    }
	}
	else if (timedelta > 60)
	{
	    if (idletimestatusbar.IsEmpty())
		idletimestatusbar = statusbar->GetStatusText(1);

	    wxString sb = wxString::Format(_("Auto-close in %ds"), prefs_autoclosetime * 60 - timedelta);
	    statusbar->SetStatusText(sb, 1);
	}
    }
}

void WCryptoTE::ResetIdleTimer()
{
    lastuserevent = ::wxGetLocalTime();

    if (!idletimestatusbar.IsEmpty())
    {
	statusbar->SetStatusText(idletimestatusbar, 1);
	idletimestatusbar.Clear();
    }
}

BEGIN_EVENT_TABLE(WCryptoTE, wxFrame)

    // *** Generic Events

    EVT_CLOSE	(WCryptoTE::OnClose)

    EVT_TIMER	(myID_TIMER_IDLECHECK, WCryptoTE::OnIdleTimerCheck)

    // *** Menu Items

    // Container
    EVT_MENU	(wxID_OPEN,		WCryptoTE::OnMenuContainerOpen)
    EVT_MENU	(wxID_SAVE,		WCryptoTE::OnMenuContainerSave)
    EVT_MENU	(wxID_SAVEAS,		WCryptoTE::OnMenuContainerSaveAs)
    EVT_MENU	(wxID_REVERT,		WCryptoTE::OnMenuContainerRevert)
    EVT_MENU	(wxID_CLOSE,		WCryptoTE::OnMenuContainerClose)

    EVT_MENU	(myID_MENU_CONTAINER_SHOWLIST, WCryptoTE::OnMenuContainerShowList)
    EVT_MENU	(wxID_PROPERTIES,	WCryptoTE::OnMenuContainerProperties)
    EVT_MENU	(myID_MENU_CONTAINER_SETPASS, WCryptoTE::OnMenuContainerSetPassword)

    EVT_MENU	(wxID_PREFERENCES,	WCryptoTE::OnMenuContainerPreferences)

    EVT_MENU	(wxID_EXIT,		WCryptoTE::OnMenuContainerQuit)

    // SubFile
    EVT_MENU	(myID_MENU_SUBFILE_NEW, WCryptoTE::OnMenuSubFileNew)
    EVT_MENU	(myID_MENU_SUBFILE_IMPORT, WCryptoTE::OnMenuSubFileImport)
    EVT_MENU	(myID_MENU_SUBFILE_EXPORT, WCryptoTE::OnMenuSubFileExport)
    EVT_MENU	(myID_MENU_SUBFILE_PROPERTIES, WCryptoTE::OnMenuSubFileProperties)
    EVT_MENU	(myID_MENU_SUBFILE_CLOSE, WCryptoTE::OnMenuSubFileClose)

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

    EVT_MENU	(myID_MENU_EDIT_INSERT_PASSWORD_DIALOG, WCryptoTE::OnMenuEditInsertPassword)
    EVT_TOOL	(myID_TOOL_EDIT_INSERT_PASSWORD, WCryptoTE::OnToolEditInsertPassword)
    EVT_MENU_RANGE(myID_MENU_EDIT_INSERT_PASSWORD_FIRST, myID_MENU_EDIT_INSERT_PASSWORD_FIRST + 999,
		   WCryptoTE::OnMenuEditInsertPasswordPreset)

    // View
    EVT_MENU	(myID_MENU_VIEW_LINEWRAP,	WCryptoTE::OnMenuViewLineWrap)
    EVT_MENU	(myID_MENU_VIEW_LINENUMBER,	WCryptoTE::OnMenuViewLineNumber)
    EVT_MENU	(myID_MENU_VIEW_WHITESPACE,	WCryptoTE::OnMenuViewWhitespace)
    EVT_MENU	(myID_MENU_VIEW_ENDOFLINE,	WCryptoTE::OnMenuViewEndOfLine)
    EVT_MENU	(myID_MENU_VIEW_INDENTGUIDE,	WCryptoTE::OnMenuViewIndentGuide)
    EVT_MENU	(myID_MENU_VIEW_LONGLINEGUIDE,	WCryptoTE::OnMenuViewLonglineGuide)

    EVT_MENU	(myID_MENU_VIEW_ZOOM_INCREASE,	WCryptoTE::OnMenuViewZoomIncrease)
    EVT_MENU	(myID_MENU_VIEW_ZOOM_DECREASE,	WCryptoTE::OnMenuViewZoomDecrease)
    EVT_MENU	(myID_MENU_VIEW_ZOOM_RESET,	WCryptoTE::OnMenuViewZoomReset)

    // Help
    EVT_MENU	(wxID_ABOUT,		WCryptoTE::OnMenuHelpAbout)

    // *** Accelerators

    EVT_MENU	(myID_ACCEL_ESCAPE,	WCryptoTE::OnAccelEscape)

    // *** wxAuiManager Callbacks

    EVT_AUI_PANE_CLOSE(WCryptoTE::OnAuiManagerPaneClose)

    // *** wxAuiNotebook Callbacks

    EVT_AUINOTEBOOK_PAGE_CHANGED(myID_AUINOTEBOOK, WCryptoTE::OnNotebookPageChanged)
    EVT_AUINOTEBOOK_PAGE_CLOSE(myID_AUINOTEBOOK, WCryptoTE::OnNotebookPageClose)
#if wxCHECK_VERSION(2,8,7)
    EVT_AUINOTEBOOK_TAB_RIGHT_DOWN(myID_AUINOTEBOOK, WCryptoTE::OnNotebookPageRightDown)
#endif

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

// *** WStatusBar ***

WStatusBar::WStatusBar(wxWindow *_parent)
    : wxStatusBar(_parent, wxID_ANY, 0),
      parent(_parent)
{
    static const int statusbar_widths[3] = { -1, 150, 28 };

    SetFieldsCount(3);
    SetStatusWidths(3, statusbar_widths);

    #include "art/unmodified-16.h"

    lockbitmap = new wxStaticBitmap(this, wxID_ANY, wxIconFromMemory(unmodified_16_png));

    panelProgress = new wxWindow(this, wxID_ANY);
    labelProgress = new wxStaticText(panelProgress, wxID_ANY, _("Reencrypting:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    gaugeProgress = new wxGauge(panelProgress, wxID_ANY, 20, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH);

    sizerProgress = new wxBoxSizer(wxHORIZONTAL);
    sizerProgress->Add(labelProgress, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    sizerProgress->Add(gaugeProgress, 1, wxALL|wxEXPAND, 2);
    panelProgress->SetSizer(sizerProgress);
}

void WStatusBar::OnSize(wxSizeEvent& event)
{
    // move progress panel
    {
	wxRect rect;
	GetFieldRect(0, rect);

	panelProgress->SetSize(rect);
	panelProgress->Layout();
    }

    // move bitmap to position
    {
	wxRect rect;
	GetFieldRect(2, rect);
	wxSize size = lockbitmap->GetSize();

	lockbitmap->Move(rect.x + (rect.width - size.x) / 2,
			 rect.y + (rect.height - size.y) / 2);
    }

    event.Skip();
}

void WStatusBar::SetModified(bool on)
{
    #include "art/modified-16.h"
    #include "art/unmodified-16.h"

    if (on) {
	lockbitmap->SetBitmap(wxIconFromMemory(modified_16_png));
	lockbitmap->SetToolTip(_("Container is modified."));
    }
    else {
	lockbitmap->SetBitmap(wxIconFromMemory(unmodified_16_png));
	lockbitmap->SetToolTip(_("Container is saved to disk and unmodified."));
    }
}

void WStatusBar::ProgressStart(const char* text, size_t value, size_t limit)
{
    panelProgress->Show();
    parent->Disable();

    labelProgress->SetLabel( wxString(text, wxConvUTF8) );
    panelProgress->Layout();

    gaugeProgress->SetRange(limit);
    gaugeProgress->SetValue(value);

    wxTheApp->Yield(true);
}

void WStatusBar::ProgressUpdate(size_t value)
{
    gaugeProgress->SetValue(value);
    Update();
}

void WStatusBar::ProgressStop()
{
    parent->Enable();
    panelProgress->Hide();

    wxTheApp->Yield(true);
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
    : wxPanel(_wmain->auinotebook),
      wmain(_wmain),
      subfileid(-1), page_modified(false)
{
}

void WNotePage::UpdateStatusBar(const wxString& str)
{
    wmain->UpdateStatusBar(str);
}

void WNotePage::SetModified(bool modified)
{
    page_modified = modified;

    wmain->UpdateSubFileModified(this, page_modified);
}

IMPLEMENT_ABSTRACT_CLASS(WNotePage, wxPanel);
