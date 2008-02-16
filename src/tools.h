// $Id$

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <wx/mstream.h>

// *** Missing in 2.8, but always added by wxGlade ***
#ifndef wxTHICK_FRAME
#define wxTHICK_FRAME wxRESIZE_BORDER
#endif

// *** Some functions to load a compiled-in transparent PNG as wxBitmap or
// *** wxIcon

#define wxBitmapFromMemory(name) _wxBitmapFromMemory(name, sizeof(name))

inline wxBitmap _wxBitmapFromMemory(const char *data, int len) {
    wxMemoryInputStream is(data, len);
    return wxBitmap(wxImage(is, wxBITMAP_TYPE_PNG, -1), -1);
}

#define wxIconFromMemory(name) _wxIconFromMemory(name, sizeof(name))

inline wxIcon _wxIconFromMemory(const char *data, int len) {
    wxIcon icon;
    icon.CopyFromBitmap( _wxBitmapFromMemory(data, len) );
    return icon;
}

#endif // __TOOLS_H__
