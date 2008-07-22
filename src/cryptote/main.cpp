// $Id$

#include <wx/wx.h>
#include <wx/cmdline.h>

#include "wcryptote.h"
#include "pwgen/wpassgen.h"
#include "common/myintl.h"
#include "common/tools.h"
#include "locale/de.h"
#include "locale/wxstd/de.h"

#ifdef __UNIX__
#include <termios.h> 
#endif

static MyLocaleMemoryCatalog cryptote_catalogs[] =
{
    { _T("de"), NULL, locale_de_mo, sizeof(locale_de_mo), locale_de_mo_uncompressed },
    { NULL, NULL, NULL, 0, 0 }
};

static MyLocaleMemoryCatalog wxstd_catalogs[] =
{
    { _T("de"), NULL, locale_wxstd_de_mo, sizeof(locale_wxstd_de_mo), locale_wxstd_de_mo_uncompressed },
    { NULL, NULL, NULL, 0, 0 }
};

/** Outputs the incoming (text) data to stdout. */
class DataOutputStdout : public Enctain::DataOutput
{
public:
    wxFileOutputStream	fstdout;

    DataOutputStdout()
	: fstdout(wxFile::fd_stdout)
    {
    }

    /// Virtual callback function.
    virtual bool Output(const void* data, size_t datalen)
    {
	return fstdout.Write(data, datalen).IsOk();
    }
};

/** Outputs the incoming (text) data into a temporary file. */
class DataOutputTempFile : public Enctain::DataOutput
{
public:
    wxFileOutputStream	filestream;

    DataOutputTempFile(const wxString& filename)
	: filestream(filename)
    {
    }

    /// Virtual callback function.
    virtual bool Output(const void* data, size_t datalen)
    {
	return filestream.Write(data, datalen).IsOk();
    }
};

class App : public wxApp
{
private:
    /// CryptoTE main dialog
    class WCryptoTE*	wcryptote;

    /// File path to load initially
    wxString		cmdlinefile;

    /// Password string given on command line
    wxString		cmdlinepass;

    /// True if to show stand-alone password generator
    bool		run_pwgen;

    /// Locale object holding translations
    MyLocale*		locale;

    /// Enctain Library Initializer
    Enctain::LibraryInitializer	enctain_init;

public:

    App()
	: wxApp(), wcryptote(NULL),
	  run_pwgen(false),
	  locale(NULL)
    {
    }

    /// This function is called during application start-up.
    virtual bool	OnInit()
    {
	wxLog::SetActiveTarget(new wxLogStderr);

	SetAppName(_T("CryptoTE"));
	SetVendorName(_T("idlebox.net"));

	// Setup locale to system default

	locale = new MyLocale(wxLANGUAGE_DEFAULT, wxLOCALE_CONV_ENCODING);

        // Load and initialize the catalog
	if (!locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs) ||
	    !locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
        {
	    // Could not load message catalog for system language, falling back
	    // to English.

	    delete locale;
	    locale = new MyLocale(wxLANGUAGE_ENGLISH, wxLOCALE_CONV_ENCODING);
	    
	    if (!locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs) ||
		!locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
	    {
		wxLogError(_T("Could not load message catalog for system or English language."));
		return false;
	    }
        }

	// call parent-class for default behaviour and cmdline parsing
	if (!wxApp::OnInit()) return false;

	wxImage::AddHandler(new wxPNGHandler());

	wxLog::SetActiveTarget(NULL);

	if (run_pwgen)
	{
	    // Create password generator's main window frame
	    WPassGen* wmain = new WPassGen(NULL, true);
	    SetTopWindow(wmain);
	    wmain->Show();
	}
	else
	{
	    // Create editor's main window frame
	    wcryptote = new WCryptoTE(NULL);
	    SetTopWindow(wcryptote);
	    wcryptote->Show();

	    if (!cmdlinefile.IsEmpty())
	    {
		wcryptote->ContainerOpen(cmdlinefile, cmdlinepass);
		cmdlinepass.Clear();
	    }
	}

	return true;
    }

