// $Id$

/*
 * CryptoTE v0.0.0
 * Copyright (C) 2008-2009 Timo Bingmann
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "wtextpage.h"
#include "wfilelist.h"
#include "bmpcat.h"
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

    cursor_firstvisibleline = -1;
    cursor_xoffset = -1;
    cursor_currentpos = -1;

    quickfind_startpos = -1;
    quickfind_length = 0;

    // *** Create Control ***

    editctrl = new wxStyledTextCtrl(this, myID_EDITCTRL);
    editctrl->SetFocus();

    //editctrl->UsePopUp(false);	// we show a context menu ourselves.

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
    editctrl->StyleSetBackground(STYLE_FINDHIGHLIGHT, wxColour(255,255,0));

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
    return strSTL2WX( wmain->container.GetSubFileProperty(subfileid, "Name") );
}

/** Appends the incoming text file data into the Scintilla edit control. */
class DataOutputTextPage : public Enctain::DataOutput
{
public:
    class WTextPage&	tpage;

    /// Constructor is given all information
    DataOutputTextPage(WTextPage& _tpage)
	: tpage(_tpage)
    {
    }

    /// Virtual callback function to save data.
    virtual bool Output(const void* data, size_t datalen)
    {
	tpage.editctrl->AddTextRaw((const char*)data, datalen);
	return true;
    }
};

bool WTextPage::LoadSubFile(unsigned int sfid)
{
    editctrl->ClearAll();
    editctrl->Allocate(wmain->container.GetSubFileSize(sfid));
    editctrl->SetUndoCollection(false);

    try {
	DataOutputTextPage dataout(*this);
	wmain->container.GetSubFileData(sfid, dataout);
    }
    catch (Enctain::Exception& e)
    {
	editctrl->ClearAll();
	wxLogError(WCryptoTE::EnctainExceptionString(e));
	return false;
    }

    subfileid = sfid;

    editctrl->SetUndoCollection(true);
    editctrl->EmptyUndoBuffer();
    editctrl->SetSavePoint();
    editctrl->GotoPos(0);
    editctrl->ScrollToColumn(0); // extra help to ensure scrolled to 0
				 // otherwise scrolled halfway thru 1st char

    LoadSubFileMetaSettings();

    return true;
}

bool WTextPage::LoadSubFileMetaSettings()
{
    if (!wmain->copt_restoreview) return false;

    std::string ms_str = wmain->container.GetSubFileProperty(subfileid, "WTextPageSettings");
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

	SetZoom(ms.view_zoom);

	// These cannot be set immediatedly because Scintilla hasn't update
	// it's cache to wrap lines. They are set on the first PAINTED event.
	cursor_firstvisibleline = ms.cursor_firstvisibleline;
	cursor_xoffset = ms.cursor_xoffset;
	cursor_currentpos = ms.cursor_currentpos;
	
	PageFocused(); // update menubar view checkmarks

	return true;
    }
    else {
	wxLogError(_("Could not restore settings of the text editor, maybe you need to upgrade CryptoTE to a newer version?"));
	return false;
    }
}

void WTextPage::SaveSubFileMetaSettings()
{
    if (!wmain->copt_restoreview)
    {
	wmain->container.DeleteSubFileProperty(subfileid, "WTextPageSettings");
	return;
    }

    MetaSettingsv00000001 ms;

    ms.version = 0x00000001;

    ms.view_linewrap = view_linewrap;
    ms.view_linenumber = view_linenumber;
    ms.view_whitespace = view_whitespace;
    ms.view_endofline = view_endofline;
    ms.view_indentguide = view_indentguide;
    ms.view_longlineguide = view_longlineguide;

    ms.view_zoom = GetZoom();

    ms.cursor_firstvisibleline = editctrl->GetFirstVisibleLine();
    ms.cursor_xoffset = editctrl->GetXOffset();
    ms.cursor_currentpos = editctrl->GetCurrentPos();

    wmain->container.SetSubFileProperty(subfileid, "WTextPageSettings", std::string((char*)&ms, sizeof(ms)));
}

