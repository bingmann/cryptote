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

static inline wxBitmap _wxBitmapFromMemory(const char *data, int len) {
    wxMemoryInputStream is(data, len);
    return wxBitmap(wxImage(is, wxBITMAP_TYPE_PNG, -1), -1);
}

#define wxIconFromMemory(name) _wxIconFromMemory(name, sizeof(name))

static inline wxIcon _wxIconFromMemory(const char *data, int len) {
    wxIcon icon;
    icon.CopyFromBitmap( _wxBitmapFromMemory(data, len) );
    return icon;
}

// *** Interpolate a color from the gradient given by support points.

struct GradientPoint
{
    int		value;
    wxColour	colour;
};

static inline wxColour MixColours(double percent, const wxColour& c1, const wxColor& c2)
{
    return wxColour( (unsigned char)( percent * c1.Red()   + (1-percent) * c2.Red() ),
		     (unsigned char)( percent * c1.Green() + (1-percent) * c2.Green() ),
		     (unsigned char)( percent * c1.Blue()  + (1-percent) * c2.Blue() ) );
}

static inline wxColor InterpolateGradient(int value, const struct GradientPoint* gradient, int gradientsize)
{
    for(int gp = 0; gp < gradientsize; ++gp)
    {
	if (value <= gradient[gp].value)
	{
	    if (gp == 0) {
		return gradient[gp].colour;
	    }
	    else {
		double percent = double(value - gradient[gp-1].value) / double(gradient[gp].value - gradient[gp-1].value);
		return MixColours(1.0 - percent, gradient[gp-1].colour, gradient[gp].colour);
	    }
	}
    }
    return gradient[gradientsize-1].colour;
}

#endif // __TOOLS_H__
