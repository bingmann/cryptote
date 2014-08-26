/*******************************************************************************
 * src/cryptote/wfind.h
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

#ifndef CRYPTOTE_SRC_WFIND_HEADER
#define CRYPTOTE_SRC_WFIND_HEADER

#include <wx/wx.h>
#include <wx/image.h>

// begin wxGlade: ::dependencies
#include <wx/statline.h>
// end wxGlade

// begin wxGlade: ::extracode
// end wxGlade

class WFindReplace : public wxPanel
{
public:
    // begin wxGlade: WFindReplace::ids
    enum {
        myID_COMBO_FIND = wxID_HIGHEST + 1000,
        myID_REPLACEALL = wxID_HIGHEST + 1002
    };
    // end wxGlade

    WFindReplace(class WCryptoTE* parent, int id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);

    /// Change dialog to show replacement controls
    void ShowReplace(bool show);

protected:
    /// Reference to main window
    class WCryptoTE* wmain;

    /// Whether this is a find&replace box
    bool showreplace;

    /// True if the current search string was found and can be replaced.
    bool havefound;

private:
    // begin wxGlade: WFindReplace::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: WFindReplace::attributes
    wxStaticBox* sizer3_staticbox;
    wxComboBox* comboFind;
    wxStaticText* labelReplace;
    wxComboBox* comboReplace;
    wxCheckBox* checkboxMatchWholeWord;
    wxCheckBox* checkboxMatchCaseInsensitive;
    wxCheckBox* checkboxMatchRegex;
    wxCheckBox* checkboxSearchBackwards;
    wxStaticLine* staticline1;
    wxButton* buttonFind;
    wxButton* buttonReplace;
    wxButton* buttonReplaceAll;
    wxButton* buttonClose;
    // end wxGlade

    DECLARE_EVENT_TABLE();

public:
    virtual void OnComboTextSearch(wxCommandEvent& event);  // wxGlade: <event_handler>
    virtual void OnButtonFind(wxCommandEvent& event);       // wxGlade: <event_handler>
    virtual void OnButtonReplace(wxCommandEvent& event);    // wxGlade: <event_handler>
    virtual void OnButtonReplaceAll(wxCommandEvent& event); // wxGlade: <event_handler>
    virtual void OnButtonClose(wxCommandEvent& event);      // wxGlade: <event_handler>
};                                                          // wxGlade: end class

#endif // !CRYPTOTE_SRC_WFIND_HEADER

/******************************************************************************/