size_t WTextPage::ImportFile(wxFile& file)
{
    SetViewLineWrap(false); // line wrapping seems to take so much processing
			    // time. we'll just disable it for imported
			    // files. let the user enable it if he really wants
			    // it.

    wxFileOffset filesize = file.Length();

    wmain->statusbar->ProgressStart(wxString(_("Importing")).mb_str(), Enctain::PI_GENERIC,
				    0, filesize);

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
    SetModified(true);

    return editctrl->GetTextLength();
}

void WTextPage::ExportBuffer(wxOutputStream& outstream)
{
    size_t buflen = editctrl->GetTextLength();
    wxCharBuffer buf = editctrl->GetTextRaw();

    outstream.Write(buf.data(), buflen);
}

void WTextPage::AddText(const wxString& text)
{
    editctrl->AddText(text);
}

// *** Event Handlers ***

static inline wxMenuItem* appendMenuItem(class wxMenu* parentMenu, int id,
					 const wxString& text, const wxString& helpString)
{
    wxMenuItem* mi = new wxMenuItem(parentMenu, id, text, helpString);
    mi->SetBitmap( BitmapCatalog::GetMenuBitmap(id) );
    parentMenu->Append(mi);
    return mi;
}

void WTextPage::OnContextMenu(wxContextMenuEvent& WXUNUSED(event))
{
    // Create Popup-Menu

    wxMenu* menu = new wxMenu;

    appendMenuItem(menu, wxID_UNDO,
		   _("&Undo\tCtrl+Z"),
		   _("Undo the last change."));

    appendMenuItem(menu, wxID_REDO,
		   _("&Redo\tCtrl+Shift+Z"),
		   _("Redo the previously undone change."));

    menu->AppendSeparator();

    appendMenuItem(menu, wxID_CUT,
		   _("Cu&t\tCtrl+X"),
		   _("Cut selected text into clipboard."));

    appendMenuItem(menu, wxID_COPY,
		   _("&Copy\tCtrl+C"),
		   _("Copy selected text into clipboard."));

    appendMenuItem(menu, wxID_PASTE,
		   _("&Paste\tCtrl+V"),
		   _("Paste clipboard contents at the current text position."));

    appendMenuItem(menu, wxID_CLEAR,
		   _("&Delete\tDel"),
		   _("Delete selected text."));

    menu->AppendSeparator();

    appendMenuItem(menu, wxID_SELECTALL,
		   _("&Select all\tCtrl+A"),
		   _("Select all text in the current buffer."));

    appendMenuItem(menu, myID_MENU_EDIT_SELECTLINE,
		   _("Select &line\tCtrl+L"),
		   _("Select whole line at the current cursor position."));

    menu->AppendSeparator();

    appendMenuItem(menu, myID_MENU_EDIT_INSERT_PASSWORD,
		   _("Insert &Password ...\tCtrl+P"),
		   _("Open random generator dialog box and insert the generated password."));


    // Enable or Disable Menu Items and Tool Bar Items

    menu->Enable(wxID_UNDO, editctrl->CanUndo());
    menu->Enable(wxID_REDO, editctrl->CanRedo());

    bool HasSelection = editctrl->GetSelectionEnd() > editctrl->GetSelectionStart();

    menu->Enable(wxID_CUT, HasSelection);
    menu->Enable(wxID_COPY, HasSelection);
    menu->Enable(wxID_CLEAR, HasSelection);
    menu->Enable(wxID_PASTE, editctrl->CanPaste());

    PopupMenu(menu);
}

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
    if (editctrl->GetSelectionEnd() <= editctrl->GetSelectionStart())
    {
	// Forward the DELETE key press event to the editing component
	// by synthesizing a DELETE key. Simply Veto()ing does not
	// work here.
	wxKeyEvent key(wxEVT_KEY_DOWN);
	key.SetId(editctrl->GetId());
	key.m_altDown = key.m_shiftDown = key.m_controlDown = 0;
	key.m_keyCode = WXK_DELETE;
	key.SetEventObject(editctrl);
	editctrl->OnKeyDown(key);
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
    wxMenuBar* menubar = wmain->menubar_textpage;

    // Update Menu -> View with current options

    menubar->Check(myID_MENU_VIEW_LINEWRAP, view_linewrap);
    menubar->Check(myID_MENU_VIEW_LINENUMBER, view_linenumber);
    menubar->Check(myID_MENU_VIEW_WHITESPACE, view_whitespace);
    menubar->Check(myID_MENU_VIEW_ENDOFLINE, view_endofline);
    menubar->Check(myID_MENU_VIEW_INDENTGUIDE, view_indentguide);
    menubar->Check(myID_MENU_VIEW_LONGLINEGUIDE, view_longlineguide);

    // Synthesize UpdateUI event
    wxStyledTextEvent event;
    OnScintillaUpdateUI(event);
}

