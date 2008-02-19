// $Id$

#include "cewedit.h"

CEWEdit::CEWEdit(wxWindow *parent, wxWindowID id,
		 const wxPoint &pos, const wxSize &size,
		 long style)
    : wxStyledTextCtrl(parent, id, pos, size, style)
{
}

BEGIN_EVENT_TABLE(CEWEdit, wxStyledTextCtrl)

END_EVENT_TABLE()
