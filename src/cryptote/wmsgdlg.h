/*******************************************************************************
 * src/cryptote/wmsgdlg.h
 *
 * Based on wxWidgets-2.8.7/include/generic/msgdlgg.h
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

#ifndef CRYPTOTE_SRC_WMSGDLG_HEADER
#define CRYPTOTE_SRC_WMSGDLG_HEADER

#include <wx/wx.h>

/**
 * Used to show simple message dialog boxes. This version is more flexible in
 * the buttons that can be displayed than the standand wxWidgets wxMessageBox.
 */

class WMessageDialog : public wxDialog
{
public:
    WMessageDialog(wxWindow* parent, const wxString& message,
                   const wxString& caption,
                   long style = 0,
                   int button0 = 0, int button1 = 0, int button2 = 0);

    wxButton * CreateButton(int id);

    void OnButton(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);

private:
    int button0;
    int button1;
    int button2;

    DECLARE_EVENT_TABLE()
};

extern void wxMessageDialogErrorOK(wxWindow* parent, const wxString& message);

#endif // !CRYPTOTE_SRC_WMSGDLG_HEADER

/******************************************************************************/