void WTextPage::PageBlurred()
{
    StopQuickFind();
}

void WTextPage::PageSaveData()
{
    // always save view data
    SaveSubFileMetaSettings();

    if (!page_modified) return;	// no changes to save

    size_t buflen = editctrl->GetTextLength();
    wxCharBuffer buf = editctrl->GetTextRaw();

    try {
	wmain->container.SetSubFileData(subfileid, buf.data(), buflen);
    }
    catch (Enctain::Exception& e)
    {
	wxLogFatalError(WCryptoTE::EnctainExceptionString(e));
	return;
    }

    wmain->container.SetSubFileProperty(subfileid, "MTime", strTimeStampNow());

    editctrl->SetSavePoint();

    size_t savelen = wmain->container.GetSubFileStorageSize(subfileid);

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

    wmain->filelistpane->UpdateItem(subfileid);
}

void WTextPage::PageClosed()
{
    PageSaveData();
}

void WTextPage::PrepareQuickFind(bool backwards, bool reset)
{
    // clear previous find styling
    if (quickfind_length != 0)
    {
        editctrl->StartStyling(quickfind_startpos, 0x1F);
        editctrl->SetStyling(quickfind_length, 0);
    }

    if (reset)
    {
	quickfind_startpos = editctrl->GetCurrentPos();
        quickfind_length = 0;
    }
    else
    {
	if (!backwards)
	{
	    quickfind_startpos = editctrl->GetSelectionEnd();
            quickfind_startpos += quickfind_length;
	}
	else
	{
	    quickfind_startpos = editctrl->GetSelectionStart();
            quickfind_startpos -= quickfind_length;
	}
    }
}

