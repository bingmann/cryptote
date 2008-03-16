// $Id$

#ifndef BITMAPCATALOG_H
#define BITMAPCATALOG_H

#include <wx/string.h>
#include <wx/bitmap.h>

class BitmapCatalog
{
private:

    struct ThemeEntry
    {
	int	identifier;
	int	usage;
	char*	data;
	size_t	datalen;
    };

    struct Theme
    {
	const wxString		name;
	const char*		snapshot_data;
	size_t			snapshot_datalen;
	const ThemeEntry*	entries;
    };

    // *** First Theme: KDE's Crystal ***

    const static Theme		theme_crystal_large;
    const static ThemeEntry	bitmaplist_crystal_large[];

    const static Theme		theme_crystal_small;
    const static ThemeEntry	bitmaplist_crystal_small[];

    // *** Second Theme: KDE's Slick ***

    const static Theme		theme_slick_large;
    const static ThemeEntry	bitmaplist_slick_large[];

    const static Theme		theme_slick_small;
    const static ThemeEntry	bitmaplist_slick_small[];

    // *** Third Theme: Gnome's Standard ***

    const static Theme		theme_gnome_large;
    const static ThemeEntry	bitmaplist_gnome_large[];

    const static Theme		theme_gnome_small;
    const static ThemeEntry	bitmaplist_gnome_small[];

    // *** List of Built-In Themes ***

    const static Theme*		themelist[];
    const static int		themelistsize;

    struct BitmapInfo
    {
	const int     	identifier;
	const int	usage;
	const wxString	name;
	wxBitmap	current;
    };

    /// Array of bitmap initialized from the theme
    static struct BitmapInfo	bitmaplist[];

    /// Selected Theme Id
    int		themeid;

    void	AddBuiltInTheme(const Theme* theme);

protected:
    /// Construct and initialize the bitmap catalog
    BitmapCatalog();

    /// Singleton class
    static BitmapCatalog*	singleton;

    /// Registered wxArtProvider derived class
    class BitmapCatalogArtProvider* artprovider;

public:

    /// Set the current bitmap/icon theme
    void	SetTheme(int themeid);

    /// Return current theme id
    int		GetCurrentTheme();

    /// Return info about a built-in theme. returns true if the index was
    /// valid.
    bool	GetThemeInfo(int themeid, wxString& name, wxBitmap& snapshot);

    /// Return a bitmap for the given general identifier
    wxBitmap	_GetBitmap(int id);

    /// Return a bitmap for the menu identifier id.
    wxBitmap	_GetMenuBitmap(int id);

    /// Return a bitmap for the toolbar identifier id.
    wxBitmap	_GetToolbarBitmap(int id);

    /// Return a bitmap for the file list.
    wxBitmap	_GetFileTypeBitmap(int id);

    /// Return a bitmap for the given general identifier
    static wxBitmap	GetBitmap(int id);

    /// Return a bitmap for the menu identifier id.
    static wxBitmap	GetMenuBitmap(int id);

    /// Return a bitmap for the toolbar identifier id.
    static wxBitmap	GetToolbarBitmap(int id);

    /// Return a bitmap for the file list.
    static wxBitmap	GetFileTypeBitmap(int id);

    /// Return Singleton class
    static BitmapCatalog*	GetSingleton();

    /// Register a wxArtProvider providing bitmaps from this object.
    void		RegisterArtProvider();
};

#endif // BITMAPCATALOG_H
