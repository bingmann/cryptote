/*******************************************************************************
 * src/cryptote/main.cpp
 *
 * Part of CryptoTE v0.0.0, see http://panthema.net/2007/cryptote
 *******************************************************************************
 * Copyright (C) 2008-2014 Timo Bingmann <tb@panthema.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 ******************************************************************************/

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/tokenzr.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>
#include <memory>

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
    /// Virtual callback function.
    virtual bool Output(const void* data, size_t datalen)
    {
        return (wxWrite(wxFile::fd_stdout, data, datalen) == (int)datalen);
    }
};

/** Outputs the incoming (text) data into a temporary file. */
class DataOutputTempFile : public Enctain::DataOutput
{
public:
    wxFileOutputStream filestream;

    DataOutputTempFile(const wxString& filename)
        : filestream(filename)
    { }

    /// Virtual callback function.
    virtual bool Output(const void* data, size_t datalen)
    {
        return filestream.Write(data, datalen).IsOk();
    }
};

#ifdef __WXMSW__
/** Helper class to access protected function DoGetDirectory() to get Program
 * Files directory on Windows */
class myStandardPaths : public wxStandardPaths
{
public:
    static const unsigned int myCSIDL_PROGRAM_FILES = 0x0026;

    static wxString GetProgramFilesDir()
    {
        return DoGetDirectory(myCSIDL_PROGRAM_FILES);
    }
};
#endif

class App : public wxApp
{
private:
    /// Flag if the program is in console mode
    bool consolemode;

    /// CryptoTE main dialog
    class WCryptoTE* wcryptote;

    /// File path to load initially
    wxString cmdlinefile;

    /// Password string given on command line
    wxString cmdlinepass;

    /// True if to show stand-alone password generator
    bool run_pwgen;

    /// Locale object holding translations
    MyLocale* m_locale;

    /// Enctain Library Initializer
    Enctain::LibraryInitializer enctain_init;

public:
    App(bool _consolemode = false)
        : wxApp(),
          consolemode(_consolemode),
          wcryptote(NULL),
          run_pwgen(false),
          m_locale(NULL)
    { }

    /// Virtual override function to switch between console and gui mode
    virtual bool Initialize(int& argc, wxChar** argv)
    {
        if (consolemode)
            return wxAppBase::Initialize(argc, argv);
        else
            return wxApp::Initialize(argc, argv);
    }

