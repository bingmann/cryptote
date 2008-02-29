// $Id$

#include "wtextpage.h"
#include "common/tools.h"

#include <wx/file.h>
#include <wx/wfstream.h>
#include <stc.h>

WTextPage::WTextPage(class WCryptoTE* parent)
    : WNotePage(parent)
{
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
}

wxString WTextPage::GetCaption()
{
    return _T("Test Caption");
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

    if (! gototext.ToLong(&linenum) ) {
	UpdateStatusBar(_("Yeah right. Enter a number smarty."));
	return false;
    }

    editctrl->GotoLine(linenum);
    UpdateStatusBar(wxString::Format(_("Jumped to line %d."), editctrl->GetCurrentLine()));

    return true;
}

BEGIN_EVENT_TABLE(WTextPage, wxPanel)

    // Edit Menu
    EVT_MENU	(wxID_UNDO,		WTextPage::OnMenuEditUndo)
    EVT_MENU	(wxID_REDO,		WTextPage::OnMenuEditRedo)

    EVT_MENU	(wxID_CUT,		WTextPage::OnMenuEditCut)
    EVT_MENU	(wxID_COPY,		WTextPage::OnMenuEditCopy)
    EVT_MENU	(wxID_PASTE,		WTextPage::OnMenuEditPaste)
    EVT_MENU	(wxID_CLEAR,		WTextPage::OnMenuEditDelete)

    EVT_MENU	(wxID_SELECTALL,	WTextPage::OnMenuEditSelectAll)
    EVT_MENU	(WCryptoTE::myID_MENU_EDIT_SELECTLINE, WTextPage::OnMenuEditSelectLine)

END_EVENT_TABLE()

#if 0
/*****************************************************************************/

void CEWEditCtrl::FileNew()
{
    ClearAll();
    EmptyUndoBuffer();
    SetSavePoint();

    currentfilename.Clear();
}

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

bool CEWEditCtrl::FileSave()
{
    if (!HasFilename()) {
	wxLogError(_("Current buffer has no file name assigned."));
	return false;
    }

    return FileSaveAs( currentfilename.GetFullPath() );
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

bool CEWEditCtrl::FileRevert()
{
    if (!HasFilename()) {
	wxLogError(_("Current buffer has no file name assigned."));
	return false;
    }

    return FileOpen( currentfilename.GetFullPath() );
}

bool CEWEditCtrl::HasFilename() const
{
    return currentfilename.IsOk();
}

wxString CEWEditCtrl::GetFileFullpath() const
{
    return currentfilename.GetFullPath();
}

wxString CEWEditCtrl::GetFileBasename() const
{
    if (!currentfilename.IsOk()) {
	return _("Untitled.txt");
    }
    return currentfilename.GetFullName();
}

bool& CEWEditCtrl::ModifiedFlag()
{
    return modified;
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

// *** Display Settings ***

void CEWEditCtrl::ShowLineNumber(bool on)
{
    if (!on) 
        SetMarginWidth(MARGIN_LINENUMBER, 0);
    else {
	int marginwidth = TextWidth(wxSTC_STYLE_LINENUMBER, _T("_99999"));
	SetMarginWidth(MARGIN_LINENUMBER, marginwidth);
    }
}

BEGIN_EVENT_TABLE(CEWEditCtrl, wxStyledTextCtrl)

END_EVENT_TABLE()

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
