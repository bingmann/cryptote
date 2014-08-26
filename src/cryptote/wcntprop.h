/*******************************************************************************
 * src/cryptote/wcntprop.h
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

#ifndef CRYPTOTE_SRC_WCNTPROP_HEADER
#define CRYPTOTE_SRC_WCNTPROP_HEADER

#include <wx/wx.h>
// begin wxGlade: ::dependencies
#include <wx/statline.h>
// end wxGlade

// begin wxGlade: ::extracode
// end wxGlade

class WContainerProperties : public wxDialog
{
public:
    // begin wxGlade: WContainerProperties::ids
    // end wxGlade

    WContainerProperties(class WCryptoTE* parent, int id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

protected:
    /// Reference to main window
    class WCryptoTE* wmain;

private:
    // begin wxGlade: WContainerProperties::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: WContainerProperties::attributes
    wxTextCtrl* textFilename;
    wxTextCtrl* textSize;
    wxTextCtrl* textSubFileNum;
    wxTextCtrl* textCTime;
    wxTextCtrl* textMTime;
    wxChoice* choiceCompression;
    wxChoice* choiceEncryption;
    wxCheckBox* checkboxRestoreView;
    wxTextCtrl* textAuthor;
    wxTextCtrl* textSubject;
    wxTextCtrl* textDescription;
    wxButton* buttonOK;
    wxButton* buttonCancel;
    // end wxGlade

    DECLARE_EVENT_TABLE();

public:
    virtual void OnButtonOK(wxCommandEvent& event); // wxGlade: <event_handler>
};                                                  // wxGlade: end class

#endif // !CRYPTOTE_SRC_WCNTPROP_HEADER

/******************************************************************************/
