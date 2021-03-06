/*******************************************************************************
 * src/cryptote/wfileprop.cpp
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

#include "wcryptote.h"
#include "wfileprop.h"

#include "common/tools.h"

// begin wxGlade: ::extracode
// end wxGlade

WFileProperties::WFileProperties(WCryptoTE* parent, int _subfileid, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long WXUNUSED(style))
    : wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxTHICK_FRAME),
      wmain(parent), subfileid(_subfileid)
{
    SetMinSize(wxSize(350, 0));

    // begin wxGlade: WFileProperties::WFileProperties
    textIdentifier = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    textFilename = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    textAuthor = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    textSize = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    textCompressed = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    textCTime = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    textMTime = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    const wxString choiceType_choices[] = {
        _("Binary Data"),
        _("Text File"),
        _("Image File")
    };
    choiceType = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, choiceType_choices, 0);
    const wxString choiceCompression_choices[] = {
        _("None"),
        _("ZLib"),
        _("BZ2")
    };
    choiceCompression = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, choiceCompression_choices, 0);
    const wxString choiceEncryption_choices[] = {
        _("None (Don't Use)"),
        _("Serpent 256 keybits")
    };
    choiceEncryption = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, choiceEncryption_choices, 0);
    textSubject = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    textDescription = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    buttonOK = new wxButton(this, wxID_OK, wxEmptyString);
    buttonCancel = new wxButton(this, wxID_CANCEL, wxEmptyString);

    set_properties();
    do_layout();
    // end wxGlade

    Enctain::Container cnt = wmain->container;

    if (subfileid < 0) return;

    textIdentifier->SetValue(wxString::Format(_T("%u"), subfileid));
    SetTitle(wxString::Format(_("Properties of SubFile %u"), subfileid));

    textFilename->SetValue(strSTL2WX(cnt.GetSubFileProperty(subfileid, "Name")));
    textAuthor->SetValue(strSTL2WX(cnt.GetSubFileProperty(subfileid, "Author")));

    textSize->SetValue(wxString::Format(_T("%u"), cnt.GetSubFileSize(subfileid)));
    textCompressed->SetValue(wxString::Format(_T("%u"), cnt.GetSubFileStorageSize(subfileid)));

    std::string timestr = cnt.GetSubFileProperty(subfileid, "CTime");
    if (timestr.size() == sizeof(time_t)) {
        wxDateTime ctime(*(time_t*)timestr.data());
        textCTime->SetValue(ctime.Format(_("%c")));
    }

    timestr = cnt.GetSubFileProperty(subfileid, "MTime");
    if (timestr.size() == sizeof(time_t)) {
        wxDateTime mtime(*(time_t*)timestr.data());
        textMTime->SetValue(mtime.Format(_("%c")));
    }

    const std::string& filetype = cnt.GetSubFileProperty(subfileid, "Filetype");
    if (filetype == "text") {
        choiceType->SetSelection(1);
    }
    else {
        choiceType->SetSelection(0);
    }

    choiceCompression->SetSelection(cnt.GetSubFileCompression(subfileid));
    choiceEncryption->SetSelection(cnt.GetSubFileEncryption(subfileid));

    textSubject->SetValue(strSTL2WX(cnt.GetSubFileProperty(subfileid, "Subject")));
    textDescription->SetValue(strSTL2WX(cnt.GetSubFileProperty(subfileid, "Description")));

    textFilename->SetFocus();
}

void WFileProperties::set_properties()
{
    // begin wxGlade: WFileProperties::set_properties
    SetTitle(_("Properties of"));
    textIdentifier->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    textSize->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    textCompressed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    textCTime->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    textMTime->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    choiceType->SetSelection(0);
    choiceCompression->SetSelection(0);
    choiceEncryption->SetSelection(0);
    // end wxGlade
}

void WFileProperties::do_layout()
{
    // begin wxGlade: WFileProperties::do_layout
    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    wxGridSizer* sizer3 = new wxGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* sizer2 = new wxFlexGridSizer(12, 2, 0, 0);
    wxStaticText* label1 = new wxStaticText(this, wxID_ANY, _("Identifier:"));
    sizer2->Add(label1, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textIdentifier, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label2 = new wxStaticText(this, wxID_ANY, _("Filename:"));
    sizer2->Add(label2, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textFilename, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label3 = new wxStaticText(this, wxID_ANY, _("Author:"));
    sizer2->Add(label3, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textAuthor, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label4 = new wxStaticText(this, wxID_ANY, _("Size:"));
    sizer2->Add(label4, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textSize, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label5 = new wxStaticText(this, wxID_ANY, _("Compressed:"));
    sizer2->Add(label5, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textCompressed, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label6 = new wxStaticText(this, wxID_ANY, _("Created:"));
    sizer2->Add(label6, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textCTime, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label7 = new wxStaticText(this, wxID_ANY, _("Last Modified:"));
    sizer2->Add(label7, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textMTime, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label8 = new wxStaticText(this, wxID_ANY, _("Type:"));
    sizer2->Add(label8, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(choiceType, 0, wxALL, 2);
    wxStaticText* label9 = new wxStaticText(this, wxID_ANY, _("Compression:"));
    sizer2->Add(label9, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(choiceCompression, 0, wxALL, 2);
    wxStaticText* label10 = new wxStaticText(this, wxID_ANY, _("Encryption:"));
    sizer2->Add(label10, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(choiceEncryption, 0, wxALL, 2);
    wxStaticText* label11 = new wxStaticText(this, wxID_ANY, _("Subject:"));
    sizer2->Add(label11, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textSubject, 0, wxALL | wxEXPAND, 2);
    wxStaticText* label12 = new wxStaticText(this, wxID_ANY, _("Description:"));
    sizer2->Add(label12, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textDescription, 0, wxALL | wxEXPAND, 2);
    sizer2->AddGrowableRow(11);
    sizer2->AddGrowableCol(1);
    sizer1->Add(sizer2, 1, wxALL | wxEXPAND, 8);
    wxStaticLine* staticline1 = new wxStaticLine(this, wxID_ANY);
    sizer1->Add(staticline1, 0, wxEXPAND, 0);
    sizer3->Add(buttonOK, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 4);
    sizer3->Add(buttonCancel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    sizer1->Add(sizer3, 0, wxEXPAND, 0);
    SetSizer(sizer1);
    sizer1->Fit(this);
    Layout();
    Centre();
    // end wxGlade
}

BEGIN_EVENT_TABLE(WFileProperties, wxDialog)
    // begin wxGlade: WFileProperties::event_table
    EVT_BUTTON(wxID_OK, WFileProperties::OnButtonOK)
    // end wxGlade
END_EVENT_TABLE();

// wxGlade: add WFileProperties event handlers

void WFileProperties::OnButtonOK(wxCommandEvent& WXUNUSED(event))
{
    Enctain::Container cnt = wmain->container;

    cnt.SetSubFileProperty(subfileid, "Name", strWX2STL(textFilename->GetValue()));
    cnt.SetSubFileProperty(subfileid, "Author", strWX2STL(textAuthor->GetValue()));

    int ifiletype = choiceType->GetSelection();
    if (ifiletype == 1) {
        cnt.SetSubFileProperty(subfileid, "Filetype", "text");
    }
    else {
        cnt.SetSubFileProperty(subfileid, "Filetype", "");
    }

    Enctain::compression_type newcomp = (Enctain::compression_type)choiceCompression->GetSelection();
    Enctain::encryption_type newencr = (Enctain::encryption_type)choiceEncryption->GetSelection();

    if (newcomp != cnt.GetSubFileCompression(subfileid) ||
        newencr != cnt.GetSubFileEncryption(subfileid))
    {
        size_t oldsize = cnt.GetSubFileStorageSize(subfileid);
        size_t realsize = cnt.GetSubFileSize(subfileid);

        cnt.SetSubFileCompressionEncryption(subfileid, newcomp, newencr);

        size_t newsize = cnt.GetSubFileStorageSize(subfileid);

        wmain->UpdateStatusBar(
            wxString::Format(_("Recompressed and reencrypted %u subfile bytes into %u bytes (old size %u)."),
                             realsize, newsize, oldsize)
            );
    }

    cnt.SetSubFileProperty(subfileid, "Subject", strWX2STL(textSubject->GetValue()));
    cnt.SetSubFileProperty(subfileid, "Description", strWX2STL(textDescription->GetValue()));

    EndModal(wxID_OK);
}

/******************************************************************************/
