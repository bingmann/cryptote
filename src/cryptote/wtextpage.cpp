// $Id$

#include "wtextpage.h"
#include "common/tools.h"

#include <wx/file.h>
#include <wx/wfstream.h>
#include <stc.h>

WTextPage::WTextPage(class WCryptoTE* parent)
    : WNotePage(parent)
{
    view_linewrap = false;
    view_linenumber = false;
    view_whitespace = false;
    view_endofline = false;
    view_indentguide = false;
    view_longlineguide = false;

    // *** Create Control ***

    editctrl = new wxStyledTextCtrl(this, myID_EDITCTRL);
    editctrl->SetFocus();

    // *** Set up Sizer ***
 
    wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(editctrl, 1, wxEXPAND, 0);

    SetSizer(sizerMain);
    Layout();

    // *** Set some default styles ***

    wxFont font(10, wxMODERN, wxNORMAL, wxNORMAL);

    editctrl->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    editctrl->StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
    editctrl->StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
    editctrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(_T("DARK GREY")));
    editctrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(250,250,250));
    editctrl->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(_T("DARK GREY")));

    // Set Default View Options
    editctrl->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

    editctrl->SetWrapMode(wxSTC_WRAP_WORD);
    editctrl->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);

    // Set up margin for line numbers
    editctrl->SetMarginType(MARGIN_LINENUMBER, wxSTC_MARGIN_NUMBER);
    editctrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(_T("DARK GREY")));
    editctrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(250,250,250));
    editctrl->SetMarginWidth(MARGIN_LINENUMBER, 0); // set width initially to 0

    SetViewLineWrap(true);
}

wxString WTextPage::GetCaption()
{
    if (!wmain->container) return _T("Unknown");

    return strSTL2WX( wmain->container->GetSubFileProperty(subfileid, "Name") );
}

/** Appends the incoming text file into the Scintilla edit control. */
class TextPageAcceptor : public Enctain::DataAcceptor
{
public:
    class WTextPage&	tpage;

    /// Constructor is given all information
    TextPageAcceptor(WTextPage& _tpage)
	: tpage(_tpage)
    {
    }

    /// Virtual callback function to save data.
    virtual void Append(const void* data, size_t datalen)
    {
	tpage.editctrl->AddTextRaw((const char*)data, datalen);
    }
};

bool WTextPage::LoadSubFile(unsigned int sfid)
{
    editctrl->ClearAll();
    editctrl->Allocate(wmain->container->GetSubFileSize(sfid));
    editctrl->SetUndoCollection(false);

    TextPageAcceptor acceptor(*this);

    wmain->container->GetSubFileData(sfid, acceptor);

    subfileid = sfid;

    LoadSubFileMetaSettings(subfileid);

    editctrl->SetUndoCollection(true);
    editctrl->EmptyUndoBuffer();
    editctrl->SetSavePoint();
    editctrl->GotoPos(0);
    editctrl->ScrollToColumn(0); // extra help to ensure scrolled to 0
				 // otherwise scrolled halfway thru 1st char

    return true;
}

bool WTextPage::LoadSubFileMetaSettings(unsigned int sfid)
{
    std::string ms_str = wmain->container->GetSubFileProperty(sfid, "WTextPageSettings");
    if (ms_str.size() < 4) return false;

    uint32_t version = *(uint32_t*)(ms_str.data());
    
    if (version == 0x00000001 && ms_str.size() == sizeof(struct MetaSettingsv00000001))
    {
	const MetaSettingsv00000001 &ms = *(MetaSettingsv00000001*)(ms_str.data());

	SetViewLineWrap(ms.view_linewrap);
	SetViewLineNumber(ms.view_linenumber);
	SetViewWhitespace(ms.view_whitespace);
	SetViewEndOfLine(ms.view_endofline);
	SetViewIndentGuide(ms.view_indentguide);
	SetViewLonglineGuide(ms.view_longlineguide);

	PageFocused(); // update menubar view checkmarks

	return true;
    }
    else {
	wxLogError(_("Could not restore settings of the text editor, maybe you need to upgrade CryptoTE to a newer version?"));
	return false;
    }
}