    void	OnInitCmdLine(wxCmdLineParser& parser)
    {
        parser.AddSwitch(_T("h"), _T("help"),
			 _("Display help for the command line parameters."),
			 wxCMD_LINE_OPTION_HELP);

        parser.AddOption(_T("L"), _T("lang"),
			 _("Set language for messages. Example: de or de_DE."),
			 wxCMD_LINE_VAL_STRING, 0);

        parser.AddSwitch(_T("pwgen"), wxEmptyString,
			 _("Start stand-alone password generator dialog instead of the editor."),
			 0);

        parser.AddOption(_T("p"), _T("password"),
			 _("Use the given password to decrypt the initially loaded container."),
			 wxCMD_LINE_VAL_STRING, 0);

        parser.AddSwitch(_T("l"), _T("list"),
			 _("List subfiles in container."),
			 0);

        parser.AddOption(_T("d"), _T("decrypt"),
			 _("Decrypt subfile <num> and output it to stdout."),
			 wxCMD_LINE_VAL_NUMBER, 0);

        parser.AddOption(_T("e"), _T("edit"),
			 _("Edit subfile <num> using the default (console) editor."),
			 wxCMD_LINE_VAL_NUMBER, 0);

	parser.AddParam(_("container-file"),
			wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

	// must refuse '/' as parameter starter or cannot use "/path" style paths
	parser.SetSwitchChars(wxT("-"));
    }

    bool	OnCmdLineParsed(wxCmdLineParser& parser)
    {
	if (parser.GetParamCount() > 0)
	    cmdlinefile = parser.GetParam(0);

	parser.Found(_T("p"), &cmdlinepass);

	// First thing to do: set up language and locale
	wxString langtext;
	if ( parser.Found(_T("L"), &langtext) )
	{
	    const wxLanguageInfo* langinfo = wxLocale::FindLanguageInfo(langtext);

	    if (!langinfo) {
		wxLogError(_("Invalid language identifier specified with --lang."));
		return false;
	    }

	    if (locale) delete locale;
	    locale = new MyLocale;

	    if (!locale->Init(langinfo->Language, wxLOCALE_CONV_ENCODING)) {
		wxLogError(_("This language is not supported by the program."));
		return false;
	    }

	    // Load and initialize the catalog
	    if (!locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs))
	    {
		wxLogError(_("This language is not supported by the program."));
		return false;
	    }

	    // Load and initialize the catalog
	    if (!locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
	    {
		wxLogError(_("This language is not supported by the program."));
		return false;
	    }
	}

	if (parser.Found(_T("pwgen")))
	{
	    run_pwgen = true;
	}

	int exclusiveparam = (parser.Found(_T("l")) ? 1 : 0)
	    + (parser.Found(_T("d")) ? 1 : 0)
	    + (parser.Found(_T("e")) ? 1 : 0);

	if (exclusiveparam >= 2)
	{
	    wxPuts(_("The parameter --decrypt, --edit and --list cannot be combined."));
	    return false;
	}

	if ( parser.Found(_T("l")) || parser.Found(_T("d"))  || parser.Found(_T("e")) )
	{
	    if (cmdlinefile.IsEmpty()) {
		wxPuts(_("Operation --decrypt, --edit oder --list requires a container file on the command line."));
		return false;
	    }

	    if (parser.Found(_T("e")))
	    {
		if (wxGetenv(_T("EDITOR")) == NULL)
		{
		    wxPuts(_("Cannot --edit subfile: $EDITOR environment variable not set."));
		    return false;
		}
	    }

	    std::auto_ptr<Enctain::Container> container (new Enctain::Container);

	    std::auto_ptr<wxFile> fh (new wxFile);

	    if (!fh->Open(cmdlinefile.c_str(), wxFile::read))
	    {
		return false;
	    }

	    if (!cmdlinepass.IsEmpty())
	    {
		try {
		    wxFileInputStream stream(*fh.get());
		    if (!stream.IsOk()) return false;

		    DataInputStream datain(stream);
		    container->Load(datain, strWX2STL(cmdlinepass));
		}
		catch (Enctain::Exception& e)
		{
		    wxPuts(WCryptoTE::EnctainExceptionString(e));
		    return false;
		}
	    }
	    else
	    {
		while(1)
		{
#ifdef __UNIX__
		    // Disable terminal echo of password
		    struct termios term_attr;

		    if (tcgetattr(STDIN_FILENO, &term_attr) != 0) {
			wxLogSysError(_("Cannot get terminal attributes (tcgetattr)"));
			return false;
		    }

		    int old_flags = term_attr.c_lflag;
		    term_attr.c_lflag &= ~ECHO;

		    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr) != 0) {
			wxLogSysError(_("Cannot set terminal attributes (tcsetattr)"));
			return false;
		    }
#endif

		    wxPrintf(_("Container password: "));

		    wxChar linepass[260];
		    wxFgets(linepass, sizeof(linepass), stdin);

		    wxString linepassstr(linepass);
		    linepassstr.Truncate( linepassstr.Length() - 1 );

#ifdef __UNIX__
		    // Restore terminal echo flags
		    term_attr.c_lflag = old_flags;
		    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr) != 0) {
			wxLogSysError(_("Cannot restore terminal attributes (tcsetattr)"));
			return false;
		    }

		    wxPrintf(_T("\n"));
#endif

		    if (linepassstr.IsEmpty()) {
			wxPrintf(_("Aborted.\n"));
			return false;
		    }

		    try {
			fh->Seek(0, wxFromStart);
			wxFileInputStream stream(*fh.get());
			if (!stream.IsOk()) return false;

			DataInputStream datain(stream);
			container->Load(datain, strWX2STL(linepassstr));
		    }
		    catch (Enctain::Exception& e)
		    {
			if (e.code() == Enctain::ETE_LOAD_HEADER2_INVALID_KEY)
			{
			    wxPuts(_("Error loading container: cannot decrypt header. Possibly wrong password."));
			    wxPuts(_("Try again? Abort with an empty line."));
			    continue;
			}
			else
			{
			    wxPuts(WCryptoTE::EnctainExceptionString(e));
			    return false;
			}
		    }

		    break;
		}
	    }

	    long subfileindex;
	    if (parser.Found(_T("l")))
	    {
		wxPrintf(_("Container has %d subfiles:\n"), container->CountSubFile());

		for(unsigned int sfid = 0; sfid < container->CountSubFile(); ++ sfid)
		{
		    wxPrintf(_("  %d: %s (%d bytes)\n"),
			     sfid,
			     strSTL2WX(container->GetSubFileProperty(sfid, "Name")).c_str(),
			     container->GetSubFileSize(sfid)
			);
		}

		return false;
	    }
	    else if (parser.Found(_T("d"), &subfileindex))
	    {
		if (subfileindex >= 0 && (unsigned int)subfileindex < container->CountSubFile())
		{
		    try {
			DataOutputStdout dataout;
			container->GetSubFileData(subfileindex, dataout);
		    }
		    catch (Enctain::Exception& e)
		    {
			wxPuts(WCryptoTE::EnctainExceptionString(e));
			return false;
		    }
		}
		else
		{
		    wxPuts(_("Error during --decrypt: Invalid subfile index."));
		    return false;
		}
	    }
	    else if (parser.Found(_T("e"), &subfileindex))
	    {
		OnCmdLineEdit(container.get(), subfileindex, cmdlinefile);

		return false;
	    }
	    
	    return false;
	}

	return true;
    }

    void	OnCmdLineEdit(Enctain::Container *container, int subfileindex, const wxString& cmdlinefile)
    {
	if (subfileindex < 0 || (unsigned int)subfileindex >= container->CountSubFile())
	{
	    wxPuts(_("Error during --edit: Invalid subfile index."));
	    return;
	}

	// Write data out into a temporary file

	wxString filename = strSTL2WX(container->GetSubFileProperty(subfileindex, "Name"));
	wxString tempname = wxString::Format(_T("temp-%d-%s"), (int)wxGetProcessId(), filename.c_str());

	{
	    try {
		DataOutputTempFile dataout(tempname);
		if (!dataout.filestream.IsOk()) return;

		container->GetSubFileData(subfileindex, dataout);
	    }
	    catch (Enctain::Exception& e)
	    {
		wxPuts(WCryptoTE::EnctainExceptionString(e));
		return;
	    }
	}

	// Start editor

	time_t before_modtime = wxFileModificationTime(tempname);
	wxULongLong before_size = wxFileName::GetSize(tempname);

	wxChar* args[3];
	args[0] = wxGetenv(_T("EDITOR"));
	args[1] = const_cast<wxChar*>(tempname.c_str());
	args[2] = NULL;

	long retcode = wxExecute(args, wxEXEC_SYNC | wxEXEC_NOHIDE);

	// Figure out what happened 

	if (retcode == -1)
	{
	    wxPuts(_("Could not launch console editor, check $EDITOR environment variable."));

	    if (!wxRemoveFile(tempname)) {
		wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
	    }

	    return;
	}
	else if (retcode != 0)
	{
	    wxPrintf(_("Editor returned error code %d. Possible problem with editor.\n"), retcode);

	    if (!wxRemoveFile(tempname)) {
		wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
	    }

	    return;
	}

	time_t after_modtime = wxFileModificationTime(tempname);
	wxULongLong after_size = wxFileName::GetSize(tempname);

	if (before_modtime == after_modtime &&
	    before_size == after_size)
	{
	    wxPuts(_("File unchanged. Nothing to save."));

	    if (!wxRemoveFile(tempname)) {
		wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
	    }

	    return;
	}

	// Read file back from disk
	{
	    wxFile filein(tempname, wxFile::read);
	    if (!filein.IsOpened())
	    {
		if (!wxRemoveFile(tempname)) {
		    wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
		}

		return;
	    }

	    size_t fsize = after_size.GetLo();
	    wxMemoryBuffer fdata(fsize);

	    if ((size_t)filein.Read(fdata.GetData(), fsize) != fsize)
	    {
		wxLogSysError(_("Error reading back temporary file data. Changes discarded."));

		if (!wxRemoveFile(tempname)) {
		    wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
		}

		return;
	    }

	    wxPrintf(_("Saving changed data of subfile %d into container: %+d bytes\n"), subfileindex, (after_size - before_size).GetLo());

	    try {
		container->SetSubFileData(subfileindex, fdata.GetData(), fsize);
	    }
	    catch (Enctain::Exception& e)
	    {
		wxLogError(WCryptoTE::EnctainExceptionString(e));

		if (!wxRemoveFile(tempname)) {
		    wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
		}

		return;
	    }
	}
	
	if (!wxRemoveFile(tempname)) {
	    wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
	}

	// Write container to disk
	{
	    wxPuts(_("Saving container."));

	    wxFile fh;

	    if (!fh.Create(cmdlinefile.c_str(), true, wxS_DEFAULT))
		return;

	    try {
		wxFileOutputStream stream(fh);
		if (!stream.IsOk()) return;

		container->SetGlobalEncryptedProperty("MTime", strTimeStampNow());

		DataOutputStream dataout(stream);
		container->Save(dataout);
	    }
	    catch (Enctain::Exception& e)
	    {
		wxPuts(WCryptoTE::EnctainExceptionString(e));
		return;
	    }
	}

	wxPuts(_("OK"));
    }

    virtual int OnExit()
    {
	return 0;
    }

    static inline bool IsUserEvent(const wxEvent& event)
    {
	// keyboard events
	if (event.GetEventType() == wxEVT_KEY_DOWN) return true;

	// mouse events
	if (event.GetEventType() == wxEVT_LEFT_DOWN) return true;
	if (event.GetEventType() == wxEVT_MIDDLE_DOWN) return true;
	if (event.GetEventType() == wxEVT_RIGHT_DOWN) return true;
	if (event.GetEventType() == wxEVT_LEFT_DCLICK) return true;
	if (event.GetEventType() == wxEVT_MIDDLE_DCLICK) return true;
	if (event.GetEventType() == wxEVT_RIGHT_DCLICK) return true;
	if (event.GetEventType() == wxEVT_MOUSEWHEEL) return true;

	// some extra events
	if (event.GetEventType() == wxEVT_MENU_OPEN) return true;
	if (event.GetEventType() == wxEVT_COMMAND_MENU_SELECTED) return true;

	return false;
    }

    /// This function received all event before they are processed by the
    /// target object. It monitors user events: keyboard and mouse actions and
    /// resets the idle-timer in the main window.
    virtual int FilterEvent(wxEvent& event)
    {
	if (wcryptote)
	{
	    if (IsUserEvent(event))
		wcryptote->ResetIdleTimer();
	}

	return -1;
    }
};

// This implements main(), WinMain() or whatever
IMPLEMENT_APP(App)
