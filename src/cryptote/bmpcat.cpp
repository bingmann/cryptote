/*******************************************************************************
 * src/cryptote/bmpcat.cpp
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

#include "bmpcat.h"

#include "wcryptote.h"
#include "common/tools.h"

#include <wx/artprov.h>

// *** BitmapManager Code ***

enum bitmapusage_type { BU_GENERAL, BU_MENU, BU_TOOLBAR, BU_FILETYPE };

class BitmapCatalog* BitmapCatalog::singleton = NULL;

class BitmapCatalog* BitmapCatalog::GetSingleton()
{
    if (!singleton)
    {
        singleton = new BitmapCatalog;
        singleton->RegisterArtProvider();
    }
    return singleton;
}

class BitmapCatalog * BitmapCatalog()
{
    return BitmapCatalog::GetSingleton();
}

BitmapCatalog::BitmapCatalog()
    : artprovider(NULL)
{
    SetTheme(2);
}

void BitmapCatalog::SetTheme(int nt)
{
    themeid = nt;

    // flush bitmap cache
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
        bitmaplist[i].current = wxNullBitmap;
    }

    int themelistsize;
    const Theme** themelist = GetThemeList(themelistsize);

    // load built-in theme
    if (nt >= 0 && nt < themelistsize)
    {
        AddBuiltInTheme(themelist[nt]);
    }
    else
    {
        AddBuiltInTheme(themelist[0]);
    }
}

int BitmapCatalog::GetCurrentTheme()
{
    return themeid;
}

bool BitmapCatalog::GetThemeInfo(int themeid, wxString& name, wxBitmap& snapshot)
{
    int themelistsize;
    const Theme** themelist = GetThemeList(themelistsize);

    if (themeid >= 0 && themeid < themelistsize)
    {
        name = themelist[themeid]->name;
        snapshot = wxBitmapFromMemory2(themelist[themeid]->snapshot_data, themelist[themeid]->snapshot_datalen);
        return true;
    }
    return false;
}

void BitmapCatalog::AddBuiltInTheme(const Theme* theme)
{
    assert(theme);

    for (unsigned int bi = 0; theme->entries[bi].identifier; ++bi)
    {
        const ThemeEntry& te = theme->entries[bi];

        // put bitmap into all matching slots

        for (unsigned int li = 0; bitmaplist[li].identifier; ++li)
        {
            if (bitmaplist[li].identifier != te.identifier) continue;
            if (te.usage != BU_GENERAL && bitmaplist[li].usage != te.usage) continue;

            bitmaplist[li].current = wxBitmapFromMemory2(te.data, te.datalen);
        }
    }
}

wxBitmap BitmapCatalog::GetBitmap(int id)
{
    return GetSingleton()->_GetBitmap(id);
}

wxBitmap BitmapCatalog::GetMenuBitmap(int id)
{
    return GetSingleton()->_GetMenuBitmap(id);
}

wxBitmap BitmapCatalog::GetToolbarBitmap(int id)
{
    return GetSingleton()->_GetToolbarBitmap(id);
}

wxBitmap BitmapCatalog::GetFileTypeBitmap(int id)
{
    return GetSingleton()->_GetFileTypeBitmap(id);
}

class BitmapCatalogArtProvider : public wxArtProvider
{
public:
    class BitmapCatalog& bmpcat;

    BitmapCatalogArtProvider(class BitmapCatalog& bitmapcatalog)
        : wxArtProvider(),
          bmpcat(bitmapcatalog)
    { }

    virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& WXUNUSED(size))
    {
        if (client == wxART_MESSAGE_BOX)
        {
            if (id == wxART_ERROR)
                return bmpcat.GetBitmap(wxICON_ERROR);
            else if (id == wxART_WARNING)
                return bmpcat.GetBitmap(wxICON_WARNING);
            else if (id == wxART_QUESTION)
                return bmpcat.GetBitmap(wxICON_QUESTION);
            else if (id == wxART_INFORMATION)
                return bmpcat.GetBitmap(wxICON_INFORMATION);
        }
        return wxNullBitmap;
    }
};

void BitmapCatalog::RegisterArtProvider()
{
    if (!artprovider)
    {
        artprovider = new BitmapCatalogArtProvider(*this);
        wxArtProvider::Push(artprovider);
    }
}

struct BitmapCatalog::BitmapInfo BitmapCatalog::bitmaplist[] =
{
    // Main Window Menu

    { wxID_OPEN, BU_MENU, _T("container-open"), wxNullBitmap },
    { wxID_SAVE, BU_MENU, _T("container-save"), wxNullBitmap },
    { wxID_SAVEAS, BU_MENU, _T("container-save-as"), wxNullBitmap },
    { wxID_REVERT, BU_MENU, _T("container-revert"), wxNullBitmap },
    { wxID_CLOSE, BU_MENU, _T("container-close"), wxNullBitmap },
    { myID_MENU_CONTAINER_SHOWLIST, BU_MENU, _T("container-showsubfilelist"), wxNullBitmap },
    { wxID_PROPERTIES, BU_MENU, _T("container-properties"), wxNullBitmap },
    { myID_MENU_CONTAINER_PASSLIST, BU_MENU, _T("container-password"), wxNullBitmap },
    { wxID_PREFERENCES, BU_MENU, _T("application-preferences"), wxNullBitmap },
    { wxID_EXIT, BU_MENU, _T("application-exit"), wxNullBitmap },

    { myID_MENU_SUBFILE_NEW, BU_MENU, _T("subfile-new"), wxNullBitmap },
    { myID_MENU_SUBFILE_OPEN, BU_MENU, _T("subfile-open"), wxNullBitmap },
    { myID_MENU_SUBFILE_IMPORT, BU_MENU, _T("subfile-import"), wxNullBitmap },
    { myID_MENU_SUBFILE_EXPORT, BU_MENU, _T("subfile-export"), wxNullBitmap },
    { myID_MENU_SUBFILE_PROPERTIES, BU_MENU, _T("subfile-properties"), wxNullBitmap },
    { myID_MENU_SUBFILE_RENAME, BU_MENU, _T("subfile-rename"), wxNullBitmap },
    { myID_MENU_SUBFILE_CLOSE, BU_MENU, _T("subfile-close"), wxNullBitmap },
    { myID_MENU_SUBFILE_DELETE, BU_MENU, _T("subfile-delete"), wxNullBitmap },

    { wxID_UNDO, BU_MENU, _T("edit-undo"), wxNullBitmap },
    { wxID_REDO, BU_MENU, _T("edit-redo"), wxNullBitmap },
    { wxID_CUT, BU_MENU, _T("edit-cut"), wxNullBitmap },
    { wxID_COPY, BU_MENU, _T("edit-copy"), wxNullBitmap },
    { wxID_PASTE, BU_MENU, _T("edit-paste"), wxNullBitmap },
    { wxID_CLEAR, BU_MENU, _T("edit-clear"), wxNullBitmap },
    { myID_MENU_EDIT_QUICKFIND, BU_MENU, _T("edit-quickfind"), wxNullBitmap },
    { wxID_FIND, BU_MENU, _T("edit-find"), wxNullBitmap },
    { wxID_REPLACE, BU_MENU, _T("edit-replace"), wxNullBitmap },
    { myID_MENU_EDIT_GOTO, BU_MENU, _T("edit-goto"), wxNullBitmap },
    { wxID_SELECTALL, BU_MENU, _T("edit-select-all"), wxNullBitmap },
    { myID_MENU_EDIT_SELECTLINE, BU_MENU, _T("edit-select-line"), wxNullBitmap },
    { myID_MENU_EDIT_INSERT_PASSWORD, BU_MENU, _T("edit-insert-password"), wxNullBitmap },
    { myID_MENU_EDIT_INSERT_DATETIME, BU_MENU, _T("edit-insert-datetime"), wxNullBitmap },

    { wxID_ADD, BU_MENU, _T("list-add"), wxNullBitmap },
    { wxID_REMOVE, BU_MENU, _T("list-remove"), wxNullBitmap },

    { myID_MENU_VIEW_ZOOM, BU_MENU, _T("view-zoom"), wxNullBitmap },
    { myID_MENU_VIEW_ZOOM_INCREASE, BU_MENU, _T("view-zoom-increase"), wxNullBitmap },
    { myID_MENU_VIEW_ZOOM_DECREASE, BU_MENU, _T("view-zoom-decrease"), wxNullBitmap },
    { myID_MENU_VIEW_ZOOM_RESET, BU_MENU, _T("view-zoom-reset"), wxNullBitmap },

    { myID_MENU_HELP_WEBUPDATECHECK, BU_MENU, _T("application-webupdatecheck"), wxNullBitmap },
    { wxID_ABOUT, BU_MENU, _T("application-about"), wxNullBitmap },

    { myID_MENU_VIEW_BIGICONS, BU_MENU, _T("view-bigicons"), wxNullBitmap },
    { myID_MENU_VIEW_LIST, BU_MENU, _T("view-list"), wxNullBitmap },
    { myID_MENU_VIEW_REPORT, BU_MENU, _T("view-report"), wxNullBitmap },

    // Main Window Toolbar

    { wxID_OPEN, BU_TOOLBAR, _T("container-open-tool"), wxNullBitmap },
    { wxID_SAVE, BU_TOOLBAR, _T("container-save-tool"), wxNullBitmap },
    { wxID_SAVEAS, BU_TOOLBAR, _T("container-save-as-tool"), wxNullBitmap },
    { wxID_REVERT, BU_TOOLBAR, _T("container-revert-tool"), wxNullBitmap },
    { wxID_CLOSE, BU_TOOLBAR, _T("container-close-tool"), wxNullBitmap },
    { myID_MENU_CONTAINER_SHOWLIST, BU_TOOLBAR, _T("container-showsubfilelist-tool"), wxNullBitmap },
    { wxID_PROPERTIES, BU_TOOLBAR, _T("container-properties-tool"), wxNullBitmap },
    { myID_MENU_CONTAINER_PASSLIST, BU_TOOLBAR, _T("container-setpassword-tool"), wxNullBitmap },
    { wxID_PREFERENCES, BU_TOOLBAR, _T("application-options"), wxNullBitmap },
    { wxID_EXIT, BU_TOOLBAR, _T("application-exit-tool"), wxNullBitmap },

    { myID_MENU_SUBFILE_NEW, BU_TOOLBAR, _T("subfile-new-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_OPEN, BU_TOOLBAR, _T("subfile-open-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_IMPORT, BU_TOOLBAR, _T("subfile-import-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_EXPORT, BU_TOOLBAR, _T("subfile-export-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_PROPERTIES, BU_TOOLBAR, _T("subfile-properties-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_RENAME, BU_TOOLBAR, _T("subfile-rename-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_CLOSE, BU_TOOLBAR, _T("subfile-close-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_DELETE, BU_TOOLBAR, _T("subfile-delete-tool"), wxNullBitmap },

    { wxID_UNDO, BU_TOOLBAR, _T("edit-undo-tool"), wxNullBitmap },
    { wxID_REDO, BU_TOOLBAR, _T("edit-redo-tool"), wxNullBitmap },
    { wxID_CUT, BU_TOOLBAR, _T("edit-cut-tool"), wxNullBitmap },
    { wxID_COPY, BU_TOOLBAR, _T("edit-copy-tool"), wxNullBitmap },
    { wxID_PASTE, BU_TOOLBAR, _T("edit-paste-tool"), wxNullBitmap },
    { wxID_CLEAR, BU_TOOLBAR, _T("edit-clear-tool"), wxNullBitmap },
    { myID_MENU_EDIT_QUICKFIND, BU_TOOLBAR, _T("edit-quickfind-tool"), wxNullBitmap },
    { wxID_FIND, BU_TOOLBAR, _T("edit-find-tool"), wxNullBitmap },
    { wxID_REPLACE, BU_TOOLBAR, _T("edit-replace-tool"), wxNullBitmap },
    { myID_MENU_EDIT_GOTO, BU_TOOLBAR, _T("edit-goto-tool"), wxNullBitmap },
    { wxID_SELECTALL, BU_TOOLBAR, _T("edit-select-all-tool"), wxNullBitmap },
    { myID_MENU_EDIT_SELECTLINE, BU_TOOLBAR, _T("edit-select-line-tool"), wxNullBitmap },
    { myID_TOOL_EDIT_INSERT_PASSWORD, BU_TOOLBAR, _T("edit-insert-password-tool"), wxNullBitmap },
    { myID_TOOL_EDIT_INSERT_DATETIME, BU_TOOLBAR, _T("edit-insert-datetime-tool"), wxNullBitmap },

    { wxID_ADD, BU_TOOLBAR, _T("list-add-tool"), wxNullBitmap },
    { wxID_REMOVE, BU_TOOLBAR, _T("list-remove-tool"), wxNullBitmap },

    { wxID_ABOUT, BU_TOOLBAR, _T("application-about-tool"), wxNullBitmap },

    { myID_MENU_VIEW_BIGICONS, BU_TOOLBAR, _T("view-bigicons-tool"), wxNullBitmap },
    { myID_MENU_VIEW_LIST, BU_TOOLBAR, _T("view-list-tool"), wxNullBitmap },
    { myID_MENU_VIEW_REPORT, BU_TOOLBAR, _T("view-report-tool"), wxNullBitmap },

    // Other Dialogs and Panes

    { myID_QUICKFIND_CLOSE, BU_GENERAL, _T("button-close"), wxNullBitmap },
    { myID_QUICKFIND_NEXT, BU_GENERAL, _T("button-find-next"), wxNullBitmap },
    { myID_QUICKFIND_PREV, BU_GENERAL, _T("button-find-previous"), wxNullBitmap },

    { myID_QUICKGOTO_CLOSE, BU_GENERAL, _T("button-close"), wxNullBitmap },
    { myID_QUICKGOTO_GO, BU_GENERAL, _T("button-goto"), wxNullBitmap },

    // File Icons

    { myID_FILETYPE_BINARY, BU_FILETYPE, _T("filetype-binary-16"), wxNullBitmap },
    { myID_FILETYPE_TEXT, BU_FILETYPE, _T("filetype-text-16"), wxNullBitmap },
    { myID_FILETYPE_IMAGE, BU_FILETYPE, _T("filetype-image-16"), wxNullBitmap },

    { myID_FILETYPE_BINARY_LARGE, BU_FILETYPE, _T("filetype-binary-32"), wxNullBitmap },
    { myID_FILETYPE_TEXT_LARGE, BU_FILETYPE, _T("filetype-text-32"), wxNullBitmap },
    { myID_FILETYPE_IMAGE_LARGE, BU_FILETYPE, _T("filetype-image-32"), wxNullBitmap },

    // Icons in MessageBoxes

    { wxICON_ERROR, BU_GENERAL, _T("messagebox-error"), wxNullBitmap },
    { wxICON_WARNING, BU_GENERAL, _T("messagebox-warning"), wxNullBitmap },
    { wxICON_QUESTION, BU_GENERAL, _T("messagebox-question"), wxNullBitmap },
    { wxICON_INFORMATION, BU_GENERAL, _T("messagebox-information"), wxNullBitmap },

    // Other Images

    { myID_IMAGE_USERKEYSLOT, BU_GENERAL, _T("userkeyslot"), wxNullBitmap },
    { myID_IMAGE_USERKEYSLOT_ACTIVE, BU_GENERAL, _T("userkeyslot-active"), wxNullBitmap },

    { 0, BU_GENERAL, wxEmptyString, wxNullBitmap }
};

wxBitmap BitmapCatalog::_GetBitmap(int id)
{
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
        if (bitmaplist[i].identifier == id && bitmaplist[i].usage == BU_GENERAL)
            return bitmaplist[i].current;
    }

    wxLogDebug(_T("Bitmap request for unknown identifier %u general usage."), id);

    return wxNullBitmap;
}

wxBitmap BitmapCatalog::_GetMenuBitmap(int id)
{
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
        if (bitmaplist[i].identifier == id && bitmaplist[i].usage == BU_MENU)
            return bitmaplist[i].current;
    }

    wxLogDebug(_T("Bitmap request for unknown identifier %u menu usage."), id);

    return _GetBitmap(id);
}

wxBitmap BitmapCatalog::_GetToolbarBitmap(int id)
{
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
        if (bitmaplist[i].identifier == id && bitmaplist[i].usage == BU_TOOLBAR)
            return bitmaplist[i].current;
    }

    wxLogDebug(_T("Bitmap request for unknown identifier %u toolbar usage."), id);

    return _GetMenuBitmap(id);
}

/// Return an associated bitmap for the file list.
wxBitmap BitmapCatalog::_GetFileTypeBitmap(int id)
{
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
        if (bitmaplist[i].identifier == id && bitmaplist[i].usage == BU_FILETYPE)
            return bitmaplist[i].current;
    }

    wxLogDebug(_T("Bitmap request for unknown identifier %u filetype usage."), id);

    return wxNullBitmap;
}

// *** Built-In Themes ***

// *** Crystal ***

#include "art/crystal/application-exit-16.h"
#include "art/crystal/application-exit-22.h"
#include "art/crystal/application-info-16.h"
#include "art/crystal/application-info-22.h"
#include "art/crystal/application-options-16.h"
#include "art/crystal/application-options-22.h"
#include "art/crystal/document-close-16.h"
#include "art/crystal/document-close-22.h"
#include "art/crystal/document-delete-16.h"
#include "art/crystal/document-delete-22.h"
#include "art/crystal/document-export-16.h"
#include "art/crystal/document-export-22.h"
#include "art/crystal/document-import-16.h"
#include "art/crystal/document-import-22.h"
#include "art/crystal/document-new-16.h"
#include "art/crystal/document-new-22.h"
#include "art/crystal/document-open-16.h"
#include "art/crystal/document-open-22.h"
#include "art/crystal/document-password-16.h"
#include "art/crystal/document-password-22.h"
#include "art/crystal/document-properties-16.h"
#include "art/crystal/document-properties-22.h"
#include "art/crystal/document-revert-16.h"
#include "art/crystal/document-revert-22.h"
#include "art/crystal/document-save-16.h"
#include "art/crystal/document-save-22.h"
#include "art/crystal/document-save-as-16.h"
#include "art/crystal/document-save-as-22.h"
#include "art/crystal/edit-add-16.h"
#include "art/crystal/edit-add-22.h"
#include "art/crystal/edit-clear-16.h"
#include "art/crystal/edit-clear-22.h"
#include "art/crystal/edit-copy-16.h"
#include "art/crystal/edit-copy-22.h"
#include "art/crystal/edit-cut-16.h"
#include "art/crystal/edit-cut-22.h"
#include "art/crystal/edit-datetime-16.h"
#include "art/crystal/edit-datetime-22.h"
#include "art/crystal/edit-find-16.h"
#include "art/crystal/edit-find-22.h"
#include "art/crystal/edit-goto-16.h"
#include "art/crystal/edit-goto-22.h"
#include "art/crystal/edit-paste-16.h"
#include "art/crystal/edit-paste-22.h"
#include "art/crystal/edit-redo-16.h"
#include "art/crystal/edit-redo-22.h"
#include "art/crystal/edit-remove-16.h"
#include "art/crystal/edit-remove-22.h"
#include "art/crystal/edit-undo-16.h"
#include "art/crystal/edit-undo-22.h"
#include "art/crystal/file-binary-16.h"
#include "art/crystal/file-binary-32.h"
#include "art/crystal/file-image-16.h"
#include "art/crystal/file-image-32.h"
#include "art/crystal/file-text-16.h"
#include "art/crystal/file-text-32.h"
#include "art/crystal/go-down-16.h"
#include "art/crystal/go-next-16.h"
#include "art/crystal/go-up-16.h"
#include "art/crystal/messagebox-error-32.h"
#include "art/crystal/messagebox-info-32.h"
#include "art/crystal/messagebox-warning-32.h"
#include "art/crystal/snapshot.h"
#include "art/crystal/userkeyslot.h"
#include "art/crystal/userkeyslot-active.h"
#include "art/crystal/view-choose-16.h"
#include "art/crystal/view-choose-22.h"
#include "art/crystal/view-detailed-16.h"
#include "art/crystal/view-detailed-22.h"
#include "art/crystal/view-icon-16.h"
#include "art/crystal/view-icon-22.h"
#include "art/crystal/view-multicolumn-16.h"
#include "art/crystal/view-multicolumn-22.h"
#include "art/crystal/view-zoom.h"
#include "art/crystal/view-zoom-increase.h"
#include "art/crystal/view-zoom-decrease.h"
#include "art/crystal/view-zoom-reset.h"
#include "art/crystal/window-close-16.h"

#define BUILTIN(x)      x, sizeof(x)

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_crystal_large[] =
{
    // Menu Items

    { wxID_OPEN, BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { wxID_SAVE, BU_MENU, BUILTIN(crystal_document_save_16_png) },
    { wxID_SAVEAS, BU_MENU, BUILTIN(crystal_document_save_as_16_png) },
    { wxID_REVERT, BU_MENU, BUILTIN(crystal_document_revert_16_png) },
    { wxID_CLOSE, BU_MENU, BUILTIN(crystal_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_MENU, BUILTIN(crystal_view_choose_16_png) },
    { wxID_PROPERTIES, BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { wxID_PREFERENCES, BU_MENU, BUILTIN(crystal_application_options_16_png) },
    { wxID_EXIT, BU_MENU, BUILTIN(crystal_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW, BU_MENU, BUILTIN(crystal_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN, BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { myID_MENU_SUBFILE_IMPORT, BU_MENU, BUILTIN(crystal_document_import_16_png) },
    { myID_MENU_SUBFILE_EXPORT, BU_MENU, BUILTIN(crystal_document_export_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_MENU, BUILTIN(crystal_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE, BU_MENU, BUILTIN(crystal_document_delete_16_png) },

    { wxID_UNDO, BU_MENU, BUILTIN(crystal_edit_undo_16_png) },
    { wxID_REDO, BU_MENU, BUILTIN(crystal_edit_redo_16_png) },
    { wxID_CUT, BU_MENU, BUILTIN(crystal_edit_cut_16_png) },
    { wxID_COPY, BU_MENU, BUILTIN(crystal_edit_copy_16_png) },
    { wxID_PASTE, BU_MENU, BUILTIN(crystal_edit_paste_16_png) },
    { wxID_CLEAR, BU_MENU, BUILTIN(crystal_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_FIND, BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_REPLACE, BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO, BU_MENU, BUILTIN(crystal_edit_goto_16_png) },
    { myID_MENU_EDIT_INSERT_PASSWORD, BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { myID_MENU_EDIT_INSERT_DATETIME, BU_MENU, BUILTIN(crystal_edit_datetime_16_png) },

    { wxID_ADD, BU_MENU, BUILTIN(crystal_edit_add_16_png) },
    { wxID_REMOVE, BU_MENU, BUILTIN(crystal_edit_remove_16_png) },

    { myID_MENU_VIEW_ZOOM, BU_MENU, BUILTIN(crystal_view_zoom_png) },
    { myID_MENU_VIEW_ZOOM_INCREASE, BU_MENU, BUILTIN(crystal_view_zoom_increase_png) },
    { myID_MENU_VIEW_ZOOM_DECREASE, BU_MENU, BUILTIN(crystal_view_zoom_decrease_png) },
    { myID_MENU_VIEW_ZOOM_RESET, BU_MENU, BUILTIN(crystal_view_zoom_reset_png) },

    { wxID_ABOUT, BU_MENU, BUILTIN(crystal_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS, BU_MENU, BUILTIN(crystal_view_icon_16_png) },
    { myID_MENU_VIEW_LIST, BU_MENU, BUILTIN(crystal_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT, BU_MENU, BUILTIN(crystal_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN, BU_TOOLBAR, BUILTIN(crystal_document_open_22_png) },
    { wxID_SAVE, BU_TOOLBAR, BUILTIN(crystal_document_save_22_png) },
    { wxID_SAVEAS, BU_TOOLBAR, BUILTIN(crystal_document_save_as_22_png) },
    { wxID_REVERT, BU_TOOLBAR, BUILTIN(crystal_document_revert_22_png) },
    { wxID_CLOSE, BU_TOOLBAR, BUILTIN(crystal_document_close_22_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_TOOLBAR, BUILTIN(crystal_view_choose_22_png) },
    { wxID_PROPERTIES, BU_TOOLBAR, BUILTIN(crystal_document_properties_22_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_TOOLBAR, BUILTIN(crystal_document_password_22_png) },
    { wxID_PREFERENCES, BU_TOOLBAR, BUILTIN(crystal_application_options_22_png) },
    { wxID_EXIT, BU_TOOLBAR, BUILTIN(crystal_application_exit_22_png) },

    { myID_MENU_SUBFILE_NEW, BU_TOOLBAR, BUILTIN(crystal_document_new_22_png) },
    { myID_MENU_SUBFILE_OPEN, BU_TOOLBAR, BUILTIN(crystal_document_open_22_png) },
    { myID_MENU_SUBFILE_IMPORT, BU_TOOLBAR, BUILTIN(crystal_document_import_22_png) },
    { myID_MENU_SUBFILE_EXPORT, BU_TOOLBAR, BUILTIN(crystal_document_export_22_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_TOOLBAR, BUILTIN(crystal_document_properties_22_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_TOOLBAR, BUILTIN(crystal_document_close_22_png) },
    { myID_MENU_SUBFILE_DELETE, BU_TOOLBAR, BUILTIN(crystal_document_delete_22_png) },

    { wxID_UNDO, BU_TOOLBAR, BUILTIN(crystal_edit_undo_22_png) },
    { wxID_REDO, BU_TOOLBAR, BUILTIN(crystal_edit_redo_22_png) },
    { wxID_CUT, BU_TOOLBAR, BUILTIN(crystal_edit_cut_22_png) },
    { wxID_COPY, BU_TOOLBAR, BUILTIN(crystal_edit_copy_22_png) },
    { wxID_PASTE, BU_TOOLBAR, BUILTIN(crystal_edit_paste_22_png) },
    { wxID_CLEAR, BU_TOOLBAR, BUILTIN(crystal_edit_clear_22_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_TOOLBAR, BUILTIN(crystal_edit_find_22_png) },
    { wxID_FIND, BU_TOOLBAR, BUILTIN(crystal_edit_find_22_png) },
    { wxID_REPLACE, BU_TOOLBAR, BUILTIN(crystal_edit_find_22_png) },
    { myID_MENU_EDIT_GOTO, BU_TOOLBAR, BUILTIN(crystal_edit_goto_22_png) },
    { myID_TOOL_EDIT_INSERT_PASSWORD, BU_TOOLBAR, BUILTIN(crystal_document_password_22_png) },
    { myID_TOOL_EDIT_INSERT_DATETIME, BU_TOOLBAR, BUILTIN(crystal_edit_datetime_22_png) },

    { wxID_ADD, BU_TOOLBAR, BUILTIN(crystal_edit_add_22_png) },
    { wxID_REMOVE, BU_TOOLBAR, BUILTIN(crystal_edit_remove_22_png) },

    { wxID_ABOUT, BU_TOOLBAR, BUILTIN(crystal_application_info_22_png) },

    { myID_MENU_VIEW_BIGICONS, BU_TOOLBAR, BUILTIN(crystal_view_icon_22_png) },
    { myID_MENU_VIEW_LIST, BU_TOOLBAR, BUILTIN(crystal_view_multicolumn_22_png) },
    { myID_MENU_VIEW_REPORT, BU_TOOLBAR, BUILTIN(crystal_view_detailed_22_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE, BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKFIND_NEXT, BU_GENERAL, BUILTIN(crystal_go_down_16_png) },
    { myID_QUICKFIND_PREV, BU_GENERAL, BUILTIN(crystal_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE, BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKGOTO_GO, BU_GENERAL, BUILTIN(crystal_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR, BU_GENERAL, BUILTIN(crystal_messagebox_error_32_png) },
    { wxICON_WARNING, BU_GENERAL, BUILTIN(crystal_messagebox_warning_32_png) },
    { wxICON_INFORMATION, BU_GENERAL, BUILTIN(crystal_messagebox_info_32_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY, BU_FILETYPE, BUILTIN(crystal_file_binary_16_png) },
    { myID_FILETYPE_TEXT, BU_FILETYPE, BUILTIN(crystal_file_text_16_png) },
    { myID_FILETYPE_IMAGE, BU_FILETYPE, BUILTIN(crystal_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE, BU_FILETYPE, BUILTIN(crystal_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE, BU_FILETYPE, BUILTIN(crystal_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE, BU_FILETYPE, BUILTIN(crystal_file_image_32_png) },

    // Other Images

    { myID_IMAGE_USERKEYSLOT, BU_GENERAL, BUILTIN(crystal_userkeyslot_png) },
    { myID_IMAGE_USERKEYSLOT_ACTIVE, BU_GENERAL, BUILTIN(crystal_userkeyslot_active_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_crystal_small[] =
{
    // Menu Items

    { wxID_OPEN, BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { wxID_SAVE, BU_MENU, BUILTIN(crystal_document_save_16_png) },
    { wxID_SAVEAS, BU_MENU, BUILTIN(crystal_document_save_as_16_png) },
    { wxID_REVERT, BU_MENU, BUILTIN(crystal_document_revert_16_png) },
    { wxID_CLOSE, BU_MENU, BUILTIN(crystal_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_MENU, BUILTIN(crystal_view_choose_16_png) },
    { wxID_PROPERTIES, BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { wxID_PREFERENCES, BU_MENU, BUILTIN(crystal_application_options_16_png) },
    { wxID_EXIT, BU_MENU, BUILTIN(crystal_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW, BU_MENU, BUILTIN(crystal_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN, BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { myID_MENU_SUBFILE_IMPORT, BU_MENU, BUILTIN(crystal_document_import_16_png) },
    { myID_MENU_SUBFILE_EXPORT, BU_MENU, BUILTIN(crystal_document_export_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_MENU, BUILTIN(crystal_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE, BU_MENU, BUILTIN(crystal_document_delete_16_png) },

    { wxID_UNDO, BU_MENU, BUILTIN(crystal_edit_undo_16_png) },
    { wxID_REDO, BU_MENU, BUILTIN(crystal_edit_redo_16_png) },
    { wxID_CUT, BU_MENU, BUILTIN(crystal_edit_cut_16_png) },
    { wxID_COPY, BU_MENU, BUILTIN(crystal_edit_copy_16_png) },
    { wxID_PASTE, BU_MENU, BUILTIN(crystal_edit_paste_16_png) },
    { wxID_CLEAR, BU_MENU, BUILTIN(crystal_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_FIND, BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_REPLACE, BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO, BU_MENU, BUILTIN(crystal_edit_goto_16_png) },
    { myID_MENU_EDIT_INSERT_PASSWORD, BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { myID_MENU_EDIT_INSERT_DATETIME, BU_MENU, BUILTIN(crystal_edit_datetime_16_png) },

    { wxID_ADD, BU_MENU, BUILTIN(crystal_edit_add_16_png) },
    { wxID_REMOVE, BU_MENU, BUILTIN(crystal_edit_remove_16_png) },

    { myID_MENU_VIEW_ZOOM, BU_MENU, BUILTIN(crystal_view_zoom_png) },
    { myID_MENU_VIEW_ZOOM_INCREASE, BU_MENU, BUILTIN(crystal_view_zoom_increase_png) },
    { myID_MENU_VIEW_ZOOM_DECREASE, BU_MENU, BUILTIN(crystal_view_zoom_decrease_png) },
    { myID_MENU_VIEW_ZOOM_RESET, BU_MENU, BUILTIN(crystal_view_zoom_reset_png) },

    { wxID_ABOUT, BU_MENU, BUILTIN(crystal_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS, BU_MENU, BUILTIN(crystal_view_icon_16_png) },
    { myID_MENU_VIEW_LIST, BU_MENU, BUILTIN(crystal_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT, BU_MENU, BUILTIN(crystal_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN, BU_TOOLBAR, BUILTIN(crystal_document_open_16_png) },
    { wxID_SAVE, BU_TOOLBAR, BUILTIN(crystal_document_save_16_png) },
    { wxID_SAVEAS, BU_TOOLBAR, BUILTIN(crystal_document_save_as_16_png) },
    { wxID_REVERT, BU_TOOLBAR, BUILTIN(crystal_document_revert_16_png) },
    { wxID_CLOSE, BU_TOOLBAR, BUILTIN(crystal_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_TOOLBAR, BUILTIN(crystal_view_choose_16_png) },
    { wxID_PROPERTIES, BU_TOOLBAR, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_TOOLBAR, BUILTIN(crystal_document_password_16_png) },
    { wxID_PREFERENCES, BU_TOOLBAR, BUILTIN(crystal_application_options_16_png) },
    { wxID_EXIT, BU_TOOLBAR, BUILTIN(crystal_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW, BU_TOOLBAR, BUILTIN(crystal_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN, BU_TOOLBAR, BUILTIN(crystal_document_open_16_png) },
    { myID_MENU_SUBFILE_IMPORT, BU_TOOLBAR, BUILTIN(crystal_document_import_16_png) },
    { myID_MENU_SUBFILE_EXPORT, BU_TOOLBAR, BUILTIN(crystal_document_export_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_TOOLBAR, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_TOOLBAR, BUILTIN(crystal_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE, BU_TOOLBAR, BUILTIN(crystal_document_delete_16_png) },

    { wxID_UNDO, BU_TOOLBAR, BUILTIN(crystal_edit_undo_16_png) },
    { wxID_REDO, BU_TOOLBAR, BUILTIN(crystal_edit_redo_16_png) },
    { wxID_CUT, BU_TOOLBAR, BUILTIN(crystal_edit_cut_16_png) },
    { wxID_COPY, BU_TOOLBAR, BUILTIN(crystal_edit_copy_16_png) },
    { wxID_PASTE, BU_TOOLBAR, BUILTIN(crystal_edit_paste_16_png) },
    { wxID_CLEAR, BU_TOOLBAR, BUILTIN(crystal_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_TOOLBAR, BUILTIN(crystal_edit_find_16_png) },
    { wxID_FIND, BU_TOOLBAR, BUILTIN(crystal_edit_find_16_png) },
    { wxID_REPLACE, BU_TOOLBAR, BUILTIN(crystal_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO, BU_TOOLBAR, BUILTIN(crystal_edit_goto_16_png) },
    { myID_TOOL_EDIT_INSERT_PASSWORD, BU_TOOLBAR, BUILTIN(crystal_document_password_16_png) },
    { myID_TOOL_EDIT_INSERT_DATETIME, BU_TOOLBAR, BUILTIN(crystal_edit_datetime_16_png) },

    { wxID_ADD, BU_TOOLBAR, BUILTIN(crystal_edit_add_16_png) },
    { wxID_REMOVE, BU_TOOLBAR, BUILTIN(crystal_edit_remove_16_png) },

    { wxID_ABOUT, BU_TOOLBAR, BUILTIN(crystal_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS, BU_TOOLBAR, BUILTIN(crystal_view_icon_16_png) },
    { myID_MENU_VIEW_LIST, BU_TOOLBAR, BUILTIN(crystal_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT, BU_TOOLBAR, BUILTIN(crystal_view_detailed_16_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE, BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKFIND_NEXT, BU_GENERAL, BUILTIN(crystal_go_down_16_png) },
    { myID_QUICKFIND_PREV, BU_GENERAL, BUILTIN(crystal_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE, BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKGOTO_GO, BU_GENERAL, BUILTIN(crystal_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR, BU_GENERAL, BUILTIN(crystal_messagebox_error_32_png) },
    { wxICON_WARNING, BU_GENERAL, BUILTIN(crystal_messagebox_warning_32_png) },
    { wxICON_INFORMATION, BU_GENERAL, BUILTIN(crystal_messagebox_info_32_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY, BU_FILETYPE, BUILTIN(crystal_file_binary_16_png) },
    { myID_FILETYPE_TEXT, BU_FILETYPE, BUILTIN(crystal_file_text_16_png) },
    { myID_FILETYPE_IMAGE, BU_FILETYPE, BUILTIN(crystal_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE, BU_FILETYPE, BUILTIN(crystal_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE, BU_FILETYPE, BUILTIN(crystal_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE, BU_FILETYPE, BUILTIN(crystal_file_image_32_png) },

    // Other Images

    { myID_IMAGE_USERKEYSLOT, BU_GENERAL, BUILTIN(crystal_userkeyslot_png) },
    { myID_IMAGE_USERKEYSLOT_ACTIVE, BU_GENERAL, BUILTIN(crystal_userkeyslot_active_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

// *** Slick ***

#include "art/slick/application-exit-16.h"
#include "art/slick/application-exit-22.h"
#include "art/slick/application-info-16.h"
#include "art/slick/application-info-22.h"
#include "art/slick/application-options-16.h"
#include "art/slick/application-options-22.h"
#include "art/slick/document-close-16.h"
#include "art/slick/document-close-22.h"
#include "art/slick/document-delete-16.h"
#include "art/slick/document-delete-22.h"
#include "art/slick/document-new-16.h"
#include "art/slick/document-new-22.h"
#include "art/slick/document-open-16.h"
#include "art/slick/document-open-22.h"
#include "art/slick/document-password-16.h"
#include "art/slick/document-password-22.h"
#include "art/slick/document-properties-16.h"
#include "art/slick/document-properties-22.h"
#include "art/slick/document-revert-16.h"
#include "art/slick/document-revert-22.h"
#include "art/slick/document-save-16.h"
#include "art/slick/document-save-22.h"
#include "art/slick/document-save-as-16.h"
#include "art/slick/document-save-as-22.h"
#include "art/slick/edit-add-16.h"
#include "art/slick/edit-clear-16.h"
#include "art/slick/edit-copy-16.h"
#include "art/slick/edit-copy-22.h"
#include "art/slick/edit-cut-16.h"
#include "art/slick/edit-cut-22.h"
#include "art/slick/edit-datetime-16.h"
#include "art/slick/edit-datetime-22.h"
#include "art/slick/edit-find-16.h"
#include "art/slick/edit-find-22.h"
#include "art/slick/edit-goto-16.h"
#include "art/slick/edit-goto-22.h"
#include "art/slick/edit-paste-16.h"
#include "art/slick/edit-paste-22.h"
#include "art/slick/edit-redo-16.h"
#include "art/slick/edit-redo-22.h"
#include "art/slick/edit-remove-16.h"
#include "art/slick/edit-undo-16.h"
#include "art/slick/edit-undo-22.h"
#include "art/slick/file-binary-16.h"
#include "art/slick/file-binary-32.h"
#include "art/slick/file-image-16.h"
#include "art/slick/file-image-32.h"
#include "art/slick/file-text-16.h"
#include "art/slick/file-text-32.h"
#include "art/slick/go-down-16.h"
#include "art/slick/go-next-16.h"
#include "art/slick/go-up-16.h"
#include "art/slick/messagebox-error.h"
#include "art/slick/messagebox-information.h"
#include "art/slick/messagebox-warning.h"
#include "art/slick/snapshot.h"
#include "art/slick/userkeyslot.h"
#include "art/slick/userkeyslot-active.h"
#include "art/slick/view-choose-16.h"
#include "art/slick/view-choose-22.h"
#include "art/slick/view-detailed-16.h"
#include "art/slick/view-detailed-22.h"
#include "art/slick/view-icon-16.h"
#include "art/slick/view-icon-22.h"
#include "art/slick/view-multicolumn-16.h"
#include "art/slick/view-multicolumn-22.h"
#include "art/slick/view-zoom.h"
#include "art/slick/view-zoom-increase.h"
#include "art/slick/view-zoom-decrease.h"
#include "art/slick/window-close-16.h"

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_slick_large[] =
{
    // Menu Items

    { wxID_OPEN, BU_MENU, BUILTIN(slick_document_open_16_png) },
    { wxID_SAVE, BU_MENU, BUILTIN(slick_document_save_16_png) },
    { wxID_SAVEAS, BU_MENU, BUILTIN(slick_document_save_as_16_png) },
    { wxID_REVERT, BU_MENU, BUILTIN(slick_document_revert_16_png) },
    { wxID_CLOSE, BU_MENU, BUILTIN(slick_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_MENU, BUILTIN(slick_view_choose_16_png) },
    { wxID_PROPERTIES, BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_MENU, BUILTIN(slick_document_password_16_png) },
    { wxID_PREFERENCES, BU_MENU, BUILTIN(slick_application_options_16_png) },
    { wxID_EXIT, BU_MENU, BUILTIN(slick_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW, BU_MENU, BUILTIN(slick_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN, BU_MENU, BUILTIN(slick_document_open_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_MENU, BUILTIN(slick_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE, BU_MENU, BUILTIN(slick_document_delete_16_png) },

    { wxID_UNDO, BU_MENU, BUILTIN(slick_edit_undo_16_png) },
    { wxID_REDO, BU_MENU, BUILTIN(slick_edit_redo_16_png) },
    { wxID_CUT, BU_MENU, BUILTIN(slick_edit_cut_16_png) },
    { wxID_COPY, BU_MENU, BUILTIN(slick_edit_copy_16_png) },
    { wxID_PASTE, BU_MENU, BUILTIN(slick_edit_paste_16_png) },
    { wxID_CLEAR, BU_MENU, BUILTIN(slick_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_FIND, BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_REPLACE, BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO, BU_MENU, BUILTIN(slick_edit_goto_16_png) },
    { myID_MENU_EDIT_INSERT_PASSWORD, BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { myID_MENU_EDIT_INSERT_DATETIME, BU_MENU, BUILTIN(slick_edit_datetime_16_png) },

    { wxID_ADD, BU_MENU, BUILTIN(slick_edit_add_16_png) },
    { wxID_REMOVE, BU_MENU, BUILTIN(slick_edit_remove_16_png) },

    { myID_MENU_VIEW_ZOOM, BU_MENU, BUILTIN(slick_view_zoom_png) },
    { myID_MENU_VIEW_ZOOM_INCREASE, BU_MENU, BUILTIN(slick_view_zoom_increase_png) },
    { myID_MENU_VIEW_ZOOM_DECREASE, BU_MENU, BUILTIN(slick_view_zoom_decrease_png) },

    { wxID_ABOUT, BU_MENU, BUILTIN(slick_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS, BU_MENU, BUILTIN(slick_view_icon_16_png) },
    { myID_MENU_VIEW_LIST, BU_MENU, BUILTIN(slick_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT, BU_MENU, BUILTIN(slick_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN, BU_TOOLBAR, BUILTIN(slick_document_open_22_png) },
    { wxID_SAVE, BU_TOOLBAR, BUILTIN(slick_document_save_22_png) },
    { wxID_SAVEAS, BU_TOOLBAR, BUILTIN(slick_document_save_as_22_png) },
    { wxID_REVERT, BU_TOOLBAR, BUILTIN(slick_document_revert_22_png) },
    { wxID_CLOSE, BU_TOOLBAR, BUILTIN(slick_document_close_22_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_TOOLBAR, BUILTIN(slick_view_choose_22_png) },
    { wxID_PROPERTIES, BU_TOOLBAR, BUILTIN(slick_document_properties_22_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_TOOLBAR, BUILTIN(slick_document_password_22_png) },
    { wxID_PREFERENCES, BU_TOOLBAR, BUILTIN(slick_application_options_22_png) },
    { wxID_EXIT, BU_TOOLBAR, BUILTIN(slick_application_exit_22_png) },

    { myID_MENU_SUBFILE_NEW, BU_TOOLBAR, BUILTIN(slick_document_new_22_png) },
    { myID_MENU_SUBFILE_OPEN, BU_TOOLBAR, BUILTIN(slick_document_open_22_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_TOOLBAR, BUILTIN(slick_document_properties_22_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_TOOLBAR, BUILTIN(slick_document_close_22_png) },
    { myID_MENU_SUBFILE_DELETE, BU_TOOLBAR, BUILTIN(slick_document_delete_22_png) },

    { wxID_UNDO, BU_TOOLBAR, BUILTIN(slick_edit_undo_22_png) },
    { wxID_REDO, BU_TOOLBAR, BUILTIN(slick_edit_redo_22_png) },
    { wxID_CUT, BU_TOOLBAR, BUILTIN(slick_edit_cut_22_png) },
    { wxID_COPY, BU_TOOLBAR, BUILTIN(slick_edit_copy_22_png) },
    { wxID_PASTE, BU_TOOLBAR, BUILTIN(slick_edit_paste_22_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_TOOLBAR, BUILTIN(slick_edit_find_22_png) },
    { wxID_FIND, BU_TOOLBAR, BUILTIN(slick_edit_find_22_png) },
    { wxID_REPLACE, BU_TOOLBAR, BUILTIN(slick_edit_find_22_png) },
    { myID_MENU_EDIT_GOTO, BU_TOOLBAR, BUILTIN(slick_edit_goto_22_png) },
    { myID_TOOL_EDIT_INSERT_PASSWORD, BU_TOOLBAR, BUILTIN(crystal_document_password_22_png) },
    { myID_TOOL_EDIT_INSERT_DATETIME, BU_TOOLBAR, BUILTIN(slick_edit_datetime_22_png) },

    { wxID_ABOUT, BU_TOOLBAR, BUILTIN(slick_application_info_22_png) },

    { myID_MENU_VIEW_BIGICONS, BU_TOOLBAR, BUILTIN(slick_view_icon_22_png) },
    { myID_MENU_VIEW_LIST, BU_TOOLBAR, BUILTIN(slick_view_multicolumn_22_png) },
    { myID_MENU_VIEW_REPORT, BU_TOOLBAR, BUILTIN(slick_view_detailed_22_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE, BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKFIND_NEXT, BU_GENERAL, BUILTIN(slick_go_down_16_png) },
    { myID_QUICKFIND_PREV, BU_GENERAL, BUILTIN(slick_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE, BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKGOTO_GO, BU_GENERAL, BUILTIN(slick_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR, BU_GENERAL, BUILTIN(slick_messagebox_error_png) },
    { wxICON_WARNING, BU_GENERAL, BUILTIN(slick_messagebox_warning_png) },
    { wxICON_INFORMATION, BU_GENERAL, BUILTIN(slick_messagebox_information_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY, BU_FILETYPE, BUILTIN(slick_file_binary_16_png) },
    { myID_FILETYPE_TEXT, BU_FILETYPE, BUILTIN(slick_file_text_16_png) },
    { myID_FILETYPE_IMAGE, BU_FILETYPE, BUILTIN(slick_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE, BU_FILETYPE, BUILTIN(slick_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE, BU_FILETYPE, BUILTIN(slick_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE, BU_FILETYPE, BUILTIN(slick_file_image_32_png) },

    // Other Images

    { myID_IMAGE_USERKEYSLOT, BU_GENERAL, BUILTIN(slick_userkeyslot_png) },
    { myID_IMAGE_USERKEYSLOT_ACTIVE, BU_GENERAL, BUILTIN(slick_userkeyslot_active_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_slick_small[] =
{
    // Menu Items

    { wxID_OPEN, BU_MENU, BUILTIN(slick_document_open_16_png) },
    { wxID_SAVE, BU_MENU, BUILTIN(slick_document_save_16_png) },
    { wxID_SAVEAS, BU_MENU, BUILTIN(slick_document_save_as_16_png) },
    { wxID_REVERT, BU_MENU, BUILTIN(slick_document_revert_16_png) },
    { wxID_CLOSE, BU_MENU, BUILTIN(slick_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_MENU, BUILTIN(slick_view_choose_16_png) },
    { wxID_PROPERTIES, BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_MENU, BUILTIN(slick_document_password_16_png) },
    { wxID_PREFERENCES, BU_MENU, BUILTIN(slick_application_options_16_png) },
    { wxID_EXIT, BU_MENU, BUILTIN(slick_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW, BU_MENU, BUILTIN(slick_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN, BU_MENU, BUILTIN(slick_document_open_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_MENU, BUILTIN(slick_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE, BU_MENU, BUILTIN(slick_document_delete_16_png) },

    { wxID_UNDO, BU_MENU, BUILTIN(slick_edit_undo_16_png) },
    { wxID_REDO, BU_MENU, BUILTIN(slick_edit_redo_16_png) },
    { wxID_CUT, BU_MENU, BUILTIN(slick_edit_cut_16_png) },
    { wxID_COPY, BU_MENU, BUILTIN(slick_edit_copy_16_png) },
    { wxID_PASTE, BU_MENU, BUILTIN(slick_edit_paste_16_png) },
    { wxID_CLEAR, BU_MENU, BUILTIN(slick_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_FIND, BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_REPLACE, BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO, BU_MENU, BUILTIN(slick_edit_goto_16_png) },
    { myID_MENU_EDIT_INSERT_PASSWORD, BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { myID_MENU_EDIT_INSERT_DATETIME, BU_MENU, BUILTIN(slick_edit_datetime_16_png) },

    { wxID_ADD, BU_MENU, BUILTIN(slick_edit_add_16_png) },
    { wxID_REMOVE, BU_MENU, BUILTIN(slick_edit_remove_16_png) },

    { myID_MENU_VIEW_ZOOM, BU_MENU, BUILTIN(slick_view_zoom_png) },
    { myID_MENU_VIEW_ZOOM_INCREASE, BU_MENU, BUILTIN(slick_view_zoom_increase_png) },
    { myID_MENU_VIEW_ZOOM_DECREASE, BU_MENU, BUILTIN(slick_view_zoom_decrease_png) },

    { wxID_ABOUT, BU_MENU, BUILTIN(slick_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS, BU_MENU, BUILTIN(slick_view_icon_16_png) },
    { myID_MENU_VIEW_LIST, BU_MENU, BUILTIN(slick_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT, BU_MENU, BUILTIN(slick_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN, BU_TOOLBAR, BUILTIN(slick_document_open_16_png) },
    { wxID_SAVE, BU_TOOLBAR, BUILTIN(slick_document_save_16_png) },
    { wxID_SAVEAS, BU_TOOLBAR, BUILTIN(slick_document_save_as_16_png) },
    { wxID_REVERT, BU_TOOLBAR, BUILTIN(slick_document_revert_16_png) },
    { wxID_PREFERENCES, BU_TOOLBAR, BUILTIN(slick_application_options_16_png) },
    { wxID_CLOSE, BU_TOOLBAR, BUILTIN(slick_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_TOOLBAR, BUILTIN(slick_view_choose_16_png) },
    { wxID_PROPERTIES, BU_TOOLBAR, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_TOOLBAR, BUILTIN(slick_document_password_16_png) },
    { wxID_EXIT, BU_TOOLBAR, BUILTIN(slick_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW, BU_TOOLBAR, BUILTIN(slick_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN, BU_TOOLBAR, BUILTIN(slick_document_open_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_TOOLBAR, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_TOOLBAR, BUILTIN(slick_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE, BU_TOOLBAR, BUILTIN(slick_document_delete_16_png) },

    { wxID_UNDO, BU_TOOLBAR, BUILTIN(slick_edit_undo_16_png) },
    { wxID_REDO, BU_TOOLBAR, BUILTIN(slick_edit_redo_16_png) },
    { wxID_CUT, BU_TOOLBAR, BUILTIN(slick_edit_cut_16_png) },
    { wxID_COPY, BU_TOOLBAR, BUILTIN(slick_edit_copy_16_png) },
    { wxID_PASTE, BU_TOOLBAR, BUILTIN(slick_edit_paste_16_png) },
    { wxID_CLEAR, BU_TOOLBAR, BUILTIN(slick_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_TOOLBAR, BUILTIN(slick_edit_find_16_png) },
    { wxID_FIND, BU_TOOLBAR, BUILTIN(slick_edit_find_16_png) },
    { wxID_REPLACE, BU_TOOLBAR, BUILTIN(slick_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO, BU_TOOLBAR, BUILTIN(slick_edit_goto_16_png) },
    { myID_TOOL_EDIT_INSERT_PASSWORD, BU_TOOLBAR, BUILTIN(crystal_document_password_16_png) },
    { myID_TOOL_EDIT_INSERT_DATETIME, BU_TOOLBAR, BUILTIN(slick_edit_datetime_16_png) },

    { wxID_ADD, BU_TOOLBAR, BUILTIN(slick_edit_add_16_png) },
    { wxID_REMOVE, BU_TOOLBAR, BUILTIN(slick_edit_remove_16_png) },

    { wxID_ABOUT, BU_TOOLBAR, BUILTIN(slick_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS, BU_TOOLBAR, BUILTIN(slick_view_icon_16_png) },
    { myID_MENU_VIEW_LIST, BU_TOOLBAR, BUILTIN(slick_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT, BU_TOOLBAR, BUILTIN(slick_view_detailed_16_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE, BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKFIND_NEXT, BU_GENERAL, BUILTIN(slick_go_down_16_png) },
    { myID_QUICKFIND_PREV, BU_GENERAL, BUILTIN(slick_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE, BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKGOTO_GO, BU_GENERAL, BUILTIN(slick_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR, BU_GENERAL, BUILTIN(slick_messagebox_error_png) },
    { wxICON_WARNING, BU_GENERAL, BUILTIN(slick_messagebox_warning_png) },
    { wxICON_INFORMATION, BU_GENERAL, BUILTIN(slick_messagebox_information_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY, BU_FILETYPE, BUILTIN(slick_file_binary_16_png) },
    { myID_FILETYPE_TEXT, BU_FILETYPE, BUILTIN(slick_file_text_16_png) },
    { myID_FILETYPE_IMAGE, BU_FILETYPE, BUILTIN(slick_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE, BU_FILETYPE, BUILTIN(slick_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE, BU_FILETYPE, BUILTIN(slick_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE, BU_FILETYPE, BUILTIN(slick_file_image_32_png) },

    // Other Images

    { myID_IMAGE_USERKEYSLOT, BU_GENERAL, BUILTIN(slick_userkeyslot_png) },
    { myID_IMAGE_USERKEYSLOT_ACTIVE, BU_GENERAL, BUILTIN(slick_userkeyslot_active_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

// *** Gnome Icons ***

#include "art/gnome/application-about-tool.h"
#include "art/gnome/application-about.h"
#include "art/gnome/application-exit-tool.h"
#include "art/gnome/application-exit.h"
#include "art/gnome/application-preferences-tool.h"
#include "art/gnome/application-preferences.h"
#include "art/gnome/container-close.h"
#include "art/gnome/container-open-tool.h"
#include "art/gnome/container-open.h"
#include "art/gnome/container-password.h"
#include "art/gnome/container-properties-tool.h"
#include "art/gnome/container-properties.h"
#include "art/gnome/container-revert-tool.h"
#include "art/gnome/container-revert.h"
#include "art/gnome/container-save-as-tool.h"
#include "art/gnome/container-save-as.h"
#include "art/gnome/container-save-tool.h"
#include "art/gnome/container-save.h"
#include "art/gnome/container-showlist-tool.h"
#include "art/gnome/container-showlist.h"
#include "art/gnome/edit-clear-tool.h"
#include "art/gnome/edit-clear.h"
#include "art/gnome/edit-copy-tool.h"
#include "art/gnome/edit-copy.h"
#include "art/gnome/edit-cut-tool.h"
#include "art/gnome/edit-cut.h"
#include "art/gnome/edit-datetime.h"
#include "art/gnome/edit-datetime-tool.h"
#include "art/gnome/edit-delete-tool.h"
#include "art/gnome/edit-delete.h"
#include "art/gnome/edit-find-replace-tool.h"
#include "art/gnome/edit-find-replace.h"
#include "art/gnome/edit-find-tool.h"
#include "art/gnome/edit-find.h"
#include "art/gnome/edit-paste-tool.h"
#include "art/gnome/edit-paste.h"
#include "art/gnome/edit-redo-tool.h"
#include "art/gnome/edit-redo.h"
#include "art/gnome/edit-select-all-tool.h"
#include "art/gnome/edit-select-all.h"
#include "art/gnome/edit-undo-tool.h"
#include "art/gnome/edit-undo.h"
#include "art/gnome/file-binary-16.h"
#include "art/gnome/file-binary-32.h"
#include "art/gnome/file-image-16.h"
#include "art/gnome/file-image-32.h"
#include "art/gnome/file-text-16.h"
#include "art/gnome/file-text-32.h"
#include "art/gnome/go-down.h"
#include "art/gnome/go-jump-tool.h"
#include "art/gnome/go-jump.h"
#include "art/gnome/go-next.h"
#include "art/gnome/go-up.h"
#include "art/gnome/list-add-tool.h"
#include "art/gnome/list-add.h"
#include "art/gnome/list-remove-tool.h"
#include "art/gnome/list-remove.h"
#include "art/gnome/messagebox-error.h"
#include "art/gnome/messagebox-information.h"
#include "art/gnome/messagebox-question.h"
#include "art/gnome/messagebox-warning.h"
#include "art/gnome/process-stop.h"
#include "art/gnome/seahorse.h"
#include "art/gnome/seahorse-tool.h"
#include "art/gnome/snapshot.h"
#include "art/gnome/subfile-close.h"
#include "art/gnome/subfile-export.h"
#include "art/gnome/subfile-import.h"
#include "art/gnome/subfile-new-tool.h"
#include "art/gnome/subfile-new.h"
#include "art/gnome/subfile-open-tool.h"
#include "art/gnome/subfile-open.h"
#include "art/gnome/subfile-properties.h"
#include "art/gnome/userkeyslot.h"
#include "art/gnome/userkeyslot-active.h"
#include "art/gnome/view-zoom.h"
#include "art/gnome/view-zoom-increase.h"
#include "art/gnome/view-zoom-decrease.h"
#include "art/gnome/view-zoom-reset.h"

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_gnome_large[] =
{
    // Menu Items

    { wxID_OPEN, BU_MENU, BUILTIN(gnome_container_open_png) },
    { wxID_SAVE, BU_MENU, BUILTIN(gnome_container_save_png) },
    { wxID_SAVEAS, BU_MENU, BUILTIN(gnome_container_save_as_png) },
    { wxID_REVERT, BU_MENU, BUILTIN(gnome_container_revert_png) },
    { wxID_CLOSE, BU_MENU, BUILTIN(gnome_container_close_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_MENU, BUILTIN(gnome_container_showlist_png) },
    { wxID_PROPERTIES, BU_MENU, BUILTIN(gnome_container_properties_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_MENU, BUILTIN(gnome_container_password_png) },
    { wxID_PREFERENCES, BU_MENU, BUILTIN(gnome_application_preferences_png) },
    { wxID_EXIT, BU_MENU, BUILTIN(gnome_application_exit_png) },

    { myID_MENU_SUBFILE_NEW, BU_MENU, BUILTIN(gnome_subfile_new_png) },
    { myID_MENU_SUBFILE_OPEN, BU_MENU, BUILTIN(gnome_subfile_open_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_MENU, BUILTIN(gnome_subfile_properties_png) },
    { myID_MENU_SUBFILE_IMPORT, BU_MENU, BUILTIN(gnome_subfile_import_png) },
    { myID_MENU_SUBFILE_EXPORT, BU_MENU, BUILTIN(gnome_subfile_export_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_MENU, BUILTIN(gnome_subfile_close_png) },
    { myID_MENU_SUBFILE_DELETE, BU_MENU, BUILTIN(gnome_edit_delete_png) },

    { wxID_UNDO, BU_MENU, BUILTIN(gnome_edit_undo_png) },
    { wxID_REDO, BU_MENU, BUILTIN(gnome_edit_redo_png) },
    { wxID_CUT, BU_MENU, BUILTIN(gnome_edit_cut_png) },
    { wxID_COPY, BU_MENU, BUILTIN(gnome_edit_copy_png) },
    { wxID_PASTE, BU_MENU, BUILTIN(gnome_edit_paste_png) },
    { wxID_CLEAR, BU_MENU, BUILTIN(gnome_edit_clear_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_MENU, BUILTIN(gnome_edit_find_png) },
    { wxID_FIND, BU_MENU, BUILTIN(gnome_edit_find_png) },
    { wxID_REPLACE, BU_MENU, BUILTIN(gnome_edit_find_replace_png) },
    { myID_MENU_EDIT_GOTO, BU_MENU, BUILTIN(gnome_go_jump_png) },
    { wxID_SELECTALL, BU_MENU, BUILTIN(gnome_edit_select_all_png) },
    { myID_MENU_EDIT_INSERT_PASSWORD, BU_MENU, BUILTIN(gnome_seahorse_png) },
    { myID_MENU_EDIT_INSERT_DATETIME, BU_MENU, BUILTIN(gnome_edit_datetime_png) },

    { wxID_ADD, BU_MENU, BUILTIN(gnome_list_add_png) },
    { wxID_REMOVE, BU_MENU, BUILTIN(gnome_list_remove_png) },

    { myID_MENU_VIEW_ZOOM, BU_MENU, BUILTIN(gnome_view_zoom_png) },
    { myID_MENU_VIEW_ZOOM_INCREASE, BU_MENU, BUILTIN(gnome_view_zoom_increase_png) },
    { myID_MENU_VIEW_ZOOM_DECREASE, BU_MENU, BUILTIN(gnome_view_zoom_decrease_png) },
    { myID_MENU_VIEW_ZOOM_RESET, BU_MENU, BUILTIN(gnome_view_zoom_reset_png) },

    { wxID_ABOUT, BU_MENU, BUILTIN(gnome_application_about_png) },

//  { myID_MENU_VIEW_BIGICONS,		BU_MENU, BUILTIN(gnome_view_icon_png) },
//  { myID_MENU_VIEW_LIST,		BU_MENU, BUILTIN(gnome_view_multicolumn_png) },
//  { myID_MENU_VIEW_REPORT,		BU_MENU, BUILTIN(gnome_view_detailed_png) },

    // Toolbar Icons

    { wxID_OPEN, BU_TOOLBAR, BUILTIN(gnome_container_open_tool_png) },
    { wxID_SAVE, BU_TOOLBAR, BUILTIN(gnome_container_save_tool_png) },
    { wxID_SAVEAS, BU_TOOLBAR, BUILTIN(gnome_container_save_as_tool_png) },
    { wxID_REVERT, BU_TOOLBAR, BUILTIN(gnome_container_revert_tool_png) },
    // { wxID_CLOSE,			BU_TOOLBAR, BUILTIN(gnome_container_close_tool_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_TOOLBAR, BUILTIN(gnome_container_showlist_tool_png) },
    { wxID_PROPERTIES, BU_TOOLBAR, BUILTIN(gnome_container_properties_tool_png) },
    // { myID_MENU_CONTAINER_PASSLIST,	BU_TOOLBAR, BUILTIN(gnome_container_password_tool_png) },
    { wxID_PREFERENCES, BU_TOOLBAR, BUILTIN(gnome_application_preferences_tool_png) },
    { wxID_EXIT, BU_TOOLBAR, BUILTIN(gnome_application_exit_tool_png) },

    { myID_MENU_SUBFILE_NEW, BU_TOOLBAR, BUILTIN(gnome_subfile_new_tool_png) },
    { myID_MENU_SUBFILE_OPEN, BU_TOOLBAR, BUILTIN(gnome_subfile_open_tool_png) },
    // { myID_MENU_SUBFILE_PROPERTIES,	BU_TOOLBAR, BUILTIN(gnome_container_properties_tool_png) },
    // { myID_MENU_SUBFILE_CLOSE,		BU_TOOLBAR, BUILTIN(gnome_container_close_tool_png) },
    { myID_MENU_SUBFILE_DELETE, BU_TOOLBAR, BUILTIN(gnome_edit_delete_tool_png) },

    { wxID_UNDO, BU_TOOLBAR, BUILTIN(gnome_edit_undo_tool_png) },
    { wxID_REDO, BU_TOOLBAR, BUILTIN(gnome_edit_redo_tool_png) },
    { wxID_CUT, BU_TOOLBAR, BUILTIN(gnome_edit_cut_tool_png) },
    { wxID_COPY, BU_TOOLBAR, BUILTIN(gnome_edit_copy_tool_png) },
    { wxID_PASTE, BU_TOOLBAR, BUILTIN(gnome_edit_paste_tool_png) },
    { wxID_CLEAR, BU_TOOLBAR, BUILTIN(gnome_edit_clear_tool_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_TOOLBAR, BUILTIN(gnome_edit_find_tool_png) },
    { wxID_FIND, BU_TOOLBAR, BUILTIN(gnome_edit_find_tool_png) },
    { wxID_REPLACE, BU_TOOLBAR, BUILTIN(gnome_edit_find_replace_tool_png) },
    { myID_MENU_EDIT_GOTO, BU_TOOLBAR, BUILTIN(gnome_go_jump_tool_png) },
    { wxID_SELECTALL, BU_TOOLBAR, BUILTIN(gnome_edit_select_all_tool_png) },
    { myID_TOOL_EDIT_INSERT_PASSWORD, BU_TOOLBAR, BUILTIN(gnome_seahorse_tool_png) },
    { myID_TOOL_EDIT_INSERT_DATETIME, BU_TOOLBAR, BUILTIN(gnome_edit_datetime_tool_png) },

    { wxID_ADD, BU_TOOLBAR, BUILTIN(gnome_list_add_tool_png) },
    { wxID_REMOVE, BU_TOOLBAR, BUILTIN(gnome_list_remove_tool_png) },

    { wxID_ABOUT, BU_TOOLBAR, BUILTIN(gnome_application_about_tool_png) },

//  { myID_MENU_VIEW_BIGICONS,		BU_TOOLBAR, BUILTIN(gnome_view_icon_tool_png) },
//  { myID_MENU_VIEW_LIST,		BU_TOOLBAR, BUILTIN(gnome_view_multicolumn_tool_png) },
//  { myID_MENU_VIEW_REPORT,		BU_TOOLBAR, BUILTIN(gnome_view_detailed_tool_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE, BU_GENERAL, BUILTIN(gnome_process_stop_png) },
    { myID_QUICKFIND_NEXT, BU_GENERAL, BUILTIN(gnome_go_down_png) },
    { myID_QUICKFIND_PREV, BU_GENERAL, BUILTIN(gnome_go_up_png) },

    { myID_QUICKGOTO_CLOSE, BU_GENERAL, BUILTIN(gnome_process_stop_png) },
    { myID_QUICKGOTO_GO, BU_GENERAL, BUILTIN(gnome_go_next_png) },

    // Message Dialogs

    { wxICON_ERROR, BU_GENERAL, BUILTIN(gnome_messagebox_error_png) },
    { wxICON_WARNING, BU_GENERAL, BUILTIN(gnome_messagebox_warning_png) },
    { wxICON_QUESTION, BU_GENERAL, BUILTIN(gnome_messagebox_question_png) },
    { wxICON_INFORMATION, BU_GENERAL, BUILTIN(gnome_messagebox_information_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY, BU_FILETYPE, BUILTIN(gnome_file_binary_16_png) },
    { myID_FILETYPE_TEXT, BU_FILETYPE, BUILTIN(gnome_file_text_16_png) },
    { myID_FILETYPE_IMAGE, BU_FILETYPE, BUILTIN(gnome_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE, BU_FILETYPE, BUILTIN(gnome_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE, BU_FILETYPE, BUILTIN(gnome_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE, BU_FILETYPE, BUILTIN(gnome_file_image_32_png) },

    // Other Images

    { myID_IMAGE_USERKEYSLOT, BU_GENERAL, BUILTIN(gnome_userkeyslot_png) },
    { myID_IMAGE_USERKEYSLOT_ACTIVE, BU_GENERAL, BUILTIN(gnome_userkeyslot_active_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_gnome_small[] =
{
    // Menu Items

    { wxID_OPEN, BU_MENU, BUILTIN(gnome_container_open_png) },
    { wxID_SAVE, BU_MENU, BUILTIN(gnome_container_save_png) },
    { wxID_SAVEAS, BU_MENU, BUILTIN(gnome_container_save_as_png) },
    { wxID_REVERT, BU_MENU, BUILTIN(gnome_container_revert_png) },
    { wxID_CLOSE, BU_MENU, BUILTIN(gnome_container_close_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_MENU, BUILTIN(gnome_container_showlist_png) },
    { wxID_PROPERTIES, BU_MENU, BUILTIN(gnome_container_properties_png) },
    { myID_MENU_CONTAINER_PASSLIST, BU_MENU, BUILTIN(gnome_container_password_png) },
    { wxID_PREFERENCES, BU_MENU, BUILTIN(gnome_application_preferences_png) },
    { wxID_EXIT, BU_MENU, BUILTIN(gnome_application_exit_png) },

    { myID_MENU_SUBFILE_NEW, BU_MENU, BUILTIN(gnome_subfile_new_png) },
    { myID_MENU_SUBFILE_OPEN, BU_MENU, BUILTIN(gnome_subfile_open_png) },
    { myID_MENU_SUBFILE_PROPERTIES, BU_MENU, BUILTIN(gnome_subfile_properties_png) },
    { myID_MENU_SUBFILE_IMPORT, BU_MENU, BUILTIN(gnome_subfile_import_png) },
    { myID_MENU_SUBFILE_EXPORT, BU_MENU, BUILTIN(gnome_subfile_export_png) },
    { myID_MENU_SUBFILE_CLOSE, BU_MENU, BUILTIN(gnome_subfile_close_png) },
    { myID_MENU_SUBFILE_DELETE, BU_MENU, BUILTIN(gnome_edit_delete_png) },

    { wxID_UNDO, BU_MENU, BUILTIN(gnome_edit_undo_png) },
    { wxID_REDO, BU_MENU, BUILTIN(gnome_edit_redo_png) },
    { wxID_CUT, BU_MENU, BUILTIN(gnome_edit_cut_png) },
    { wxID_COPY, BU_MENU, BUILTIN(gnome_edit_copy_png) },
    { wxID_PASTE, BU_MENU, BUILTIN(gnome_edit_paste_png) },
    { wxID_CLEAR, BU_MENU, BUILTIN(gnome_edit_clear_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_MENU, BUILTIN(gnome_edit_find_png) },
    { wxID_FIND, BU_MENU, BUILTIN(gnome_edit_find_png) },
    { wxID_REPLACE, BU_MENU, BUILTIN(gnome_edit_find_replace_png) },
    { myID_MENU_EDIT_GOTO, BU_MENU, BUILTIN(gnome_go_jump_png) },
    { wxID_SELECTALL, BU_MENU, BUILTIN(gnome_edit_select_all_png) },
    { myID_MENU_EDIT_INSERT_PASSWORD, BU_MENU, BUILTIN(gnome_seahorse_png) },
    { myID_MENU_EDIT_INSERT_DATETIME, BU_MENU, BUILTIN(gnome_edit_datetime_png) },

    { wxID_ADD, BU_MENU, BUILTIN(gnome_list_add_png) },
    { wxID_REMOVE, BU_MENU, BUILTIN(gnome_list_remove_png) },

    { myID_MENU_VIEW_ZOOM, BU_MENU, BUILTIN(gnome_view_zoom_png) },
    { myID_MENU_VIEW_ZOOM_INCREASE, BU_MENU, BUILTIN(gnome_view_zoom_increase_png) },
    { myID_MENU_VIEW_ZOOM_DECREASE, BU_MENU, BUILTIN(gnome_view_zoom_decrease_png) },
    { myID_MENU_VIEW_ZOOM_RESET, BU_MENU, BUILTIN(gnome_view_zoom_reset_png) },

    { wxID_ABOUT, BU_MENU, BUILTIN(gnome_application_about_png) },

//  { myID_MENU_VIEW_BIGICONS,		BU_MENU, BUILTIN(gnome_view_icon_png) },
//  { myID_MENU_VIEW_LIST,		BU_MENU, BUILTIN(gnome_view_multicolumn_png) },
//  { myID_MENU_VIEW_REPORT,		BU_MENU, BUILTIN(gnome_view_detailed_png) },

    // Toolbar Icons

    { wxID_OPEN, BU_TOOLBAR, BUILTIN(gnome_container_open_png) },
    { wxID_SAVE, BU_TOOLBAR, BUILTIN(gnome_container_save_png) },
    { wxID_SAVEAS, BU_TOOLBAR, BUILTIN(gnome_container_save_as_png) },
    { wxID_REVERT, BU_TOOLBAR, BUILTIN(gnome_container_revert_png) },
    // { wxID_CLOSE,			BU_TOOLBAR, BUILTIN(gnome_container_close_png) },

    { myID_MENU_CONTAINER_SHOWLIST, BU_TOOLBAR, BUILTIN(gnome_container_showlist_png) },
    { wxID_PROPERTIES, BU_TOOLBAR, BUILTIN(gnome_container_properties_png) },
    // { myID_MENU_CONTAINER_PASSLIST,	BU_TOOLBAR, BUILTIN(gnome_container_password_png) },
    { wxID_PREFERENCES, BU_TOOLBAR, BUILTIN(gnome_application_preferences_png) },
    { wxID_EXIT, BU_TOOLBAR, BUILTIN(gnome_application_exit_png) },

    { myID_MENU_SUBFILE_NEW, BU_TOOLBAR, BUILTIN(gnome_subfile_new_png) },
    { myID_MENU_SUBFILE_OPEN, BU_TOOLBAR, BUILTIN(gnome_subfile_open_png) },
    // { myID_MENU_SUBFILE_PROPERTIES,	BU_TOOLBAR, BUILTIN(gnome_container_properties_png) },
    // { myID_MENU_SUBFILE_CLOSE,		BU_TOOLBAR, BUILTIN(gnome_container_close_png) },
    { myID_MENU_SUBFILE_DELETE, BU_TOOLBAR, BUILTIN(gnome_edit_delete_png) },

    { wxID_UNDO, BU_TOOLBAR, BUILTIN(gnome_edit_undo_png) },
    { wxID_REDO, BU_TOOLBAR, BUILTIN(gnome_edit_redo_png) },
    { wxID_CUT, BU_TOOLBAR, BUILTIN(gnome_edit_cut_png) },
    { wxID_COPY, BU_TOOLBAR, BUILTIN(gnome_edit_copy_png) },
    { wxID_PASTE, BU_TOOLBAR, BUILTIN(gnome_edit_paste_png) },
    { wxID_CLEAR, BU_TOOLBAR, BUILTIN(gnome_edit_clear_png) },
    { myID_MENU_EDIT_QUICKFIND, BU_TOOLBAR, BUILTIN(gnome_edit_find_png) },
    { wxID_FIND, BU_TOOLBAR, BUILTIN(gnome_edit_find_png) },
    { wxID_REPLACE, BU_TOOLBAR, BUILTIN(gnome_edit_find_replace_png) },
    { myID_MENU_EDIT_GOTO, BU_TOOLBAR, BUILTIN(gnome_go_jump_png) },
    { wxID_SELECTALL, BU_TOOLBAR, BUILTIN(gnome_edit_select_all_png) },
    { myID_TOOL_EDIT_INSERT_PASSWORD, BU_TOOLBAR, BUILTIN(gnome_seahorse_png) },
    { myID_TOOL_EDIT_INSERT_DATETIME, BU_TOOLBAR, BUILTIN(gnome_edit_datetime_png) },

    { wxID_ADD, BU_TOOLBAR, BUILTIN(gnome_list_add_png) },
    { wxID_REMOVE, BU_TOOLBAR, BUILTIN(gnome_list_remove_png) },

    { wxID_ABOUT, BU_TOOLBAR, BUILTIN(gnome_application_about_png) },

//  { myID_MENU_VIEW_BIGICONS,		BU_TOOLBAR, BUILTIN(gnome_view_icon_png) },
//  { myID_MENU_VIEW_LIST,		BU_TOOLBAR, BUILTIN(gnome_view_multicolumn_png) },
//  { myID_MENU_VIEW_REPORT,		BU_TOOLBAR, BUILTIN(gnome_view_detailed_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE, BU_GENERAL, BUILTIN(gnome_process_stop_png) },
    { myID_QUICKFIND_NEXT, BU_GENERAL, BUILTIN(gnome_go_down_png) },
    { myID_QUICKFIND_PREV, BU_GENERAL, BUILTIN(gnome_go_up_png) },

    { myID_QUICKGOTO_CLOSE, BU_GENERAL, BUILTIN(gnome_process_stop_png) },
    { myID_QUICKGOTO_GO, BU_GENERAL, BUILTIN(gnome_go_next_png) },

    // Message Dialogs

    { wxICON_ERROR, BU_GENERAL, BUILTIN(gnome_messagebox_error_png) },
    { wxICON_WARNING, BU_GENERAL, BUILTIN(gnome_messagebox_warning_png) },
    { wxICON_QUESTION, BU_GENERAL, BUILTIN(gnome_messagebox_question_png) },
    { wxICON_INFORMATION, BU_GENERAL, BUILTIN(gnome_messagebox_information_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY, BU_FILETYPE, BUILTIN(gnome_file_binary_16_png) },
    { myID_FILETYPE_TEXT, BU_FILETYPE, BUILTIN(gnome_file_text_16_png) },
    { myID_FILETYPE_IMAGE, BU_FILETYPE, BUILTIN(gnome_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE, BU_FILETYPE, BUILTIN(gnome_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE, BU_FILETYPE, BUILTIN(gnome_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE, BU_FILETYPE, BUILTIN(gnome_file_image_32_png) },

    // Other Images

    { myID_IMAGE_USERKEYSLOT, BU_GENERAL, BUILTIN(gnome_userkeyslot_png) },
    { myID_IMAGE_USERKEYSLOT_ACTIVE, BU_GENERAL, BUILTIN(gnome_userkeyslot_active_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::Theme** BitmapCatalog::GetThemeList(int& size)
{
    const static Theme theme_crystal_large = {
        _("Crystal with large toolbar icons"),
        BUILTIN(crystal_snapshot_png),
        bitmaplist_crystal_large
    };

    const static Theme theme_crystal_small = {
        _("Crystal with small toolbar icons"),
        BUILTIN(crystal_snapshot_png),
        bitmaplist_crystal_small
    };

    const static Theme theme_slick_large = {
        _("Slick with large toolbar icons"),
        BUILTIN(slick_snapshot_png),
        bitmaplist_slick_large
    };

    const static Theme theme_slick_small = {
        _("Slick with small toolbar icons"),
        BUILTIN(slick_snapshot_png),
        bitmaplist_slick_small
    };

    const static Theme theme_gnome_large = {
        _("Gnome with large toolbar icons"),
        BUILTIN(gnome_snapshot_png),
        bitmaplist_gnome_large
    };

    const static Theme theme_gnome_small = {
        _("Gnome with small toolbar icons"),
        BUILTIN(gnome_snapshot_png),
        bitmaplist_gnome_small
    };

    const static Theme* themelist[] = {
        &theme_crystal_large, &theme_crystal_small,
        &theme_slick_large, &theme_slick_small,
        &theme_gnome_large, &theme_gnome_small
    };

    size = sizeof(themelist) / sizeof(themelist[0]);
    return themelist;
}

/******************************************************************************/