    /// This function is called during application start-up.
    virtual bool OnInit()
    {
        wxLog::SetActiveTarget(new wxLogStderr);

        SetAppName(_T("CryptoTE"));
        SetVendorName(_T("idlebox.net"));

        // Setup locale to system default

        m_locale = new MyLocale(wxLANGUAGE_DEFAULT, wxLOCALE_CONV_ENCODING);

        // Load and initialize the catalog
        if (!m_locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs) ||
            !m_locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
        {
            // Could not load message catalog for system language, falling back
            // to English.

            delete m_locale;
            m_locale = new MyLocale(_T("en_GB"), _T("en"), _T("C"), true, true);

            if (!m_locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs) ||
                !m_locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
            {
                wxLogError(_T("Could not load message catalog for system or English language."));
                return false;
            }
        }

        // call parent-class for default behaviour and cmdline parsing
        if (!wxApp::OnInit()) return false;

        wxImage::AddHandler(new wxPNGHandler());

        wxLog::SetActiveTarget(NULL);

#if defined(__WXMSW__) || defined(__UNIX__)
        // Set up wxConfigBase storage: either to the system user-local storage
        // or to a local INI-file if the portable application is not in the
        // standard programs folder on Windows / Unix.
        {
#if defined(__WXMSW__)
            wxString exepath = wxStandardPaths::Get().GetExecutablePath().MakeLower();
            wxString stdpath = myStandardPaths::GetProgramFilesDir().MakeLower();
#else
            wxString exepath = wxStandardPaths::Get().GetExecutablePath();
            wxString stdpath = _T("/usr");
#endif

            if (!exepath.StartsWith(stdpath))
            {
                // construct a configuration file name local to the current
                // program's directory.

                wxFileName cfgpath(exepath);
                cfgpath.SetFullName(_T("CryptoTE.ini"));

                wxFileConfig* filecfg = new wxFileConfig(wxEmptyString, wxEmptyString,
                                                         cfgpath.GetFullPath(), cfgpath.GetFullPath());

                wxConfigBase::Set(filecfg);
            }
        }
#endif

        // Run desired program

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
            wcryptote = new WCryptoTE(NULL, m_locale);
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

    void OnInitCmdLine(wxCmdLineParser& parser)
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

        parser.AddSwitch(_T("s"), _T("shell"),
                         _("Start a simple shell to list and edit subfiles in container."),
                         0);

        parser.AddParam(_("container-file"),
                        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

        // must refuse '/' as parameter starter or cannot use "/path" style paths
        parser.SetSwitchChars(wxT("-"));
    }

    bool OnCmdLineParsed(wxCmdLineParser& parser)
    {
        if (parser.GetParamCount() > 0)
            cmdlinefile = parser.GetParam(0);

        parser.Found(_T("p"), &cmdlinepass);

        // First thing to do: set up language and locale
        wxString langtext;
        if (parser.Found(_T("L"), &langtext))
        {
            const wxLanguageInfo* langinfo = wxLocale::FindLanguageInfo(langtext);

            if (!langinfo) {
                wxLogError(_("Invalid language identifier specified with --lang."));
                return false;
            }

            if (m_locale) delete m_locale;
            m_locale = new MyLocale;

            if (!m_locale->Init(langinfo->Language, wxLOCALE_CONV_ENCODING)) {
                wxLogError(_("This language is not supported by the program."));
                return false;
            }

            // Load and initialize the catalog
            if (!m_locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs))
            {
                wxLogError(_("This language is not supported by the program."));
                return false;
            }

            // Load and initialize the catalog
            if (!m_locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
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

        if (parser.Found(_T("l")) || parser.Found(_T("d")) || parser.Found(_T("e")) || parser.Found(_T("s")))
        {
            if (cmdlinefile.IsEmpty()) {
                wxPuts(_("Operation --decrypt, --edit, --list or --shell requires a container file on the command line."));
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

            std::auto_ptr<Enctain::Container> container(new Enctain::Container);

            std::auto_ptr<wxFile> fh(new wxFile);

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
                while (1)
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
                    wxFgets(linepass, sizeof(linepass) - 1, stdin);

                    wxString linepassstr(linepass);
                    linepassstr.Truncate(linepassstr.Length() - 1);

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

                    try
                    {
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
                OnCmdLineList(container.get());
            }
            else if (parser.Found(_T("d"), &subfileindex))
            {
                if (subfileindex >= 0 && (unsigned int)subfileindex < container->CountSubFile())
                {
                    OnCmdLineDecryptCat(container.get(), subfileindex);
                }
                else
                {
                    wxPuts(_("Error during --decrypt: Invalid subfile index."));
                }
            }
            else if (parser.Found(_T("e"), &subfileindex))
            {
                if (subfileindex >= 0 && (unsigned int)subfileindex < container->CountSubFile())
                {
                    bool ed = OnCmdLineEdit(container.get(), subfileindex);

                    if (ed) {
                        OnCmdLineSaveContainer(container.get());
                    }
                }
                else
                {
                    wxPuts(_("Error during --edit: Invalid subfile index."));
                }
            }
            else if (parser.Found(_T("s")))
            {
                OnCmdLineShell(container.get());
            }

            return false;
        } // parser.Found(_T("l")) || parser.Found(_T("d"))  || parser.Found(_T("e")) )
        else if (consolemode)
        {
            wxPuts(_("Graphical Interface not available: check DISPLAY environment variable."));

            parser.Usage();
            return false;
        }

        return true;
    }

    void OnCmdLineList(Enctain::Container* container)
    {
        wxPrintf(_("Container has %d subfiles:\n"), container->CountSubFile());

        for (unsigned int sfid = 0; sfid < container->CountSubFile(); ++sfid)
        {
            wxPrintf(_("  %d: %s (%d bytes)\n"),
                     sfid,
                     strSTL2WX(container->GetSubFileProperty(sfid, "Name")).c_str(),
                     container->GetSubFileSize(sfid)
                     );
        }
    }

    void OnCmdLineDecryptCat(Enctain::Container* container, int subfileindex)
    {
        if (subfileindex < 0 || (unsigned int)subfileindex >= container->CountSubFile())
        {
            wxPuts(_("Error during cat: Invalid subfile index."));
            return;
        }

        try
        {
            DataOutputStdout dataout;
            container->GetSubFileData(subfileindex, dataout);
        }
        catch (Enctain::Exception& e)
        {
            wxPuts(WCryptoTE::EnctainExceptionString(e));
        }
    }

    void OnCmdLineSaveContainer(Enctain::Container* container)
    {
        wxPuts(_("Saving container."));

        wxFile fh;

        if (!fh.Create(cmdlinefile.c_str(), true, wxS_DEFAULT))
            return;

        try
        {
            wxFileOutputStream stream(fh);
            if (!stream.IsOk()) return;

            container->SetGlobalEncryptedProperty("MTime", strTimeStampNow());

            DataOutputStream dataout(stream);
            container->Save(dataout);

            wxPuts(_("OK"));
        }
        catch (Enctain::Exception& e)
        {
            wxPuts(WCryptoTE::EnctainExceptionString(e));
        }
    }

    bool OnCmdLineEdit(Enctain::Container* container, int subfileindex)
    {
        if (subfileindex < 0 || (unsigned int)subfileindex >= container->CountSubFile())
        {
            wxPuts(_("Error during edit: Invalid subfile index."));
            return false;
        }

        // Write data out into a temporary file

        wxString filename = strSTL2WX(container->GetSubFileProperty(subfileindex, "Name"));
        wxString tempname = wxString::Format(_T("temp-%d-%s"), (int)wxGetProcessId(), filename.c_str());

        try {
            DataOutputTempFile dataout(tempname);
            if (!dataout.filestream.IsOk()) return false;

            container->GetSubFileData(subfileindex, dataout);
        }
        catch (Enctain::Exception& e)
        {
            wxPuts(WCryptoTE::EnctainExceptionString(e));
            return false;
        }

        // Start editor

        time_t before_modtime = wxFileModificationTime(tempname);
        wxULongLong before_size = wxFileName::GetSize(tempname);

        wxChar* args[3];
        args[0] = wxGetenv(_T("EDITOR"));
        args[1] = const_cast<wxChar*>((const wxChar*)tempname.c_str());
        args[2] = NULL;

        long retcode = wxExecute(args, wxEXEC_SYNC | wxEXEC_NOHIDE);

        // Figure out what happened

        if (retcode == -1)
        {
            wxPuts(_("Could not launch console editor, check $EDITOR environment variable."));

            if (!wxRemoveFile(tempname)) {
                wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
            }

            return false;
        }
        else if (retcode != 0)
        {
            wxPrintf(_("Editor returned error code %d. Possible problem with editor.\n"), retcode);

            if (!wxRemoveFile(tempname)) {
                wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
            }

            return false;
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

            return false;
        }

        // Read file back from disk
        {
            wxFile filein(tempname, wxFile::read);
            if (!filein.IsOpened())
            {
                if (!wxRemoveFile(tempname)) {
                    wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
                }

                return false;
            }

            size_t fsize = after_size.GetLo();
            wxMemoryBuffer fdata(fsize);

            if ((size_t)filein.Read(fdata.GetData(), fsize) != fsize)
            {
                wxLogSysError(_("Error reading back temporary file data. Changes discarded."));

                if (!wxRemoveFile(tempname)) {
                    wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
                }

                return false;
            }

            wxPrintf(_("Saving changed data of subfile %d into container: %+d bytes\n"), subfileindex, (after_size - before_size).GetLo());

            try
            {
                container->SetSubFileData(subfileindex, fdata.GetData(), fsize);
            }
            catch (Enctain::Exception& e)
            {
                wxLogError(WCryptoTE::EnctainExceptionString(e));

                if (!wxRemoveFile(tempname)) {
                    wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
                }

                return false;
            }
        }

        if (!wxRemoveFile(tempname)) {
            wxPrintf(_("Error removing temporary file %s. It contains your sensitive data!\n"), tempname.c_str());
        }

        return true;
    }

    void OnCmdLineShell(Enctain::Container* container)
    {
        wxPrintf(_("Loaded container with %d subfiles.\n"), container->CountSubFile());
        wxPrintf(_("Launching shell: enter help if needed.\n"));

        while (1)
        {
            wxPrintf(_T("cryptote> "));

            wxChar linebuff[260];
            wxFgets(linebuff, sizeof(linebuff) - 1, stdin);

            wxString line(linebuff);
            line.Truncate(line.Length() - 1);

            wxStringTokenizer linetokens(line, _T(" "));

            wxString token = linetokens.GetNextToken();

            if (token == _T("h") || token == _T("?") || token == _T("help"))
            {
                wxPrintf(_("Shell commands:\n"));
                wxPrintf(_("    help        Show this shell command help.\n"));
                wxPrintf(_("    quit        Exit shell and possibly save changes to container.\n"));
                wxPrintf(_("    save        Save changes to container.\n"));
                wxPrintf(_("    list        List subfiles in opened container.\n"));
                wxPrintf(_("    cat <num>   Dump subfile number onto the console.\n"));
                wxPrintf(_("    edit <num>  Edit subfile number using standard editor.\n"));
            }
            else if (token == _T("quit") || token == _T("q") || token == _T("exit"))
            {
                break;
            }
            else if (token == _T("save") || token == _T("s"))
            {
                OnCmdLineSaveContainer(container);
            }
            else if (token == _T("list") || token == _T("l"))
            {
                OnCmdLineList(container);
            }
            else if (token == _T("cat") || token == _T("c"))
            {
                wxString sfstr = linetokens.GetNextToken();
                unsigned long subfileindex;

                if (!sfstr.ToULong(&subfileindex))
                {
                    wxPrintf(_("Error dumping subfile: command requires a subfile index.\n"));
                }
                else
                {
                    OnCmdLineDecryptCat(container, subfileindex);
                    wxPrintf(_T("\n"));
                }
            }
            else if (token == _T("edit") || token == _T("e"))
            {
                wxString sfstr = linetokens.GetNextToken();
                unsigned long subfileindex;

                if (!sfstr.ToULong(&subfileindex))
                {
                    wxPrintf(_("Error editing subfile: command requires a subfile index.\n"));
                }
                else
                {
                    OnCmdLineEdit(container, subfileindex);
                    wxPrintf(_T("\n"));
                }
            }
            else
            {
                wxPrintf(_("Unrecognized command '%s': see 'help'.\n"), token.c_str());
            }
        }

        if (container->GetModified())
        {
            while (1)
            {
                wxPrintf(_("Container was modified. Save? (y/n) "));

                wxChar linebuff[260];
                wxFgets(linebuff, sizeof(linebuff) - 1, stdin);

                wxString line(linebuff);
                line.Truncate(line.Length() - 1);

                if (line == _("y")) {
                    OnCmdLineSaveContainer(container);
                    break;
                }
                else if (line == _("n")) {
                    break;
                }
            }
        }
        else
        {
            wxPrintf(_("Container was not modified.\n"));
        }
    }

    virtual void MacOpenFile(const wxString& fileName)
    {
        if (!wcryptote)
        {
            // Create editor's main window frame
            wcryptote = new WCryptoTE(NULL, m_locale);
            SetTopWindow(wcryptote);
            wcryptote->Show();
        }

        if (!fileName.IsEmpty())
        {
            wcryptote->ContainerOpen(fileName);
        }
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

// This code starts up the application. Implements main() or WinMain(). Special
// code here to allow the program to function as a console program on Unix
// without X11 DISPLAY environment set.

#ifdef __WXMSW__

// This implements WinMain() or something. Always starts in graphics mode, ever
// seen a Windows without GUI?
IMPLEMENT_APP(App)

#else

wxAppConsole* wxCreateAppGui()
{
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "CryptoTE");
    return new App;
}

wxAppConsole * wxCreateAppConsole()
{
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "CryptoTE");
    return new App(true);
}

// Implement main() ourselves: check for DISPLAY and fall back to console mode
// when it is missing.

int main(int argc, char** argv)
{
    wxAppInitializerFunction createfunc = wxCreateAppGui;

    if (!getenv("DISPLAY"))
    {
        createfunc = wxCreateAppConsole;
    }

    wxAppInitializer wxTheAppInitializer(createfunc);
    return wxEntry(argc, argv);
}

#endif

/******************************************************************************/
