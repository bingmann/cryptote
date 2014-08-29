/*******************************************************************************
 * src/cryptote/wprefs.h
 *
 * Part of CryptoTE v0.5.999, see http://panthema.net/2007/cryptote
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

#ifndef CRYPTOTE_SRC_WPREFS_HEADER
#define CRYPTOTE_SRC_WPREFS_HEADER

#include <wx/wx.h>
#include <wx/image.h>

// begin wxGlade: ::dependencies
#include <wx/spinctrl.h>
#include <wx/notebook.h>
// end wxGlade

#include "imaglbox.h"

// begin wxGlade: ::extracode
// end wxGlade

class WPreferences : public wxDialog
{
public:
    // begin wxGlade: WPreferences::ids
    enum {
        myID_CHECK_BACKUPS = wxID_HIGHEST + 1000,
        myID_CHECK_AUTOCLOSE = wxID_HIGHEST + 1002
    };
    // end wxGlade

    WPreferences(class WCryptoTE* parent, int id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

    /// Reference to parent window class
    class WCryptoTE* wmain;

private:
    // begin wxGlade: WPreferences::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: WPreferences::attributes
    wxStaticBox* sizerC2_staticbox;
    wxStaticBox* sizerB2_staticbox;
    wxStaticBox* sizerA6_staticbox;
    wxStaticBox* sizerA4_staticbox;
    wxStaticBox* sizerA2_staticbox;
    wxCheckBox* checkboxBackups;
    wxStaticText* labelBackup1;
    wxSpinCtrl* spinctrlBackupNum;
    wxCheckBox* checkboxAutoClose;
    wxStaticText* labelAutoClose1;
    wxSpinCtrl* spinctrlAutoCloseTime;
    wxStaticText* labelAutoClose2;
    wxCheckBox* checkboxAutoCloseExit;
    wxCheckBox* checkboxShareLock;
    wxStaticText* labelShareLock1;
    wxPanel* notebook_pane1;
    wxSimpleImageListBox* listboxTheme;
    wxPanel* notebook_pane2;
    wxCheckBox* checkboxWebUpdateCheck;
    wxPanel* notebook_pane3;
    wxNotebook* notebook;
    wxButton* buttonOK;
    wxButton* buttonCancel;
    // end wxGlade

    DECLARE_EVENT_TABLE();

public:
    virtual void OnCheckboxBackups(wxCommandEvent& event);   // wxGlade: <event_handler>
    virtual void OnCheckboxAutoClose(wxCommandEvent& event); // wxGlade: <event_handler>
    virtual void OnButtonOK(wxCommandEvent& event);          // wxGlade: <event_handler>
};                                                           // wxGlade: end class

#endif // !CRYPTOTE_SRC_WPREFS_HEADER

/******************************************************************************/