void WTextPage::SaveSubFileMetaSettings(unsigned int sfid)
{
    MetaSettingsv00000001 ms;

    ms.version = 0x00000001;
    ms.view_linewrap = view_linewrap;
    ms.view_linenumber = view_linenumber;
    ms.view_whitespace = view_whitespace;
    ms.view_endofline = view_endofline;
    ms.view_indentguide = view_indentguide;
    ms.view_longlineguide = view_longlineguide;

    wmain->container->SetSubFileProperty(sfid, "WTextPageSettings", std::string((char*)&ms, sizeof(ms)));
}

size_t WTextPage::ImportFile(wxFile& file)
{
    SetViewLineWrap(false); // line wrapping seems to take so much processing
			    // time. we'll just disable it for imported
			    // files. let the user enable it if he really wants
			    // it.

    wxFileOffset filesize = file.Length();

    wmain->statusbar->ProgressStart("Importing", 0, filesize);

    editctrl->ClearAll();
    editctrl->Allocate(filesize);
    editctrl->SetUndoCollection(false);

    char buffer[65536];

    for (int i = 0; !file.Eof(); i++)
    {
	size_t rb = file.Read(buffer, sizeof(buffer));
	if (rb == 0) break;

	editctrl->AddTextRaw(buffer, rb);

	wmain->statusbar->ProgressUpdate(editctrl->GetTextLength());
    }

    editctrl->SetUndoCollection(true);
    editctrl->EmptyUndoBuffer();
    editctrl->GotoPos(0);
    editctrl->ScrollToColumn(0); // extra help to ensure scrolled to 0
				 // otherwise scrolled halfway thru 1st char

    wmain->statusbar->ProgressStop();

    return editctrl->GetTextLength();
}

void WTextPage::ExportBuffer(wxOutputStream& outstream)
{
    size_t buflen = editctrl->GetTextLength();
    wxCharBuffer buf = editctrl->GetTextRaw();

    outstream.Write(buf.data(), buflen);
}

// *** Event Handlers ***

void WTextPage::OnMenuEditUndo(wxCommandEvent& WXUNUSED(event))
{
    if (!editctrl->CanUndo()) {
	UpdateStatusBar(_("No more change operations to undo."));
	return;
    }

    editctrl->Undo();
}

void WTextPage::OnMenuEditRedo(wxCommandEvent& WXUNUSED(event))
{
    if (!editctrl->CanRedo()) {
	UpdateStatusBar(_("No more change operations to redo."));
	return;
    }

    editctrl->Redo();
}

void WTextPage::OnMenuEditCut(wxCommandEvent& WXUNUSED(event))
{
    if (editctrl->GetReadOnly()) {
	UpdateStatusBar(_("Buffer is read-only."));
	return;
    }
    if (editctrl->GetSelectionEnd() <= editctrl->GetSelectionStart()) {
	UpdateStatusBar(_("Nothing selected."));
	return;
    }

    int cutlen = editctrl->GetSelectionEnd() - editctrl->GetSelectionStart();

    editctrl->Cut();

    UpdateStatusBar( wxString::Format(_("Cut %u characters into clipboard."), cutlen) );
}

void WTextPage::OnMenuEditCopy(wxCommandEvent& WXUNUSED(event))
{
    if (editctrl->GetSelectionEnd() <= editctrl->GetSelectionStart()) {
	UpdateStatusBar(_("Nothing selected."));
	return;
    }

    int copylen = editctrl->GetSelectionEnd() - editctrl->GetSelectionStart();
    
    editctrl->Copy();

    UpdateStatusBar( wxString::Format(_("Copied %u characters into clipboard."), copylen) );
}

void WTextPage::OnMenuEditPaste(wxCommandEvent& WXUNUSED(event))
{
    if (!editctrl->CanPaste()) {
	UpdateStatusBar(_("Nothing pasted, the clipboard is empty."));
	return;
    }

    int prevlen = editctrl->GetTextLength();
    prevlen -= editctrl->GetSelectionEnd() - editctrl->GetSelectionStart();

    editctrl->Paste();

    UpdateStatusBar(
	wxString::Format(_("Pasted %u characters from clipboard."),
			 editctrl->GetTextLength() - prevlen)
	);
}

void WTextPage::OnMenuEditDelete(wxCommandEvent& WXUNUSED(event))
{
    if (editctrl->GetReadOnly()) {
	UpdateStatusBar(_("Buffer is read-only."));
	return;
    }
    if (editctrl->GetSelectionEnd() <= editctrl->GetSelectionStart()) {
	UpdateStatusBar(_("Nothing selected."));
	return;
    }

    int deletelen = editctrl->GetSelectionEnd() - editctrl->GetSelectionStart();

    editctrl->Clear();
    
    UpdateStatusBar( wxString::Format(_("Deleted %u characters from buffer."), deletelen) );
}

