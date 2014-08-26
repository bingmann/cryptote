/*******************************************************************************
 * src/cryptote/wmsgdlg.cpp
 *
 * Based on wxWidgets-2.8.7/src/generic/msgdlgg.cpp
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

#include "wmsgdlg.h"
#include "bmpcat.h"

#include "common/tools.h"

WMessageDialog::WMessageDialog(wxWindow* parent, const wxString& message,
                               const wxString& caption, long style,
                               int _button0, int _button1, int _button2)
    : wxDialog(parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      button0(_button0),
      button1(_button1),
      button2(_button2)
{
    wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* sizerIconText = new wxBoxSizer(wxHORIZONTAL);

    // 1) icon
    if (style & wxICON_MASK)
    {
        wxBitmap bitmap;
        switch (style & wxICON_MASK)
        {
        default:
            wxFAIL_MSG(_T("Incorrect log style"));
        // fall through

        case wxICON_ERROR:
            bitmap = BitmapCatalog::GetBitmap(wxICON_ERROR);
            break;

        case wxICON_WARNING:
            bitmap = BitmapCatalog::GetBitmap(wxICON_WARNING);
            break;

        case wxICON_INFORMATION:
            bitmap = BitmapCatalog::GetBitmap(wxICON_INFORMATION);
            break;
        }

        wxStaticBitmap* icon = new wxStaticBitmap(this, wxID_ANY, bitmap);

        sizerIconText->Add(icon, 0, wxCENTER);
    }

    // 2) text
    sizerIconText->Add(CreateTextSizer(message), 0, wxALIGN_CENTER | wxLEFT, 16);

    sizerTop->Add(sizerIconText, 1, wxCENTER | wxLEFT | wxRIGHT | wxTOP, 10);

    // 3) buttons
    wxGridSizer* sizerButton = new wxGridSizer(0, 0, 0);

    if (button0 != 0)
        sizerButton->Add(CreateButton(button0), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 4);

    if (button1 != 0)
        sizerButton->Add(CreateButton(button1), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 4);

    if (button2 != 0)
        sizerButton->Add(CreateButton(button2), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 4);

    sizerTop->Add(sizerButton, 0, wxEXPAND | wxALL, 10);

    SetAutoLayout(true);
    SetSizer(sizerTop);

    sizerTop->SetSizeHints(this);
    sizerTop->Fit(this);

    wxSize size(GetSize());
    if (size.x < size.y * 3 / 2)
    {
        size.x = size.y * 3 / 2;
        SetSize(size);
    }

    Centre(wxBOTH | wxCENTER_FRAME);
}

wxButton* WMessageDialog::CreateButton(int id)
{
    switch (id)
    {
    case wxID_OK:
    case wxID_CANCEL:
    case wxID_APPLY:
    case wxID_YES:
    case wxID_NO:
    case wxID_ABORT:
    case wxID_SAVE:
    case wxID_EXIT:
        return new wxButton(this, id, wxEmptyString);

    case wxID_IGNORE:
        return new wxButton(this, id, _("&Ignore"));

    default:
        wxFAIL_MSG(_T("Unknown button id"));
        return new wxButton(this, id, wxEmptyString);
    }
}

void WMessageDialog::OnButton(wxCommandEvent& event)
{
    EndModal(event.GetId());
}

void WMessageDialog::OnButtonCancel(wxCommandEvent& WXUNUSED(event))
{
    // Allow cancellation via Close-X or ESC only if the button is defined
    if (button0 == wxID_CANCEL || button1 == wxID_CANCEL || button2 == wxID_CANCEL)
    {
        EndModal(wxID_CANCEL);
    }
}

BEGIN_EVENT_TABLE(WMessageDialog, wxDialog)

    EVT_BUTTON(wxID_OK, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_CANCEL, WMessageDialog::OnButtonCancel)
    EVT_BUTTON(wxID_APPLY, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_YES, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_NO, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_ABORT, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_RETRY, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_IGNORE, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_SAVE, WMessageDialog::OnButton)
    EVT_BUTTON(wxID_EXIT, WMessageDialog::OnButton)

END_EVENT_TABLE();

void wxMessageDialogErrorOK(wxWindow* parent, const wxString& message)
{
    wxMessageDialog dlg(parent, message,
                        _("CryptoTE"), wxOK | wxICON_ERROR);

    dlg.ShowModal();
}

/******************************************************************************/
