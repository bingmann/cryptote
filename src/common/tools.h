// $Id$

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <wx/mstream.h>
#include <wx/string.h>
#include <string>

// *** Missing in 2.8, but always added by wxGlade ***
#ifndef wxTHICK_FRAME
#define wxTHICK_FRAME wxRESIZE_BORDER
#endif

// *** Some functions to load a compiled-in transparent PNG as wxBitmap or
// *** wxIcon

#define wxBitmapFromMemory(name) wxBitmapFromMemory2(name, sizeof(name))

static inline wxBitmap wxBitmapFromMemory2(const char *data, int len) {
    wxMemoryInputStream is(data, len);
    return wxBitmap(wxImage(is, wxBITMAP_TYPE_PNG, -1), -1);
}

#define wxIconFromMemory(name) wxIconFromMemory2(name, sizeof(name))

static inline wxIcon wxIconFromMemory2(const char *data, int len) {
    wxIcon icon;
    icon.CopyFromBitmap( wxBitmapFromMemory2(data, len) );
    return icon;
}

// *** Somewhat safe conversions between wxString and std::string ***

static inline wxString strSTL2WX(const std::string& str) {
    return wxString(str.data(), wxConvUTF8, str.size());
}

static inline std::string strWX2STL(const wxString& str) {
#if wxUSE_UNICODE
    size_t outlen;
    const wxCharBuffer cbuf = wxConvUTF8.cWC2MB(str.GetData(), str.Length(), &outlen);
    return std::string(cbuf.data(), outlen);
#else
    return std::string(str.GetData(), str.Length());
#endif
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