void WTextPage::OnMenuEditSelectAll(wxCommandEvent& WXUNUSED(event))
{
    editctrl->SetSelection(0, editctrl->GetTextLength());

    UpdateStatusBar(
	wxString::Format(_("Selected all %u characters in buffer."),
			 editctrl->GetTextLength())
	);
}

void WTextPage::OnMenuEditSelectLine(wxCommandEvent& WXUNUSED(event))
{
    int lineStart = editctrl->PositionFromLine(editctrl->GetCurrentLine());
    int lineEnd = editctrl->PositionFromLine(editctrl->GetCurrentLine() + 1);

    editctrl->SetSelection(lineStart, lineEnd);

    UpdateStatusBar(
	wxString::Format(_("Selected %u characters on line."),
			 lineEnd - lineStart)
	);
}

// *** Virtual Callbacks via WNotePage ***

void WTextPage::PageFocused()
{
    wxMenuBar* menubar = wmain->menubar;

    // Update Menu -> View with current options

    menubar->Check(WCryptoTE::myID_MENU_VIEW_LINEWRAP, view_linewrap);
    menubar->Check(WCryptoTE::myID_MENU_VIEW_LINENUMBER, view_linenumber);
    menubar->Check(WCryptoTE::myID_MENU_VIEW_WHITESPACE, view_whitespace);
    menubar->Check(WCryptoTE::myID_MENU_VIEW_ENDOFLINE, view_endofline);
    menubar->Check(WCryptoTE::myID_MENU_VIEW_INDENTGUIDE, view_indentguide);
    menubar->Check(WCryptoTE::myID_MENU_VIEW_LONGLINEGUIDE, view_longlineguide);

    // Synthesize UpdateUI event
    wxStyledTextEvent event;
    OnScintillaUpdateUI(event);
}

void WTextPage::PageBlurred()
{
}

void WTextPage::PageSaveData()
{
    size_t buflen = editctrl->GetTextLength();
    wxCharBuffer buf = editctrl->GetTextRaw();

    wmain->container->SetSubFileData(subfileid, buf.data(), buflen);

    SaveSubFileMetaSettings(subfileid);

    editctrl->SetSavePoint();

    size_t savelen = wmain->container->GetSubFileStorageSize(subfileid);

    if (savelen != buflen)
    {
	UpdateStatusBar(
	    wxString::Format(_("Compressed %u characters from buffer into %u bytes for encrypted storage."),
			     buflen, savelen)
	    );
    }
    else
    {
	UpdateStatusBar(
	    wxString::Format(_("Saving %u characters from buffer for encrypted storage."),
			     buflen)
	    );
    }

    // Page Modified Flag is automatically cleared though SavePoint callbacks
    wmain->SetModified();
}

void WTextPage::PageClosed()
{
    PageSaveData();
}

void WTextPage::PrepareQuickFind(bool backwards, bool reset)
{
    if (reset)
    {
	quickfind_startpos = editctrl->GetCurrentPos();
    }
    else
    {
	if (!backwards)
	{
	    quickfind_startpos = editctrl->GetSelectionEnd();
	}
	else
	{
	    quickfind_startpos = editctrl->GetSelectionStart();
	}
    }
}