void WTextPage::DoQuickFind(bool backwards, const wxString& findtext)
{
    // clear previous find styling
    if (quickfind_length != 0)
    {
        editctrl->StartStyling(quickfind_startpos, 0x1F);
        editctrl->SetStyling(quickfind_length, 0);
    }

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
	quickfind_startpos = editctrl->GetTargetStart();
	quickfind_length = editctrl->GetTargetEnd() - quickfind_startpos;

	editctrl->EnsureVisible( editctrl->LineFromPosition(quickfind_startpos) );

        editctrl->StartStyling(quickfind_startpos, 0x1F);
        editctrl->SetStyling(quickfind_length, STYLE_FINDHIGHLIGHT);

        editctrl->SetSelection(quickfind_startpos, quickfind_startpos);
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

void WTextPage::StopQuickFind()
{
    PrepareQuickFind(false, true);
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

    wxMenuBar* menubar = wmain->menubar_textpage;
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
    menubar->Enable(wxID_CLEAR, true);

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
	sb.Printf( _("Ln %d Col %d Sel %d"), (row+1), col, sel);

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

void WTextPage::OnScintillaZoom(wxStyledTextEvent& WXUNUSED(event))
{
    UpdateStatusBar(wxString::Format(_("Zoom level set to %+d."), editctrl->GetZoom()));
}

void WTextPage::OnScintillaPainted(wxStyledTextEvent& WXUNUSED(event))
{
    if (cursor_firstvisibleline >= 0)
    {
	editctrl->LineScroll(0, cursor_firstvisibleline);
	cursor_firstvisibleline = -1;
    }
    if (cursor_xoffset >= 0)
    {
	editctrl->SetXOffset(cursor_xoffset);
	cursor_xoffset = -1;
    }
    if (cursor_currentpos >= 0)
    {
	editctrl->GotoPos(cursor_currentpos);
	cursor_currentpos = -1;
    }
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

void WTextPage::SetZoom(int level)
{
    editctrl->SetZoom(level);
}

int WTextPage::GetZoom()
{
    return editctrl->GetZoom();
}

BEGIN_EVENT_TABLE(WTextPage, WNotePage)

    EVT_CONTEXT_MENU(WTextPage::OnContextMenu)

    // *** Edit Menu Event passed down by WCryptoTE

    EVT_MENU	(wxID_UNDO,		WTextPage::OnMenuEditUndo)
    EVT_MENU	(wxID_REDO,		WTextPage::OnMenuEditRedo)

    EVT_MENU	(wxID_CUT,		WTextPage::OnMenuEditCut)
    EVT_MENU	(wxID_COPY,		WTextPage::OnMenuEditCopy)
    EVT_MENU	(wxID_PASTE,		WTextPage::OnMenuEditPaste)
    EVT_MENU	(wxID_CLEAR,		WTextPage::OnMenuEditDelete)

    EVT_MENU	(wxID_SELECTALL,	WTextPage::OnMenuEditSelectAll)
    EVT_MENU	(myID_MENU_EDIT_SELECTLINE, WTextPage::OnMenuEditSelectLine)

    // *** Scintilla Edit Callbacks

    EVT_STC_UPDATEUI(myID_EDITCTRL,		WTextPage::OnScintillaUpdateUI)
    EVT_STC_SAVEPOINTREACHED(myID_EDITCTRL,	WTextPage::OnScintillaSavePointReached)
    EVT_STC_SAVEPOINTLEFT(myID_EDITCTRL,	WTextPage::OnScintillaSavePointLeft)
    EVT_STC_ZOOM(myID_EDITCTRL,			WTextPage::OnScintillaZoom)
    EVT_STC_PAINTED(myID_EDITCTRL,		WTextPage::OnScintillaPainted)

END_EVENT_TABLE()

IMPLEMENT_ABSTRACT_CLASS(WTextPage, WNotePage);

// *** WQuickFindBar ***

WQuickFindBar::WQuickFindBar(class WCryptoTE* parent)
    : wxPanel(parent)
{
    buttonQuickFindClose
	= new wxBitmapButton(this, myID_QUICKFIND_CLOSE, wxNullBitmap,
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindClose->SetLabel(_("Close"));
    buttonQuickFindClose->SetToolTip(_("Close Quick-Find bar"));

    wxStaticText* labelQuickFind = new wxStaticText(this, wxID_ANY, _("Find: "));

    textctrlQuickFind = new wxTextCtrl(this, myID_QUICKFIND_TEXT, wxEmptyString);

    buttonQuickFindNext
	= new wxBitmapButton(this, myID_QUICKFIND_NEXT, wxNullBitmap,
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonQuickFindNext->SetLabel(_("Next"));
    buttonQuickFindNext->SetToolTip(_("Search for next occurance"));

    buttonQuickFindPrev
	= new wxBitmapButton(this, myID_QUICKFIND_PREV, wxNullBitmap,
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

    set_bitmaps();
}

void WQuickFindBar::set_bitmaps()
{
    buttonQuickFindClose->SetBitmapLabel( BitmapCatalog::GetBitmap(myID_QUICKFIND_CLOSE) );

    buttonQuickFindNext->SetBitmapLabel( BitmapCatalog::GetBitmap(myID_QUICKFIND_NEXT) );
    buttonQuickFindPrev->SetBitmapLabel( BitmapCatalog::GetBitmap(myID_QUICKFIND_PREV) );

    Layout();
}

// *** WQuickGotoBar ***

WQuickGotoBar::WQuickGotoBar(class WCryptoTE* parent)
    : wxPanel(parent)
{
    buttonGotoCancel
	= new wxBitmapButton(this, myID_QUICKGOTO_CLOSE, wxNullBitmap,
			     wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    buttonGotoCancel->SetLabel(_("Cancel"));
    buttonGotoCancel->SetToolTip(_("Cancel Go to Line"));

    wxStaticText* labelGoto = new wxStaticText(this, wxID_ANY, _("Goto: "));

    textctrlGoto = new wxTextCtrl(this, myID_QUICKGOTO_TEXT, wxEmptyString,
				   wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    buttonGotoGo
	= new wxBitmapButton(this, myID_QUICKGOTO_GO, wxNullBitmap,
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

    set_bitmaps();
}

void WQuickGotoBar::set_bitmaps()
{
    buttonGotoCancel->SetBitmapLabel( BitmapCatalog::GetBitmap(myID_QUICKGOTO_CLOSE) );

    buttonGotoGo->SetBitmapLabel( BitmapCatalog::GetBitmap(myID_QUICKGOTO_GO) );

    Layout();
}
