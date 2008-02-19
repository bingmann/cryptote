// $Id$

#include "cewmain.h"

CEWMain::CEWMain(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(750, 550),
	      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetTitle(_("CryptoTE v0.1"));

    Centre();
}
