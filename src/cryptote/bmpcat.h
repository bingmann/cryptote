/*******************************************************************************
 * src/cryptote/bmpcat.h
 *
 * Part of CryptoTE, see http://panthema.net/2007/cryptote
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

#ifndef CRYPTOTE_SRC_BMPCAT_HEADER
#define CRYPTOTE_SRC_BMPCAT_HEADER

#include <wx/bitmap.h>
#include <wx/string.h>

class BitmapCatalog
{
private:
    struct ThemeEntry
    {
        int                identifier;
        int                usage;
        const unsigned char* data;
        size_t             datalen;
    };

    struct Theme
    {
        const wxString     name;
        const unsigned char* snapshot_data;
        size_t             snapshot_datalen;
        const ThemeEntry   * entries;
    };

    // *** First Theme: KDE's Crystal ***

    const static ThemeEntry bitmaplist_crystal_large[];
    const static ThemeEntry bitmaplist_crystal_small[];

    // *** Second Theme: KDE's Slick ***

    const static ThemeEntry bitmaplist_slick_large[];
    const static ThemeEntry bitmaplist_slick_small[];

    // *** Third Theme: Gnome's Standard ***

    const static ThemeEntry bitmaplist_gnome_large[];
    const static ThemeEntry bitmaplist_gnome_small[];

    // *** List of Built-In Themes ***

    /// This function is used because the arrays contain translated strings.
    static const Theme ** GetThemeList(int& size);

    struct BitmapInfo
    {
        const int      identifier;
        const int      usage;
        const wxString name;
        wxBitmap       current;
    };

    /// Array of bitmap initialized from the theme
    static struct BitmapInfo bitmaplist[];

    /// Selected Theme Id
    int themeid;

    void AddBuiltInTheme(const Theme* theme);

protected:
    /// Construct and initialize the bitmap catalog
    BitmapCatalog();

    /// Singleton class
    static BitmapCatalog* singleton;

    /// Registered wxArtProvider derived class
    class BitmapCatalogArtProvider* artprovider;

public:
    /// Set the current bitmap/icon theme
    void SetTheme(int themeid);

    /// Return current theme id
    int GetCurrentTheme();

    /// Return info about a built-in theme. returns true if the index was
    /// valid.
    bool GetThemeInfo(int themeid, wxString& name, wxBitmap& snapshot);

    /// Return a bitmap for the given general identifier
    wxBitmap _GetBitmap(int id);

    /// Return a bitmap for the menu identifier id.
    wxBitmap _GetMenuBitmap(int id);

    /// Return a bitmap for the toolbar identifier id.
    wxBitmap _GetToolbarBitmap(int id);

    /// Return a bitmap for the file list.
    wxBitmap _GetFileTypeBitmap(int id);

    /// Return a bitmap for the given general identifier
    static wxBitmap GetBitmap(int id);

    /// Return a bitmap for the menu identifier id.
    static wxBitmap GetMenuBitmap(int id);

    /// Return a bitmap for the toolbar identifier id.
    static wxBitmap GetToolbarBitmap(int id);

    /// Return a bitmap for the file list.
    static wxBitmap GetFileTypeBitmap(int id);

    /// Return Singleton class
    static BitmapCatalog * GetSingleton();

    /// Register a wxArtProvider providing bitmaps from this object.
    void RegisterArtProvider();
};

#endif // !CRYPTOTE_SRC_BMPCAT_HEADER

/******************************************************************************/
