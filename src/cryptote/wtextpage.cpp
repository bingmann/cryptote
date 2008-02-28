// $Id$

#include "wtextpage.h"

#include <wx/file.h>
#include <wx/wfstream.h>

WTextPage::WTextPage(class WCryptoTE* parent)
    : WNotePage(parent)
{

    wxTextCtrl* editctrl = new wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
 
    wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(editctrl, 1, wxEXPAND, 0);

    SetSizer(sizerMain);
    Layout();
}

wxString WTextPage::GetCaption()
{
    return _T("Test Caption");
}

BEGIN_EVENT_TABLE(WTextPage, wxPanel)

END_EVENT_TABLE()

#if 0
/*****************************************************************************/

{
    // set some styles

    wxFont font(10, wxMODERN, wxNORMAL, wxNORMAL);

    StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
    StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(_T("DARK GREY")));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(250,250,250));
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(_T("DARK GREY")));

    // Set Default View Options
    SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

    SetWrapMode(wxSTC_WRAP_WORD);
    SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);

    // Set up margin for line numbers
    SetMarginType(MARGIN_LINENUMBER, wxSTC_MARGIN_NUMBER);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(_T("DARK GREY")));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(250,250,250));
    SetMarginWidth(MARGIN_LINENUMBER, 0); // set width to 0
}

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

    wmain->UpdateStatusBar(
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
	wmain->UpdateStatusBar(
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

// *** Event Handlers ***

void CEWEditCtrl::OnMenuEditUndo(wxCommandEvent& WXUNUSED(event))
{
    if (!CanUndo()) {
	wmain->UpdateStatusBar(_("No more change operations to undo."));
	return;
    }
    Undo();
}

void CEWEditCtrl::OnMenuEditRedo(wxCommandEvent& WXUNUSED(event))
{
    if (!CanRedo()) {
	wmain->UpdateStatusBar(_("No more change operations to redo."));
	return;
    }
    Redo();
}

void CEWEditCtrl::OnMenuEditCut(wxCommandEvent& WXUNUSED(event))
{
    if (GetReadOnly()) {
	wmain->UpdateStatusBar(_("Buffer is read-only."));
	return;
    }
    if (GetSelectionEnd() <= GetSelectionStart()) {
	wmain->UpdateStatusBar(_("Nothing selected."));
	return;
    }

    int cutlen = GetSelectionEnd() - GetSelectionStart();
    Cut();

    wmain->UpdateStatusBar(
	wxString::Format(_("Cut %u characters into clipboard."), cutlen)
	);
}

void CEWEditCtrl::OnMenuEditCopy(wxCommandEvent& WXUNUSED(event))
{
    if (GetSelectionEnd() <= GetSelectionStart()) {
	wmain->UpdateStatusBar(_("Nothing selected."));
	return;
    }

    int copylen = GetSelectionEnd() - GetSelectionStart();
    
    Copy();

    wmain->UpdateStatusBar(
	wxString::Format(_("Copied %u characters into clipboard."), copylen)
	);
}

void CEWEditCtrl::OnMenuEditPaste(wxCommandEvent& WXUNUSED(event))
{
    if (!CanPaste()) {
	wmain->UpdateStatusBar(_("Nothing pasted, the clipboard is empty."));
	return;
    }

    int prevlen = GetTextLength();
    prevlen -= GetSelectionEnd() - GetSelectionStart();

    Paste();

    wmain->UpdateStatusBar(
	wxString::Format(_("Pasted %u characters from clipboard."),
			 GetTextLength() - prevlen)
	);
}

void CEWEditCtrl::OnMenuEditDelete(wxCommandEvent& WXUNUSED(event))
{
    if (GetReadOnly()) {
	wmain->UpdateStatusBar(_("Buffer is read-only."));
	return;
    }
    if (GetSelectionEnd() <= GetSelectionStart()) {
	wmain->UpdateStatusBar(_("Nothing selected."));
	return;
    }

    int deletelen = GetSelectionEnd() - GetSelectionStart();

    Clear();

    wmain->UpdateStatusBar(
	wxString::Format(_("Deleted %u characters from buffer."), deletelen)
	);
}

void CEWEditCtrl::OnMenuEditSelectAll(wxCommandEvent& WXUNUSED(event))
{
    SetSelection(0, GetTextLength());
}

void CEWEditCtrl::OnMenuEditSelectLine(wxCommandEvent& WXUNUSED(event))
{
    int lineStart = PositionFromLine(GetCurrentLine());
    int lineEnd = PositionFromLine(GetCurrentLine() + 1);

    SetSelection(lineStart, lineEnd);
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

    // Edit Menu
    EVT_MENU	(wxID_UNDO,		CEWEditCtrl::OnMenuEditUndo)
    EVT_MENU	(wxID_REDO,		CEWEditCtrl::OnMenuEditRedo)

    EVT_MENU	(wxID_CUT,		CEWEditCtrl::OnMenuEditCut)
    EVT_MENU	(wxID_COPY,		CEWEditCtrl::OnMenuEditCopy)
    EVT_MENU	(wxID_PASTE,		CEWEditCtrl::OnMenuEditPaste)
    EVT_MENU	(wxID_CLEAR,		CEWEditCtrl::OnMenuEditDelete)

    EVT_MENU	(wxID_SELECTALL,	CEWEditCtrl::OnMenuEditSelectAll)
    EVT_MENU	(CEWMain::myID_MENU_SELECTLINE, CEWEditCtrl::OnMenuEditSelectLine)

END_EVENT_TABLE()

/*****************************************************************************/
#endif
