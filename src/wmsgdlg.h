// $Id$
// Based on wxWidgets-2.8.7/include/generic/msgdlgg.h

#ifndef WMSGDLG_H
#define WMSGDLG_H

#include <wx/wx.h>

/**
 * Used to show simple message dialog boxes. This version is more flexible in
 * the buttons that can be displayed than the standand wxWidgets wxMessageBox.
 */

class WMessageDialog : public wxDialog
{
public:

    WMessageDialog(wxWindow *parent, const wxString& message,
		   const wxString& caption,
		   long style = 0,
		   int button0 = 0, int button1 = 0, int button2 = 0);

    wxButton*	CreateButton(int id);

    void	OnButton(wxCommandEvent& event);
    void	OnButtonCancel(wxCommandEvent& event);

private:

    int		button0;
    int		button1;
    int		button2;

    DECLARE_EVENT_TABLE()
};

#endif // WMSGDLG_H
