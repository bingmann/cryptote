/*******************************************************************************
 * src/cryptote/wbinpage.cpp
 *
 * Part of CryptoTE, see http://panthema.net/2007/cryptote
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

#include "common/tools.h"
#include "wbinpage.h"
#include "wfilelist.h"

#include <wx/file.h>
#include <wx/wfstream.h>

WBinaryPage::WBinaryPage(class WCryptoTE* parent)
    : WNotePage(parent),
      needsave(false)
{
    // *** Create Control ***

    listctrl = new WBinaryPageList(this, myID_LISTCTRL);
    listctrl->SetFocus();

    // *** Set up Sizer ***

    wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(listctrl, 1, wxEXPAND, 0);

    SetSizer(sizerMain);
    Layout();
}

wxString WBinaryPage::GetCaption()
{
    return strSTL2WX(wmain->container.GetSubFileProperty(subfileid, "Name"));
}

struct DataOutputMemoryBuffer : public Enctain::DataOutput
{
    wxMemoryBuffer& buffer;

    DataOutputMemoryBuffer(wxMemoryBuffer& out)
        : buffer(out)
    { }

    ~DataOutputMemoryBuffer()
    { }

    virtual bool Output(const void* data, size_t datalen)
    {
        buffer.AppendData(data, datalen);
        return true;
    }
};

bool WBinaryPage::LoadSubFile(unsigned int sfid)
{
    DataOutputMemoryBuffer dataout(bindata);

    bindata.SetBufSize(wmain->container.GetSubFileSize(sfid));
    bindata.SetDataLen(0);

    try {
        wmain->container.GetSubFileData(sfid, dataout);
    }
    catch (Enctain::Exception& e)
    {
        wxLogError(WCryptoTE::EnctainExceptionString(e));
        return false;
    }

    subfileid = sfid;
    needsave = false;

    listctrl->UpdateData();

    return true;
}

size_t WBinaryPage::ImportFile(wxFile& file)
{
    wxFileOffset filesize = file.Length();

    wmain->statusbar->ProgressStart(wxString(_("Importing")).mb_str(), Enctain::PI_GENERIC,
                                    0, filesize);

    bindata.SetBufSize(filesize);

    char buffer[65536];

    for (int i = 0; !file.Eof(); i++)
    {
        size_t rb = file.Read(buffer, sizeof(buffer));
        if (rb == 0) break;

        bindata.AppendData(buffer, rb);

        wmain->statusbar->ProgressUpdate(bindata.GetDataLen());
    }

    listctrl->UpdateData();
    needsave = true;

    wmain->statusbar->ProgressStop();

    return bindata.GetDataLen();
}

void WBinaryPage::ExportBuffer(wxOutputStream& outstream)
{
    outstream.Write(bindata.GetData(), bindata.GetDataLen());
}

// *** Virtual Callbacks via WNotePage ***

void WBinaryPage::PageFocused()
{
    // clear status bar field 1
    wmain->statusbar->SetStatusText(_T(""), 1);
}

void WBinaryPage::PageBlurred()
{ }

void WBinaryPage::PageSaveData()
{
    if (needsave)
    {
        try {
            wmain->container.SetSubFileData(subfileid, bindata.GetData(), bindata.GetDataLen());
        }
        catch (Enctain::Exception& e)
        {
            wxLogFatalError(WCryptoTE::EnctainExceptionString(e));
            return;
        }

        wmain->container.SetSubFileProperty(subfileid, "MTime", strTimeStampNow());

        size_t savelen = wmain->container.GetSubFileStorageSize(subfileid);

        if (savelen != bindata.GetDataLen())
        {
            UpdateStatusBar(
                wxString::Format(_("Compressed %u bytes from buffer into %u bytes for encrypted storage."),
                                 bindata.GetDataLen(), savelen)
                );
        }
        else
        {
            UpdateStatusBar(
                wxString::Format(_("Saving %u bytes from buffer for encrypted storage."),
                                 bindata.GetDataLen())
                );
        }

        wmain->SetModified();
        wmain->filelistpane->UpdateItem(subfileid);

        needsave = false;
    }
}

void WBinaryPage::PageClosed()
{
    PageSaveData();
}

void WBinaryPage::StopQuickFind()
{ }

bool WBinaryPage::DoQuickGoto(const wxString& gototext)
{
    long offset;

    if (! gototext.ToLong(&offset) || offset <= 0) {
        UpdateStatusBar(_("Yeah right. Enter a number smarty."));
        return false;
    }

    if (offset >= (long)bindata.GetDataLen()) {
        UpdateStatusBar(_("Yeah right. Offset is beyond the end of the data."));
        return false;
    }

    listctrl->EnsureVisible(offset / 16);
    listctrl->SetItemState(offset / 16,
                           wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                           wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    UpdateStatusBar(wxString::Format(_("Jumped to offset %d."), offset));

    return true;
}

void WBinaryPage::OnListItemFocused(wxListEvent& WXUNUSED(event))
{
    // clear status bar field 1
    wmain->statusbar->SetStatusText(_T(""), 1);
}

BEGIN_EVENT_TABLE(WBinaryPage, WNotePage)

    EVT_LIST_ITEM_FOCUSED(myID_LISTCTRL, WBinaryPage::OnListItemFocused)

END_EVENT_TABLE();

IMPLEMENT_ABSTRACT_CLASS(WBinaryPage, WNotePage);

// *** WBinaryPageList ***

WBinaryPageList::WBinaryPageList(class WBinaryPage* parent, wxWindowID id)
    : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL),
      binpage(*parent)
{
    wxFont font(10, wxMODERN, wxNORMAL, wxNORMAL);
    SetFont(font);

    unsigned int size = binpage.bindata.GetDataLen();

    SetItemCount((size / 16) + ((size % 16) != 0));

    InsertColumn(0, _T("Offset"), wxLIST_FORMAT_RIGHT, 32);
    InsertColumn(1, _T("Hexadecimal"), wxLIST_FORMAT_LEFT, 400);
    InsertColumn(2, _T("ASCII"), wxLIST_FORMAT_LEFT);

    wxWindowDC dc(this);
    wxCoord textwidth, textheight;

    dc.SetFont(font);

    dc.GetTextExtent(_T("000000000"), &textwidth, &textheight);
    SetColumnWidth(0, textwidth);

    dc.GetTextExtent(_T("00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  "), &textwidth, &textheight);
    SetColumnWidth(1, textwidth);

    dc.GetTextExtent(_T("0000000000000000  "), &textwidth, &textheight);
    SetColumnWidth(2, textwidth);

    SetItemCount((size / 16) + ((size % 16) != 0));
}

wxString WBinaryPageList::OnGetItemText(long item, long column) const
{
    const unsigned int offset = item * 16;
    const size_t binsize = binpage.bindata.GetDataLen();
    const char* dataoff = (const char*)binpage.bindata.GetData() + offset;

    if (offset > binsize) return _("Invalid");

    if (column == 0)
    {
        return wxString::Format(_T("%d"), offset);
    }
    else if (column == 1)
    {
        wxString hexdump;

        for (unsigned int hi = 0; hi < 16 && offset + hi < binsize; ++hi)
        {
            if (hexdump.size()) hexdump += _T(" ");
            if (hi == 8) hexdump += _T(" ");

            hexdump += wxString::Format(_T("%02X"), (unsigned char)(dataoff[hi]));
        }

        return hexdump;
    }
    else if (column == 2)
    {
        char buffer[16];
        unsigned int len = std::min<unsigned int>(16, binsize - offset);

        static const unsigned char asciiok[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x50 - 0x5F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 0x70 - 0x7F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80 - 0x8F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x90 - 0x9F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xA0 - 0xAF
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xB0 - 0xBF
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xC0 - 0xCF
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xD0 - 0xDF
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xE0 - 0xEF
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  // 0xF0 - 0xFF
        };

        for (unsigned int hi = 0; hi < 16 && offset + hi < binsize; ++hi)
        {
            char c = dataoff[hi];

            if (asciiok[(unsigned char)c])
                buffer[hi] = c;
            else
                buffer[hi] = '.';
        }

        return wxString(buffer, wxConvISO8859_1, len);
    }
    return _T("Error");
}

void WBinaryPageList::UpdateData()
{
    unsigned int size = binpage.bindata.GetDataLen();

    SetItemCount((size / 16) + ((size % 16) != 0));
}

/******************************************************************************/
