// $Id$

#ifndef CEWEDIT_H
#define CEWEDIT_H

#include <wx/wx.h>
#include <stc.h>

class CEWEdit : public wxStyledTextCtrl
{
public:
    CEWEdit(wxWindow *parent, wxWindowID id = wxID_ANY,
	    const wxPoint& pos = wxDefaultPosition,
	    const wxSize& size = wxDefaultSize,
	    long style = 0);

    // *** Event Handlers

protected:

    DECLARE_EVENT_TABLE()
};

#endif // CEWEDIT_H