void WTextPage::DoQuickFind(bool backwards, const wxString& findtext)
{
    if (findtext.IsEmpty())
    {
	// move cursor and screen back to search start position
	
	editctrl->SetSelection(quickfind_startpos, quickfind_startpos);
	
	UpdateStatusBar( wxT("") );
	
	wmain->quickfindbar->textctrlQuickFind->SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW) );
	wmain->quickfindbar->textctrlQuickFind->Refresh();

	return;
    }

    if (!backwards)
    {
	editctrl->SetTargetStart(quickfind_startpos);
	editctrl->SetTargetEnd(editctrl->GetLength());
    }
    else
    {
	editctrl->SetTargetStart(quickfind_startpos);
	editctrl->SetTargetEnd(0);
    }

    editctrl->SetSearchFlags(0);

    int respos = editctrl->SearchInTarget(findtext);

    bool wrapped = false;

    if (respos < 0)
    {
	// wrap-around search
	wrapped = true;

	if (!backwards)
	{
	    editctrl->SetTargetStart(0);
	    editctrl->SetTargetEnd(quickfind_startpos);
	}
	else
	{
	    editctrl->SetTargetStart(editctrl->GetLength());
	    editctrl->SetTargetEnd(quickfind_startpos);
	}

	respos = editctrl->SearchInTarget(findtext);
    }

    bool found = false;
    if (respos >= 0)
    {
	found = true;
	int start = editctrl->GetTargetStart();
	int end = editctrl->GetTargetEnd();

	editctrl->EnsureVisible( editctrl->LineFromPosition(start) );
	editctrl->SetSelection(start, end);
    }

    if (found && !wrapped) {
	UpdateStatusBar( wxT("") );
    }
    else if (found && wrapped) {
	if (!backwards)
	    UpdateStatusBar( _("Search wrapped to beginning of document.") );
	else
	    UpdateStatusBar( _("Search wrapped to end of document.") );
    }
    else if (!found) {
	UpdateStatusBar( _("Search string not found in document.") );
    }

    wxColor clr;
    if (found) clr = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    else clr = wxColor(255, 102, 102);

    wmain->quickfindbar->textctrlQuickFind->SetBackgroundColour(clr);
    wmain->quickfindbar->textctrlQuickFind->Refresh();
}

bool WTextPage::DoQuickGoto(const wxString& gototext)
{
    long linenum;

    if (! gototext.ToLong(&linenum) || linenum <= 0 ) {
	UpdateStatusBar(_("Yeah right. Enter a number smarty."));
	return false;
    }

    editctrl->GotoLine(linenum-1);
    UpdateStatusBar(wxString::Format(_("Jumped to line %d."), editctrl->GetCurrentLine()+1));

    return true;
}

// *** Scintilla Callbacks ***

void WTextPage::OnScintillaUpdateUI(wxStyledTextEvent& WXUNUSED(event))
{
    if (!editctrl->GetUndoCollection()) return;

    wxMenuBar* menubar = wmain->menubar;
    wxToolBar* toolbar = wmain->toolbar;

    // Enable or Disable Menu Items and Tool Bar Items

    menubar->Enable(wxID_UNDO, editctrl->CanUndo());
    menubar->Enable(wxID_REDO, editctrl->CanRedo());

    menubar->Enable(wxID_PASTE, editctrl->CanPaste());

    toolbar->EnableTool(wxID_UNDO, editctrl->CanUndo());
    toolbar->EnableTool(wxID_REDO, editctrl->CanRedo());

    toolbar->EnableTool(wxID_PASTE, editctrl->CanPaste());

    bool HasSelection = editctrl->GetSelectionEnd() > editctrl->GetSelectionStart();

    menubar->Enable(wxID_CUT, HasSelection);
    menubar->Enable(wxID_COPY, HasSelection);
    menubar->Enable(wxID_CLEAR, HasSelection);

    toolbar->EnableTool(wxID_CUT, HasSelection);
    toolbar->EnableTool(wxID_COPY, HasSelection);
    toolbar->EnableTool(wxID_CLEAR, HasSelection);

    // Update status bar field
    {
	int pos = editctrl->GetCurrentPos();
	int row = editctrl->LineFromPosition(pos);
	int col = editctrl->GetColumn(pos);
	int sel = editctrl->GetSelectionEnd () - editctrl->GetSelectionStart();

	wxString sb;
	sb.Printf( _("Ln %d Col %d Sel %d"), row, col, sel);

	wmain->statusbar->SetStatusText(sb, 1);
    }
}

void WTextPage::OnScintillaSavePointReached(wxStyledTextEvent& WXUNUSED(event))
{
    // Document is un-modified (via Undo or Save)
    
    SetModified(false);
}

void WTextPage::OnScintillaSavePointLeft(wxStyledTextEvent& WXUNUSED(event))
{
    // Document is modified

    SetModified(true);
}

// *** Set/Get View Options ***

