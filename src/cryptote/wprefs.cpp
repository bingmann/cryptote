/*******************************************************************************
 * src/cryptote/wprefs.cpp
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

#include "wprefs.h"
#include "wcryptote.h"
#include "bmpcat.h"
#include "common/tools.h"

#include <wx/config.h>

// begin wxGlade: ::extracode
// end wxGlade

WPreferences::WPreferences(WCryptoTE* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long WXUNUSED(style))
    : wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE),
      wmain(parent)
{
    // begin wxGlade: WPreferences::WPreferences
    notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    notebook_pane3 = new wxPanel(notebook, wxID_ANY);
    notebook_pane2 = new wxPanel(notebook, wxID_ANY);
    notebook_pane1 = new wxPanel(notebook, wxID_ANY);
    sizerA4_staticbox = new wxStaticBox(notebook_pane1, -1, _("Automatic Closing"));
    sizerA6_staticbox = new wxStaticBox(notebook_pane1, -1, _("Share Locks"));
    sizerB2_staticbox = new wxStaticBox(notebook_pane2, -1, _("Icon Theme"));
    sizerC2_staticbox = new wxStaticBox(notebook_pane3, -1, _("Update Check"));
    sizerA2_staticbox = new wxStaticBox(notebook_pane1, -1, _("Backup Files"));
    checkboxBackups = new wxCheckBox(notebook_pane1, myID_CHECK_BACKUPS, _("Keep backups of container during saving."));
    labelBackup1 = new wxStaticText(notebook_pane1, wxID_ANY, _("Number of backups to keep: "));
    spinctrlBackupNum = new wxSpinCtrl(notebook_pane1, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1000);
    checkboxAutoClose = new wxCheckBox(notebook_pane1, myID_CHECK_AUTOCLOSE, _("Automatically save and close loaded containers"));
    labelAutoClose1 = new wxStaticText(notebook_pane1, wxID_ANY, _("after"));
    spinctrlAutoCloseTime = new wxSpinCtrl(notebook_pane1, wxID_ANY, wxT("15"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10000);
    labelAutoClose2 = new wxStaticText(notebook_pane1, wxID_ANY, _("minutes of inactivity."));
    checkboxAutoCloseExit = new wxCheckBox(notebook_pane1, wxID_ANY, _("Also close CryptoTE after saving the container."));
    checkboxShareLock = new wxCheckBox(notebook_pane1, wxID_ANY, _("Keep exclusive Share-Lock on container file."));
    labelShareLock1 = new wxStaticText(notebook_pane1, wxID_ANY, _("Other users cannot open it while loaded."));
    listboxTheme = new wxSimpleImageListBox(notebook_pane2, wxID_ANY);
    checkboxWebUpdateCheck = new wxCheckBox(notebook_pane3, wxID_ANY, _("Automatically check for updated versions."));
    buttonOK = new wxButton(this, wxID_OK, wxEmptyString);
    buttonCancel = new wxButton(this, wxID_CANCEL, wxEmptyString);

    set_properties();
    do_layout();
    // end wxGlade

    // *** Change dialog depending on OS ***

#if defined(__WINDOWS__)
    labelShareLock1->SetLabel(_("Windows: No other user or program can open the container file while it is loaded in CryptoTE."));
#elif defined(__LINUX__)
    labelShareLock1->SetLabel(_("Linux: advisory file locking is used.\nNo two instances of CryptoTE can open the same file. Mandatory file locking can only be enabled using a mount option (see man mount, option 'mand')."));
#else
    checkboxShareLock->Disable();
    labelShareLock1->SetLabel(_("Unknown OS: share locking is not supported."));
#endif
    labelShareLock1->Wrap(340);
    Layout();
    GetSizer()->Fit(this);
    Centre();

    // *** Load current settings from WCryptoTE ***

    checkboxBackups->SetValue(wmain->prefs_makebackups);
    spinctrlBackupNum->SetValue(wmain->prefs_backupnum);

    checkboxAutoClose->SetValue(wmain->prefs_autoclose);
    spinctrlAutoCloseTime->SetValue(wmain->prefs_autoclosetime);
    checkboxAutoCloseExit->SetValue(wmain->prefs_autocloseexit);

    checkboxShareLock->SetValue(wmain->prefs_sharelock);

    checkboxWebUpdateCheck->SetValue(wmain->prefs_webupdatecheck);

    wxCommandEvent event;
    OnCheckboxBackups(event);
    OnCheckboxAutoClose(event);

    // *** Initialize the Theme List ***

    listboxTheme->SetImageSpacing(6, 6, 6);

    BitmapCatalog* bitmapcatalog = BitmapCatalog::GetSingleton();

    for (int ti = 0; ; ++ti)
    {
        wxString str;
        wxBitmap bmp;

        if (!bitmapcatalog->GetThemeInfo(ti, str, bmp)) break;

        listboxTheme->Append(str);
        listboxTheme->SetBitmap(ti, bmp);
    }

    listboxTheme->SetSelection(bitmapcatalog->GetCurrentTheme());
}

void WPreferences::set_properties()
{
    // begin wxGlade: WPreferences::set_properties
    SetTitle(_("CryptoTE Preferences"));
    // end wxGlade
}

void WPreferences::do_layout()
{
    // begin wxGlade: WPreferences::do_layout
    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizerC1 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizerC2 = new wxStaticBoxSizer(sizerC2_staticbox, wxVERTICAL);
    wxBoxSizer* sizerB1 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizerB2 = new wxStaticBoxSizer(sizerB2_staticbox, wxVERTICAL);
    wxBoxSizer* sizerA1 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizerA6 = new wxStaticBoxSizer(sizerA6_staticbox, wxVERTICAL);
    wxStaticBoxSizer* sizerA4 = new wxStaticBoxSizer(sizerA4_staticbox, wxVERTICAL);
    wxBoxSizer* sizerA5 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticBoxSizer* sizerA2 = new wxStaticBoxSizer(sizerA2_staticbox, wxVERTICAL);
    wxFlexGridSizer* sizerA3 = new wxFlexGridSizer(2, 2, 0, 0);
    sizerA2->Add(checkboxBackups, 0, wxALL, 4);
    sizerA3->Add(labelBackup1, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 4);
    sizerA3->Add(spinctrlBackupNum, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 4);
    sizerA2->Add(sizerA3, 1, wxEXPAND, 0);
    sizerA1->Add(sizerA2, 0, wxALL | wxEXPAND, 8);
    sizerA4->Add(checkboxAutoClose, 0, wxALL, 4);
    sizerA5->Add(labelAutoClose1, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 4);
    sizerA5->Add(spinctrlAutoCloseTime, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 4);
    sizerA5->Add(labelAutoClose2, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 4);
    sizerA4->Add(sizerA5, 0, wxEXPAND, 0);
    sizerA4->Add(checkboxAutoCloseExit, 0, wxLEFT | wxRIGHT | wxBOTTOM, 4);
    sizerA1->Add(sizerA4, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 8);
    sizerA6->Add(checkboxShareLock, 0, wxALL, 4);
    sizerA6->Add(labelShareLock1, 0, wxLEFT | wxRIGHT | wxBOTTOM, 4);
    sizerA1->Add(sizerA6, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 8);
    sizerA1->Add(0, 0, 1, 0, 0);
    notebook_pane1->SetSizer(sizerA1);
    sizerB2->Add(listboxTheme, 1, wxALL | wxEXPAND, 4);
    sizerB1->Add(sizerB2, 1, wxALL | wxEXPAND, 8);
    notebook_pane2->SetSizer(sizerB1);
    sizerC2->Add(checkboxWebUpdateCheck, 0, wxALL, 4);
    wxStaticText* labelWebUpdateCheck1 = new wxStaticText(notebook_pane3, wxID_ANY, _("Checks for updated versions of CryptoTE by\nquerying the web. Checks once every 24h,\nand only during program idle time. No data\nother than the current version of CryptoTE\nis sent. No automatic download is started."));
    sizerC2->Add(labelWebUpdateCheck1, 0, wxLEFT | wxRIGHT | wxBOTTOM, 4);
    sizerC1->Add(sizerC2, 0, wxALL | wxEXPAND, 8);
    notebook_pane3->SetSizer(sizerC1);
    notebook->AddPage(notebook_pane1, _("Containers"));
    notebook->AddPage(notebook_pane2, _("Icons"));
    notebook->AddPage(notebook_pane3, _("WebUpdate"));
    sizer1->Add(notebook, 1, wxALL | wxEXPAND, 8);
    sizer2->Add(5, 5, 1, 0, 0);
    sizer2->Add(buttonOK, 0, wxLEFT | wxTOP | wxBOTTOM, 4);
    sizer2->Add(buttonCancel, 0, wxALL, 4);
    sizer1->Add(sizer2, 0, wxEXPAND, 0);
    SetSizer(sizer1);
    sizer1->Fit(this);
    Layout();
    Centre();
    // end wxGlade
}

BEGIN_EVENT_TABLE(WPreferences, wxDialog)
    // begin wxGlade: WPreferences::event_table
    EVT_CHECKBOX(myID_CHECK_BACKUPS, WPreferences::OnCheckboxBackups)
    EVT_CHECKBOX(myID_CHECK_AUTOCLOSE, WPreferences::OnCheckboxAutoClose)
    EVT_BUTTON(wxID_ANY, WPreferences::OnButtonOK)
    // end wxGlade
END_EVENT_TABLE();

void WPreferences::OnCheckboxBackups(wxCommandEvent& WXUNUSED(event))
{
    labelBackup1->Enable(checkboxBackups->GetValue());
    spinctrlBackupNum->Enable(checkboxBackups->GetValue());
}

void WPreferences::OnCheckboxAutoClose(wxCommandEvent& WXUNUSED(event))
{
    labelAutoClose1->Enable(checkboxAutoClose->GetValue());
    spinctrlAutoCloseTime->Enable(checkboxAutoClose->GetValue());
    labelAutoClose2->Enable(checkboxAutoClose->GetValue());
    checkboxAutoCloseExit->Enable(checkboxAutoClose->GetValue());
}

void WPreferences::OnButtonOK(wxCommandEvent& WXUNUSED(event))
{
    wxConfigBase* cfg = wxConfigBase::Get();

    cfg->SetPath(_T("/cryptote"));

    long bitmaptheme = listboxTheme->GetSelection();
    if (bitmaptheme >= 0) {
        cfg->Write(_T("bitmaptheme"), bitmaptheme);
    }

    cfg->Write(_T("backups"), checkboxBackups->GetValue());
    cfg->Write(_T("backupnum"), spinctrlBackupNum->GetValue());

    cfg->Write(_T("autoclose"), checkboxAutoClose->GetValue());
    cfg->Write(_T("autoclosetime"), spinctrlAutoCloseTime->GetValue());
    cfg->Write(_T("autocloseexit"), checkboxAutoCloseExit->GetValue());

    cfg->Write(_T("sharelock"), checkboxShareLock->GetValue());

    cfg->Write(_T("webupdatecheck"), checkboxWebUpdateCheck->GetValue());

    cfg->Flush();

    EndModal(wxID_OK);
}

/******************************************************************************/