void WTextPage::SetViewLineWrap(bool on)
{
    view_linewrap = on;
    editctrl->SetWrapMode(view_linewrap ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
}

bool WTextPage::GetViewLineWrap()
{
    return view_linewrap;
}

void WTextPage::SetViewLineNumber(bool on)
{
    view_linenumber = on;

    if (!view_linenumber) {
        editctrl->SetMarginWidth(MARGIN_LINENUMBER, 0);
    }
    else {
	int marginwidth = editctrl->TextWidth(wxSTC_STYLE_LINENUMBER, _T("_99999"));
	editctrl->SetMarginWidth(MARGIN_LINENUMBER, marginwidth);
    }
}
bool WTextPage::GetViewLineNumber()
{
    return view_linenumber;
}

void WTextPage::SetViewWhitespace(bool on)
{
    view_whitespace = on;
    editctrl->SetViewWhiteSpace(view_whitespace ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
}
bool WTextPage::GetViewWhitespace()
{
    return view_whitespace;
}

void WTextPage::SetViewEndOfLine(bool on)
{
    view_endofline = on;
    editctrl->SetViewEOL(view_endofline);
}
bool WTextPage::GetViewEndOfLine()
{
    return view_endofline;
}

void WTextPage::SetViewIndentGuide(bool on)
{
    view_indentguide = on;
    editctrl->SetIndentationGuides(view_indentguide);
}
bool WTextPage::GetViewIndentGuide()
{
    return view_indentguide;
}
    
void WTextPage::SetViewLonglineGuide(bool on)
{
    view_longlineguide = on;
    editctrl->SetEdgeColumn(80);
    editctrl->SetEdgeMode(view_longlineguide ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
}
bool WTextPage::GetViewLonglineGuide()
{
    return view_longlineguide;
}


BEGIN_EVENT_TABLE(WTextPage, wxPanel)

    // *** Edit Menu Event passed down by WCryptoTE

    EVT_MENU	(wxID_UNDO,		WTextPage::OnMenuEditUndo)
    EVT_MENU	(wxID_REDO,		WTextPage::OnMenuEditRedo)

    EVT_MENU	(wxID_CUT,		WTextPage::OnMenuEditCut)
    EVT_MENU	(wxID_COPY,		WTextPage::OnMenuEditCopy)
    EVT_MENU	(wxID_PASTE,		WTextPage::OnMenuEditPaste)
    EVT_MENU	(wxID_CLEAR,		WTextPage::OnMenuEditDelete)

    EVT_MENU	(wxID_SELECTALL,	WTextPage::OnMenuEditSelectAll)
    EVT_MENU	(WCryptoTE::myID_MENU_EDIT_SELECTLINE, WTextPage::OnMenuEditSelectLine)

    // *** Scintilla Edit Callbacks

    EVT_STC_UPDATEUI(myID_EDITCTRL,		WTextPage::OnScintillaUpdateUI)
    EVT_STC_SAVEPOINTREACHED(myID_EDITCTRL,	WTextPage::OnScintillaSavePointReached)
    EVT_STC_SAVEPOINTLEFT(myID_EDITCTRL,	WTextPage::OnScintillaSavePointLeft)

END_EVENT_TABLE()

#if 0
/*****************************************************************************/

bool CEWEditCtrl::FileOpen(const wxString& filename)
{
    wxFileInputStream stream(filename);

    if (!stream.IsOk()) return false;

    if (!LoadInputStream(stream)) return false;

    currentfilename.Assign(filename);
    if (!currentfilename.IsAbsolute())
	currentfilename.MakeAbsolute();

    UpdateStatusBar(
	wxString::Format(_("Loaded %1$lu bytes from %2$S"),
			 GetLength(),
			 currentfilename.GetFullPath().c_str()));

    return true;
}

bool CEWEditCtrl::FileSaveAs(const wxString& filename)
{
    wxFile file(filename, wxFile::write);
    if (!file.IsOpened()) return false;

    size_t buflen = GetTextLength();
    wxCharBuffer buf = GetTextRaw();

    if (file.Write(buf, buflen))
    {
        file.Close();

	currentfilename.Assign(filename);
	if (!currentfilename.IsAbsolute())
	    currentfilename.MakeAbsolute();

        SetSavePoint();
	UpdateStatusBar(
	    wxString::Format(_("Wrote %1$lu bytes to %2$S"),
			     buflen,
			     currentfilename.GetFullPath().c_str()));
	return true;
    }

    return false;
}

bool CEWEditCtrl::LoadInputStream(wxInputStream& stream)
{
    const wxFileOffset stream_len = stream.GetLength();

    ClearAll();

    const size_t buf_len = wxMin(1024*1024, (size_t)stream_len);
    wxCharBuffer charBuf(buf_len + 2);

    for (int i = 0; !stream.Eof(); i++)
    {
	stream.Read(charBuf.data(), buf_len);

	const size_t last_read = stream.LastRead();
	if (last_read == 0) break;

	AddTextRaw(charBuf.data(), last_read);

	// at end or it was read all at once
	if (last_read == (size_t)stream_len) break;
    }

    EmptyUndoBuffer();
    SetSavePoint();
    GotoPos(0);
    ScrollToColumn(0); // extra help to ensure scrolled to 0 otherwise scrolled
                       // halfway thru 1st char

    return true;
}

/*****************************************************************************/
#endif

// *** WQuickFindBar ***

WQuickFindBar::WQuickFindBar(class WCryptoTE* parent)
    : wxPanel(parent)
{
    #include "art/window_close.h"
    #include "art/go_up.h"
    #include "art/go_down.h"

    wxBitmapButton* buttonQuickFindClose
	= new wxBitmapButton(this, WCryptoTE::myID_QUICKFIND_CLOSE, wxBitmapFromMemory(window_close_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindClose->SetLabel(_("Close"));
    buttonQuickFindClose->SetToolTip(_("Close Quick-Find bar"));

    wxStaticText* labelQuickFind = new wxStaticText(this, wxID_ANY, _("Find: "));

    textctrlQuickFind = new wxTextCtrl(this, WCryptoTE::myID_QUICKFIND_TEXT, wxEmptyString);

    wxBitmapButton* buttonQuickFindNext
	= new wxBitmapButton(this, WCryptoTE::myID_QUICKFIND_NEXT, wxBitmapFromMemory(go_down_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindNext->SetLabel(_("Next"));
    buttonQuickFindNext->SetToolTip(_("Search for next occurance"));

    wxBitmapButton* buttonQuickFindPrev
	= new wxBitmapButton(this, WCryptoTE::myID_QUICKFIND_PREV, wxBitmapFromMemory(go_up_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindPrev->SetLabel(_("Previous"));
    buttonQuickFindPrev->SetToolTip(_("Search for previous occurance"));

    // Set up sizers

    sizerQuickFind = new wxBoxSizer(wxHORIZONTAL);
    sizerQuickFind->Add(buttonQuickFindClose, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickFind->Add(labelQuickFind, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickFind->Add(textctrlQuickFind, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickFind->Add(buttonQuickFindNext, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickFind->Add(buttonQuickFindPrev, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    wxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(sizerQuickFind, 0, wxLEFT | wxRIGHT | wxEXPAND, 2);

    SetSizer(sizerMain);
    Layout();
    sizerMain->Fit(this);
}

// *** WQuickGotoBar ***

WQuickGotoBar::WQuickGotoBar(class WCryptoTE* parent)
    : wxPanel(parent)
{
    #include "art/window_close.h"
    #include "art/go_next.h"

    wxBitmapButton* buttonGotoCancel
	= new wxBitmapButton(this, WCryptoTE::myID_QUICKGOTO_CLOSE, wxBitmapFromMemory(window_close_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonGotoCancel->SetLabel(_("Cancel"));
    buttonGotoCancel->SetToolTip(_("Cancel Go to Line"));

    wxStaticText* labelGoto = new wxStaticText(this, wxID_ANY, _("Goto: "));

    textctrlGoto = new wxTextCtrl(this, WCryptoTE::myID_QUICKGOTO_TEXT, wxEmptyString,
				   wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    wxBitmapButton* buttonGotoGo
	= new wxBitmapButton(this, WCryptoTE::myID_QUICKGOTO_GO, wxBitmapFromMemory(go_next_png),
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonGotoGo->SetLabel(_("Go"));
    buttonGotoGo->SetToolTip(_("Go to Line"));

    // Set up sizers

    sizerQuickGoto = new wxBoxSizer(wxHORIZONTAL);
    sizerQuickGoto->Add(buttonGotoCancel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickGoto->Add(labelGoto, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickGoto->Add(textctrlGoto, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    sizerQuickGoto->Add(buttonGotoGo, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    wxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(sizerQuickGoto, 0, wxLEFT | wxRIGHT | wxEXPAND, 2);

    SetSizer(sizerMain);
    Layout();
    sizerMain->Fit(this);
}
