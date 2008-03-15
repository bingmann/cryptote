// $Id$

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

class BitmapCatalog* BitmapCatalog()
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

    // load built-in theme
    if (nt == 0)
    {
	AddBuiltInTheme(&theme_crystal_large);
    }
    else if (nt == 1)
    {
	AddBuiltInTheme(&theme_crystal_small);
    }
    else if (nt == 2)
    {
	AddBuiltInTheme(&theme_slick_large);
    }
    else if (nt == 3)
    {
	AddBuiltInTheme(&theme_slick_small);
    }
}

int BitmapCatalog::GetCurrentTheme()
{
    return themeid;
}

bool BitmapCatalog::GetThemeInfo(int themeid, wxString& name, wxBitmap& snapshot)
{
    switch(themeid)
    {
    case 0:
	name = theme_crystal_large.name;
	snapshot = wxBitmapFromMemory2(theme_crystal_large.snapshot_data, theme_crystal_large.snapshot_datalen);
	return true;

    case 1:
	name = theme_crystal_small.name;
	snapshot = wxBitmapFromMemory2(theme_crystal_small.snapshot_data, theme_crystal_small.snapshot_datalen);
	return true;

    case 2:
	name = theme_slick_large.name;
	snapshot = wxBitmapFromMemory2(theme_slick_large.snapshot_data, theme_slick_large.snapshot_datalen);
	return true;

    case 3:
	name = theme_slick_small.name;
	snapshot = wxBitmapFromMemory2(theme_slick_small.snapshot_data, theme_slick_small.snapshot_datalen);
	return true;
    }

    return false;
}

void BitmapCatalog::AddBuiltInTheme(const Theme* theme)
{
    assert(theme);

    for(unsigned int bi = 0; theme->entries[bi].identifier; ++bi)
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
    class BitmapCatalog&	bmpcat;

    BitmapCatalogArtProvider(class BitmapCatalog& bitmapcatalog)
	: wxArtProvider(),
	  bmpcat(bitmapcatalog)
    {
    }

    virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& WXUNUSED(size))
    {
	if (client == wxART_MESSAGE_BOX)
	{
	    if (id == wxART_ERROR)
		return bmpcat.GetBitmap(wxICON_ERROR);
	    else if (id == wxART_WARNING)
		return bmpcat.GetBitmap(wxICON_WARNING);
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
    
    { wxID_OPEN,			BU_MENU, _T("container-open"), wxNullBitmap },
    { wxID_SAVE,			BU_MENU, _T("container-save"), wxNullBitmap },
    { wxID_SAVEAS,			BU_MENU, _T("container-save-as"), wxNullBitmap },
    { wxID_REVERT,			BU_MENU, _T("container-revert"), wxNullBitmap },
    { wxID_CLOSE,			BU_MENU, _T("container-close"), wxNullBitmap },
    { myID_MENU_CONTAINER_SHOWLIST,	BU_MENU, _T("container-showsubfilelist"), wxNullBitmap },
    { wxID_PROPERTIES,			BU_MENU, _T("container-properties"), wxNullBitmap },
    { myID_MENU_CONTAINER_SETPASS,	BU_MENU, _T("container-setpassword"), wxNullBitmap },
    { wxID_PREFERENCES,			BU_MENU, _T("application-options"), wxNullBitmap },
    { wxID_EXIT,			BU_MENU, _T("application-exit"), wxNullBitmap },

    { myID_MENU_SUBFILE_NEW,		BU_MENU, _T("subfile-new"), wxNullBitmap },
    { myID_MENU_SUBFILE_OPEN,		BU_MENU, _T("subfile-open"), wxNullBitmap },
    { myID_MENU_SUBFILE_IMPORT,		BU_MENU, _T("subfile-import"), wxNullBitmap },
    { myID_MENU_SUBFILE_EXPORT,		BU_MENU, _T("subfile-export"), wxNullBitmap },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_MENU, _T("subfile-properties"), wxNullBitmap },
    { myID_MENU_SUBFILE_RENAME,		BU_MENU, _T("subfile-rename"), wxNullBitmap },
    { myID_MENU_SUBFILE_CLOSE,		BU_MENU, _T("subfile-close"), wxNullBitmap },
    { myID_MENU_SUBFILE_DELETE,		BU_MENU, _T("subfile-delete"), wxNullBitmap },

    { wxID_UNDO,			BU_MENU, _T("edit-undo"), wxNullBitmap },
    { wxID_REDO,			BU_MENU, _T("edit-redo"), wxNullBitmap },
    { wxID_CUT,				BU_MENU, _T("edit-cut"), wxNullBitmap },
    { wxID_COPY,			BU_MENU, _T("edit-copy"), wxNullBitmap },
    { wxID_PASTE,			BU_MENU, _T("edit-paste"), wxNullBitmap },
    { wxID_CLEAR,			BU_MENU, _T("edit-clear"), wxNullBitmap },
    { myID_MENU_EDIT_QUICKFIND,		BU_MENU, _T("edit-quickfind"), wxNullBitmap },
    { wxID_FIND,			BU_MENU, _T("edit-find"), wxNullBitmap },
    { wxID_REPLACE,			BU_MENU, _T("edit-replace"), wxNullBitmap },
    { myID_MENU_EDIT_GOTO,		BU_MENU, _T("edit-goto"), wxNullBitmap },
    { wxID_SELECTALL,			BU_MENU, _T("edit-select-all"), wxNullBitmap },
    { myID_MENU_EDIT_SELECTLINE,	BU_MENU, _T("edit-select-line"), wxNullBitmap },

    { wxID_ADD,				BU_MENU, _T("list-add"), wxNullBitmap },
    { wxID_REMOVE,			BU_MENU, _T("list-remove"), wxNullBitmap },

    { wxID_ABOUT,			BU_MENU, _T("application-about"), wxNullBitmap },

    { myID_MENU_VIEW_BIGICONS,		BU_MENU, _T("view-bigicons"), wxNullBitmap },
    { myID_MENU_VIEW_LIST,		BU_MENU, _T("view-list"), wxNullBitmap },
    { myID_MENU_VIEW_REPORT,		BU_MENU, _T("view-report"), wxNullBitmap },

    // Main Window Toolbar

    { wxID_OPEN,			BU_TOOLBAR, _T("container-open-tool"), wxNullBitmap },
    { wxID_SAVE,			BU_TOOLBAR, _T("container-save-tool"), wxNullBitmap },
    { wxID_SAVEAS,			BU_TOOLBAR, _T("container-save-as-tool"), wxNullBitmap },
    { wxID_REVERT,			BU_TOOLBAR, _T("container-revert-tool"), wxNullBitmap },
    { wxID_CLOSE,			BU_TOOLBAR, _T("container-close-tool"), wxNullBitmap },
    { myID_MENU_CONTAINER_SHOWLIST,	BU_TOOLBAR, _T("container-showsubfilelist-tool"), wxNullBitmap },
    { wxID_PROPERTIES,			BU_TOOLBAR, _T("container-properties-tool"), wxNullBitmap },
    { myID_MENU_CONTAINER_SETPASS,	BU_TOOLBAR, _T("container-setpassword-tool"), wxNullBitmap },
    { wxID_PREFERENCES,			BU_TOOLBAR, _T("application-options"), wxNullBitmap },
    { wxID_EXIT,			BU_TOOLBAR, _T("application-exit-tool"), wxNullBitmap },

    { myID_MENU_SUBFILE_NEW,		BU_TOOLBAR, _T("subfile-new-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_OPEN,		BU_TOOLBAR, _T("subfile-open-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_IMPORT,		BU_TOOLBAR, _T("subfile-import-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_EXPORT,		BU_TOOLBAR, _T("subfile-export-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_TOOLBAR, _T("subfile-properties-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_RENAME,		BU_TOOLBAR, _T("subfile-rename-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_CLOSE,		BU_TOOLBAR, _T("subfile-close-tool"), wxNullBitmap },
    { myID_MENU_SUBFILE_DELETE,		BU_TOOLBAR, _T("subfile-delete-tool"), wxNullBitmap },

    { wxID_UNDO,			BU_TOOLBAR, _T("edit-undo-tool"), wxNullBitmap },
    { wxID_REDO,			BU_TOOLBAR, _T("edit-redo-tool"), wxNullBitmap },
    { wxID_CUT,				BU_TOOLBAR, _T("edit-cut-tool"), wxNullBitmap },
    { wxID_COPY,			BU_TOOLBAR, _T("edit-copy-tool"), wxNullBitmap },
    { wxID_PASTE,			BU_TOOLBAR, _T("edit-paste-tool"), wxNullBitmap },
    { wxID_CLEAR,			BU_TOOLBAR, _T("edit-clear-tool"), wxNullBitmap },
    { myID_MENU_EDIT_QUICKFIND,		BU_TOOLBAR, _T("edit-quickfind-tool"), wxNullBitmap },
    { wxID_FIND,			BU_TOOLBAR, _T("edit-find-tool"), wxNullBitmap },
    { wxID_REPLACE,			BU_TOOLBAR, _T("edit-replace-tool"), wxNullBitmap },
    { myID_MENU_EDIT_GOTO,		BU_TOOLBAR, _T("edit-goto-tool"), wxNullBitmap },
    { wxID_SELECTALL,			BU_TOOLBAR, _T("edit-select-all-tool"), wxNullBitmap },
    { myID_MENU_EDIT_SELECTLINE,	BU_TOOLBAR, _T("edit-select-line-tool"), wxNullBitmap },

    { wxID_ADD,				BU_TOOLBAR, _T("list-add-tool"), wxNullBitmap },
    { wxID_REMOVE,			BU_TOOLBAR, _T("list-remove-tool"), wxNullBitmap },

    { wxID_ABOUT,			BU_TOOLBAR, _T("application-about-tool"), wxNullBitmap },

    { myID_MENU_VIEW_BIGICONS,		BU_TOOLBAR, _T("view-bigicons-tool"), wxNullBitmap },
    { myID_MENU_VIEW_LIST,		BU_TOOLBAR, _T("view-list-tool"), wxNullBitmap },
    { myID_MENU_VIEW_REPORT,		BU_TOOLBAR, _T("view-report-tool"), wxNullBitmap },

    // Other Dialogs and Panes

    { myID_QUICKFIND_CLOSE,		BU_GENERAL, _T("button-close"), wxNullBitmap },
    { myID_QUICKFIND_NEXT,		BU_GENERAL, _T("button-find-next"), wxNullBitmap },
    { myID_QUICKFIND_PREV,		BU_GENERAL, _T("button-find-previous"), wxNullBitmap },

    { myID_QUICKGOTO_CLOSE,		BU_GENERAL, _T("button-close"), wxNullBitmap },
    { myID_QUICKGOTO_GO,		BU_GENERAL, _T("button-goto"), wxNullBitmap },

    // File Icons

    { myID_FILETYPE_BINARY,		BU_FILETYPE, _T("filetype-binary"), wxNullBitmap },
    { myID_FILETYPE_TEXT,		BU_FILETYPE, _T("filetype-text"), wxNullBitmap },
    { myID_FILETYPE_IMAGE,		BU_FILETYPE, _T("filetype-image"), wxNullBitmap },

    { myID_FILETYPE_BINARY_LARGE,	BU_FILETYPE, _T("filetype-binary-large"), wxNullBitmap },
    { myID_FILETYPE_TEXT_LARGE,		BU_FILETYPE, _T("filetype-text-large"), wxNullBitmap },
    { myID_FILETYPE_IMAGE_LARGE,	BU_FILETYPE, _T("filetype-image-large"), wxNullBitmap },

    // Icons in MessageBoxes

    { wxICON_ERROR,			BU_GENERAL, _T("messagebox-error"), wxNullBitmap },
    { wxICON_WARNING,			BU_GENERAL, _T("messagebox-warning"), wxNullBitmap },
    { wxICON_INFORMATION,		BU_GENERAL, _T("messagebox-information"), wxNullBitmap },

    { 0, BU_GENERAL, wxEmptyString, wxNullBitmap }
};

wxBitmap BitmapCatalog::_GetBitmap(int id)
{
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
	if (bitmaplist[i].identifier == id && bitmaplist[i].usage == BU_GENERAL)
	    return bitmaplist[i].current;
    }

    printf("Bitmap request for unknown identifier %u general usage\n", id);

    return wxNullBitmap;
}

wxBitmap BitmapCatalog::_GetMenuBitmap(int id)
{
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
	if (bitmaplist[i].identifier == id && bitmaplist[i].usage == BU_MENU)
	    return bitmaplist[i].current;
    }

    printf("Bitmap request for unknown identifier %u menu usage\n", id);

    return _GetBitmap(id);
}

wxBitmap BitmapCatalog::_GetToolbarBitmap(int id)
{
    for (unsigned int i = 0; bitmaplist[i].identifier; ++i)
    {
	if (bitmaplist[i].identifier == id && bitmaplist[i].usage == BU_TOOLBAR)
	    return bitmaplist[i].current;
    }

    printf("Bitmap request for unknown identifier %u toolbar usage\n", id);

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

    printf("Bitmap request for unknown identifier %u filetype usage\n", id);

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
#include "art/crystal/view-choose-16.h"
#include "art/crystal/view-choose-22.h"
#include "art/crystal/view-detailed-16.h"
#include "art/crystal/view-detailed-22.h"
#include "art/crystal/view-icon-16.h"
#include "art/crystal/view-icon-22.h"
#include "art/crystal/view-multicolumn-16.h"
#include "art/crystal/view-multicolumn-22.h"
#include "art/crystal/window-close-16.h"

#define BUILTIN(x)	x, sizeof(x)

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_crystal_large[] =
{
    // Menu Items
    
    { wxID_OPEN,			BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { wxID_SAVE,			BU_MENU, BUILTIN(crystal_document_save_16_png) },
    { wxID_SAVEAS,			BU_MENU, BUILTIN(crystal_document_save_as_16_png) },
    { wxID_REVERT,			BU_MENU, BUILTIN(crystal_document_revert_16_png) },
    { wxID_CLOSE,			BU_MENU, BUILTIN(crystal_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_MENU, BUILTIN(crystal_view_choose_16_png) },
    { wxID_PROPERTIES,			BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { wxID_PREFERENCES,			BU_MENU, BUILTIN(crystal_application_options_16_png) },
    { wxID_EXIT,			BU_MENU, BUILTIN(crystal_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW,		BU_MENU, BUILTIN(crystal_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { myID_MENU_SUBFILE_IMPORT,		BU_MENU, BUILTIN(crystal_document_import_16_png) },
    { myID_MENU_SUBFILE_EXPORT,		BU_MENU, BUILTIN(crystal_document_export_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_MENU, BUILTIN(crystal_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_MENU, BUILTIN(crystal_document_delete_16_png) },

    { wxID_UNDO,			BU_MENU, BUILTIN(crystal_edit_undo_16_png) },
    { wxID_REDO,			BU_MENU, BUILTIN(crystal_edit_redo_16_png) },
    { wxID_CUT,				BU_MENU, BUILTIN(crystal_edit_cut_16_png) },
    { wxID_COPY,			BU_MENU, BUILTIN(crystal_edit_copy_16_png) },
    { wxID_PASTE,			BU_MENU, BUILTIN(crystal_edit_paste_16_png) },
    { wxID_CLEAR,			BU_MENU, BUILTIN(crystal_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_FIND,			BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_REPLACE,			BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO,		BU_MENU, BUILTIN(crystal_edit_goto_16_png) },

    { wxID_ADD,				BU_MENU, BUILTIN(crystal_edit_add_16_png) },
    { wxID_REMOVE,			BU_MENU, BUILTIN(crystal_edit_remove_16_png) },

    { wxID_ABOUT,			BU_MENU, BUILTIN(crystal_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_MENU, BUILTIN(crystal_view_icon_16_png) },
    { myID_MENU_VIEW_LIST,		BU_MENU, BUILTIN(crystal_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT,		BU_MENU, BUILTIN(crystal_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN,			BU_TOOLBAR, BUILTIN(crystal_document_open_22_png) },
    { wxID_SAVE,			BU_TOOLBAR, BUILTIN(crystal_document_save_22_png) },
    { wxID_SAVEAS,			BU_TOOLBAR, BUILTIN(crystal_document_save_as_22_png) },
    { wxID_REVERT,			BU_TOOLBAR, BUILTIN(crystal_document_revert_22_png) },
    { wxID_CLOSE,			BU_TOOLBAR, BUILTIN(crystal_document_close_22_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_TOOLBAR, BUILTIN(crystal_view_choose_22_png) },
    { wxID_PROPERTIES,			BU_TOOLBAR, BUILTIN(crystal_document_properties_22_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_TOOLBAR, BUILTIN(crystal_document_password_22_png) },
    { wxID_PREFERENCES,			BU_TOOLBAR, BUILTIN(crystal_application_options_22_png) },
    { wxID_EXIT,			BU_TOOLBAR, BUILTIN(crystal_application_exit_22_png) },

    { myID_MENU_SUBFILE_NEW,		BU_TOOLBAR, BUILTIN(crystal_document_new_22_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_TOOLBAR, BUILTIN(crystal_document_open_22_png) },
    { myID_MENU_SUBFILE_IMPORT,		BU_TOOLBAR, BUILTIN(crystal_document_import_22_png) },
    { myID_MENU_SUBFILE_EXPORT,		BU_TOOLBAR, BUILTIN(crystal_document_export_22_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_TOOLBAR, BUILTIN(crystal_document_properties_22_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_TOOLBAR, BUILTIN(crystal_document_close_22_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_TOOLBAR, BUILTIN(crystal_document_delete_22_png) },

    { wxID_UNDO,			BU_TOOLBAR, BUILTIN(crystal_edit_undo_22_png) },
    { wxID_REDO,			BU_TOOLBAR, BUILTIN(crystal_edit_redo_22_png) },
    { wxID_CUT,				BU_TOOLBAR, BUILTIN(crystal_edit_cut_22_png) },
    { wxID_COPY,			BU_TOOLBAR, BUILTIN(crystal_edit_copy_22_png) },
    { wxID_PASTE,			BU_TOOLBAR, BUILTIN(crystal_edit_paste_22_png) },
    { wxID_CLEAR,			BU_TOOLBAR, BUILTIN(crystal_edit_clear_22_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_TOOLBAR, BUILTIN(crystal_edit_find_22_png) },
    { wxID_FIND,			BU_TOOLBAR, BUILTIN(crystal_edit_find_22_png) },
    { wxID_REPLACE,			BU_TOOLBAR, BUILTIN(crystal_edit_find_22_png) },
    { myID_MENU_EDIT_GOTO,		BU_TOOLBAR, BUILTIN(crystal_edit_goto_22_png) },

    { wxID_ADD,				BU_TOOLBAR, BUILTIN(crystal_edit_add_22_png) },
    { wxID_REMOVE,			BU_TOOLBAR, BUILTIN(crystal_edit_remove_22_png) },

    { wxID_ABOUT,			BU_TOOLBAR, BUILTIN(crystal_application_info_22_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_TOOLBAR, BUILTIN(crystal_view_icon_22_png) },
    { myID_MENU_VIEW_LIST,		BU_TOOLBAR, BUILTIN(crystal_view_multicolumn_22_png) },
    { myID_MENU_VIEW_REPORT,		BU_TOOLBAR, BUILTIN(crystal_view_detailed_22_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE,		BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKFIND_NEXT,		BU_GENERAL, BUILTIN(crystal_go_down_16_png) },
    { myID_QUICKFIND_PREV,		BU_GENERAL, BUILTIN(crystal_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE,		BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKGOTO_GO,		BU_GENERAL, BUILTIN(crystal_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR,			BU_GENERAL, BUILTIN(crystal_messagebox_error_32_png) },
    { wxICON_WARNING,			BU_GENERAL, BUILTIN(crystal_messagebox_warning_32_png) },
    { wxICON_INFORMATION,		BU_GENERAL, BUILTIN(crystal_messagebox_info_32_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY,		BU_FILETYPE, BUILTIN(crystal_file_binary_16_png) },
    { myID_FILETYPE_TEXT,		BU_FILETYPE, BUILTIN(crystal_file_text_16_png) },
    { myID_FILETYPE_IMAGE,		BU_FILETYPE, BUILTIN(crystal_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE,	BU_FILETYPE, BUILTIN(crystal_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE,		BU_FILETYPE, BUILTIN(crystal_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE,	BU_FILETYPE, BUILTIN(crystal_file_image_32_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_crystal_small[] =
{
    // Menu Items
    
    { wxID_OPEN,			BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { wxID_SAVE,			BU_MENU, BUILTIN(crystal_document_save_16_png) },
    { wxID_SAVEAS,			BU_MENU, BUILTIN(crystal_document_save_as_16_png) },
    { wxID_REVERT,			BU_MENU, BUILTIN(crystal_document_revert_16_png) },
    { wxID_CLOSE,			BU_MENU, BUILTIN(crystal_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_MENU, BUILTIN(crystal_view_choose_16_png) },
    { wxID_PROPERTIES,			BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_MENU, BUILTIN(crystal_document_password_16_png) },
    { wxID_PREFERENCES,			BU_MENU, BUILTIN(crystal_application_options_16_png) },
    { wxID_EXIT,			BU_MENU, BUILTIN(crystal_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW,		BU_MENU, BUILTIN(crystal_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_MENU, BUILTIN(crystal_document_open_16_png) },
    { myID_MENU_SUBFILE_IMPORT,		BU_MENU, BUILTIN(crystal_document_import_16_png) },
    { myID_MENU_SUBFILE_EXPORT,		BU_MENU, BUILTIN(crystal_document_export_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_MENU, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_MENU, BUILTIN(crystal_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_MENU, BUILTIN(crystal_document_delete_16_png) },

    { wxID_UNDO,			BU_MENU, BUILTIN(crystal_edit_undo_16_png) },
    { wxID_REDO,			BU_MENU, BUILTIN(crystal_edit_redo_16_png) },
    { wxID_CUT,				BU_MENU, BUILTIN(crystal_edit_cut_16_png) },
    { wxID_COPY,			BU_MENU, BUILTIN(crystal_edit_copy_16_png) },
    { wxID_PASTE,			BU_MENU, BUILTIN(crystal_edit_paste_16_png) },
    { wxID_CLEAR,			BU_MENU, BUILTIN(crystal_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_FIND,			BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { wxID_REPLACE,			BU_MENU, BUILTIN(crystal_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO,		BU_MENU, BUILTIN(crystal_edit_goto_16_png) },

    { wxID_ADD,				BU_MENU, BUILTIN(crystal_edit_add_16_png) },
    { wxID_REMOVE,			BU_MENU, BUILTIN(crystal_edit_remove_16_png) },

    { wxID_ABOUT,			BU_MENU, BUILTIN(crystal_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_MENU, BUILTIN(crystal_view_icon_16_png) },
    { myID_MENU_VIEW_LIST,		BU_MENU, BUILTIN(crystal_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT,		BU_MENU, BUILTIN(crystal_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN,			BU_TOOLBAR, BUILTIN(crystal_document_open_16_png) },
    { wxID_SAVE,			BU_TOOLBAR, BUILTIN(crystal_document_save_16_png) },
    { wxID_SAVEAS,			BU_TOOLBAR, BUILTIN(crystal_document_save_as_16_png) },
    { wxID_REVERT,			BU_TOOLBAR, BUILTIN(crystal_document_revert_16_png) },
    { wxID_CLOSE,			BU_TOOLBAR, BUILTIN(crystal_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_TOOLBAR, BUILTIN(crystal_view_choose_16_png) },
    { wxID_PROPERTIES,			BU_TOOLBAR, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_TOOLBAR, BUILTIN(crystal_document_password_16_png) },
    { wxID_PREFERENCES,			BU_TOOLBAR, BUILTIN(crystal_application_options_16_png) },
    { wxID_EXIT,			BU_TOOLBAR, BUILTIN(crystal_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW,		BU_TOOLBAR, BUILTIN(crystal_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_TOOLBAR, BUILTIN(crystal_document_open_16_png) },
    { myID_MENU_SUBFILE_IMPORT,		BU_TOOLBAR, BUILTIN(crystal_document_import_16_png) },
    { myID_MENU_SUBFILE_EXPORT,		BU_TOOLBAR, BUILTIN(crystal_document_export_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_TOOLBAR, BUILTIN(crystal_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_TOOLBAR, BUILTIN(crystal_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_TOOLBAR, BUILTIN(crystal_document_delete_16_png) },

    { wxID_UNDO,			BU_TOOLBAR, BUILTIN(crystal_edit_undo_16_png) },
    { wxID_REDO,			BU_TOOLBAR, BUILTIN(crystal_edit_redo_16_png) },
    { wxID_CUT,				BU_TOOLBAR, BUILTIN(crystal_edit_cut_16_png) },
    { wxID_COPY,			BU_TOOLBAR, BUILTIN(crystal_edit_copy_16_png) },
    { wxID_PASTE,			BU_TOOLBAR, BUILTIN(crystal_edit_paste_16_png) },
    { wxID_CLEAR,			BU_TOOLBAR, BUILTIN(crystal_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_TOOLBAR, BUILTIN(crystal_edit_find_16_png) },
    { wxID_FIND,			BU_TOOLBAR, BUILTIN(crystal_edit_find_16_png) },
    { wxID_REPLACE,			BU_TOOLBAR, BUILTIN(crystal_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO,		BU_TOOLBAR, BUILTIN(crystal_edit_goto_16_png) },

    { wxID_ADD,				BU_TOOLBAR, BUILTIN(crystal_edit_add_16_png) },
    { wxID_REMOVE,			BU_TOOLBAR, BUILTIN(crystal_edit_remove_16_png) },

    { wxID_ABOUT,			BU_TOOLBAR, BUILTIN(crystal_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_TOOLBAR, BUILTIN(crystal_view_icon_16_png) },
    { myID_MENU_VIEW_LIST,		BU_TOOLBAR, BUILTIN(crystal_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT,		BU_TOOLBAR, BUILTIN(crystal_view_detailed_16_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE,		BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKFIND_NEXT,		BU_GENERAL, BUILTIN(crystal_go_down_16_png) },
    { myID_QUICKFIND_PREV,		BU_GENERAL, BUILTIN(crystal_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE,		BU_GENERAL, BUILTIN(crystal_window_close_16_png) },
    { myID_QUICKGOTO_GO,		BU_GENERAL, BUILTIN(crystal_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR,			BU_GENERAL, BUILTIN(crystal_messagebox_error_32_png) },
    { wxICON_WARNING,			BU_GENERAL, BUILTIN(crystal_messagebox_warning_32_png) },
    { wxICON_INFORMATION,		BU_GENERAL, BUILTIN(crystal_messagebox_info_32_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY,		BU_FILETYPE, BUILTIN(crystal_file_binary_16_png) },
    { myID_FILETYPE_TEXT,		BU_FILETYPE, BUILTIN(crystal_file_text_16_png) },
    { myID_FILETYPE_IMAGE,		BU_FILETYPE, BUILTIN(crystal_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE,	BU_FILETYPE, BUILTIN(crystal_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE,		BU_FILETYPE, BUILTIN(crystal_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE,	BU_FILETYPE, BUILTIN(crystal_file_image_32_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::Theme BitmapCatalog::theme_crystal_large =
{
    _("Crystal with large toolbar icons"),
    BUILTIN(crystal_snapshot_png),
    bitmaplist_crystal_large
};

const BitmapCatalog::Theme BitmapCatalog::theme_crystal_small =
{
    _("Crystal with small toolbar icons"),
    BUILTIN(crystal_snapshot_png),
    bitmaplist_crystal_small
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
#include "art/slick/messagebox-error-48.h"
#include "art/slick/messagebox-info-48.h"
#include "art/slick/messagebox-warning-48.h"
#include "art/slick/snapshot.h"
#include "art/slick/view-choose-16.h"
#include "art/slick/view-choose-22.h"
#include "art/slick/view-detailed-16.h"
#include "art/slick/view-detailed-22.h"
#include "art/slick/view-icon-16.h"
#include "art/slick/view-icon-22.h"
#include "art/slick/view-multicolumn-16.h"
#include "art/slick/view-multicolumn-22.h"
#include "art/slick/window-close-16.h"

#define BUILTIN(x)	x, sizeof(x)

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_slick_large[] =
{
    // Menu Items
    
    { wxID_OPEN,			BU_MENU, BUILTIN(slick_document_open_16_png) },
    { wxID_SAVE,			BU_MENU, BUILTIN(slick_document_save_16_png) },
    { wxID_SAVEAS,			BU_MENU, BUILTIN(slick_document_save_as_16_png) },
    { wxID_REVERT,			BU_MENU, BUILTIN(slick_document_revert_16_png) },
    { wxID_CLOSE,			BU_MENU, BUILTIN(slick_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_MENU, BUILTIN(slick_view_choose_16_png) },
    { wxID_PROPERTIES,			BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_MENU, BUILTIN(slick_document_password_16_png) },
    { wxID_PREFERENCES,			BU_MENU, BUILTIN(slick_application_options_16_png) },
    { wxID_EXIT,			BU_MENU, BUILTIN(slick_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW,		BU_MENU, BUILTIN(slick_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_MENU, BUILTIN(slick_document_open_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_MENU, BUILTIN(slick_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_MENU, BUILTIN(slick_document_delete_16_png) },

    { wxID_UNDO,			BU_MENU, BUILTIN(slick_edit_undo_16_png) },
    { wxID_REDO,			BU_MENU, BUILTIN(slick_edit_redo_16_png) },
    { wxID_CUT,				BU_MENU, BUILTIN(slick_edit_cut_16_png) },
    { wxID_COPY,			BU_MENU, BUILTIN(slick_edit_copy_16_png) },
    { wxID_PASTE,			BU_MENU, BUILTIN(slick_edit_paste_16_png) },
    { wxID_CLEAR,			BU_MENU, BUILTIN(slick_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_FIND,			BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_REPLACE,			BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO,		BU_MENU, BUILTIN(slick_edit_goto_16_png) },

    { wxID_ADD,				BU_MENU, BUILTIN(slick_edit_add_16_png) },
    { wxID_REMOVE,			BU_MENU, BUILTIN(slick_edit_remove_16_png) },

    { wxID_ABOUT,			BU_MENU, BUILTIN(slick_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_MENU, BUILTIN(slick_view_icon_16_png) },
    { myID_MENU_VIEW_LIST,		BU_MENU, BUILTIN(slick_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT,		BU_MENU, BUILTIN(slick_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN,			BU_TOOLBAR, BUILTIN(slick_document_open_22_png) },
    { wxID_SAVE,			BU_TOOLBAR, BUILTIN(slick_document_save_22_png) },
    { wxID_SAVEAS,			BU_TOOLBAR, BUILTIN(slick_document_save_as_22_png) },
    { wxID_REVERT,			BU_TOOLBAR, BUILTIN(slick_document_revert_22_png) },
    { wxID_CLOSE,			BU_TOOLBAR, BUILTIN(slick_document_close_22_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_TOOLBAR, BUILTIN(slick_view_choose_22_png) },
    { wxID_PROPERTIES,			BU_TOOLBAR, BUILTIN(slick_document_properties_22_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_TOOLBAR, BUILTIN(slick_document_password_22_png) },
    { wxID_PREFERENCES,			BU_TOOLBAR, BUILTIN(slick_application_options_22_png) },
    { wxID_EXIT,			BU_TOOLBAR, BUILTIN(slick_application_exit_22_png) },

    { myID_MENU_SUBFILE_NEW,		BU_TOOLBAR, BUILTIN(slick_document_new_22_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_TOOLBAR, BUILTIN(slick_document_open_22_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_TOOLBAR, BUILTIN(slick_document_properties_22_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_TOOLBAR, BUILTIN(slick_document_close_22_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_TOOLBAR, BUILTIN(slick_document_delete_22_png) },

    { wxID_UNDO,			BU_TOOLBAR, BUILTIN(slick_edit_undo_22_png) },
    { wxID_REDO,			BU_TOOLBAR, BUILTIN(slick_edit_redo_22_png) },
    { wxID_CUT,				BU_TOOLBAR, BUILTIN(slick_edit_cut_22_png) },
    { wxID_COPY,			BU_TOOLBAR, BUILTIN(slick_edit_copy_22_png) },
    { wxID_PASTE,			BU_TOOLBAR, BUILTIN(slick_edit_paste_22_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_TOOLBAR, BUILTIN(slick_edit_find_22_png) },
    { wxID_FIND,			BU_TOOLBAR, BUILTIN(slick_edit_find_22_png) },
    { wxID_REPLACE,			BU_TOOLBAR, BUILTIN(slick_edit_find_22_png) },
    { myID_MENU_EDIT_GOTO,		BU_TOOLBAR, BUILTIN(slick_edit_goto_22_png) },

    { wxID_ABOUT,			BU_TOOLBAR, BUILTIN(slick_application_info_22_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_TOOLBAR, BUILTIN(slick_view_icon_22_png) },
    { myID_MENU_VIEW_LIST,		BU_TOOLBAR, BUILTIN(slick_view_multicolumn_22_png) },
    { myID_MENU_VIEW_REPORT,		BU_TOOLBAR, BUILTIN(slick_view_detailed_22_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE,		BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKFIND_NEXT,		BU_GENERAL, BUILTIN(slick_go_down_16_png) },
    { myID_QUICKFIND_PREV,		BU_GENERAL, BUILTIN(slick_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE,		BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKGOTO_GO,		BU_GENERAL, BUILTIN(slick_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR,			BU_GENERAL, BUILTIN(slick_messagebox_error_48_png) },
    { wxICON_WARNING,			BU_GENERAL, BUILTIN(slick_messagebox_warning_48_png) },
    { wxICON_INFORMATION,		BU_GENERAL, BUILTIN(slick_messagebox_info_48_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY,		BU_FILETYPE, BUILTIN(slick_file_binary_16_png) },
    { myID_FILETYPE_TEXT,		BU_FILETYPE, BUILTIN(slick_file_text_16_png) },
    { myID_FILETYPE_IMAGE,		BU_FILETYPE, BUILTIN(slick_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE,	BU_FILETYPE, BUILTIN(slick_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE,		BU_FILETYPE, BUILTIN(slick_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE,	BU_FILETYPE, BUILTIN(slick_file_image_32_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::ThemeEntry BitmapCatalog::bitmaplist_slick_small[] =
{
    // Menu Items
    
    { wxID_OPEN,			BU_MENU, BUILTIN(slick_document_open_16_png) },
    { wxID_SAVE,			BU_MENU, BUILTIN(slick_document_save_16_png) },
    { wxID_SAVEAS,			BU_MENU, BUILTIN(slick_document_save_as_16_png) },
    { wxID_REVERT,			BU_MENU, BUILTIN(slick_document_revert_16_png) },
    { wxID_CLOSE,			BU_MENU, BUILTIN(slick_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_MENU, BUILTIN(slick_view_choose_16_png) },
    { wxID_PROPERTIES,			BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_MENU, BUILTIN(slick_document_password_16_png) },
    { wxID_PREFERENCES,			BU_MENU, BUILTIN(slick_application_options_16_png) },
    { wxID_EXIT,			BU_MENU, BUILTIN(slick_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW,		BU_MENU, BUILTIN(slick_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_MENU, BUILTIN(slick_document_open_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_MENU, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_MENU, BUILTIN(slick_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_MENU, BUILTIN(slick_document_delete_16_png) },

    { wxID_UNDO,			BU_MENU, BUILTIN(slick_edit_undo_16_png) },
    { wxID_REDO,			BU_MENU, BUILTIN(slick_edit_redo_16_png) },
    { wxID_CUT,				BU_MENU, BUILTIN(slick_edit_cut_16_png) },
    { wxID_COPY,			BU_MENU, BUILTIN(slick_edit_copy_16_png) },
    { wxID_PASTE,			BU_MENU, BUILTIN(slick_edit_paste_16_png) },
    { wxID_CLEAR,			BU_MENU, BUILTIN(slick_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_FIND,			BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { wxID_REPLACE,			BU_MENU, BUILTIN(slick_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO,		BU_MENU, BUILTIN(slick_edit_goto_16_png) },

    { wxID_ADD,				BU_MENU, BUILTIN(slick_edit_add_16_png) },
    { wxID_REMOVE,			BU_MENU, BUILTIN(slick_edit_remove_16_png) },

    { wxID_ABOUT,			BU_MENU, BUILTIN(slick_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_MENU, BUILTIN(slick_view_icon_16_png) },
    { myID_MENU_VIEW_LIST,		BU_MENU, BUILTIN(slick_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT,		BU_MENU, BUILTIN(slick_view_detailed_16_png) },

    // Toolbar Icons

    { wxID_OPEN,			BU_TOOLBAR, BUILTIN(slick_document_open_16_png) },
    { wxID_SAVE,			BU_TOOLBAR, BUILTIN(slick_document_save_16_png) },
    { wxID_SAVEAS,			BU_TOOLBAR, BUILTIN(slick_document_save_as_16_png) },
    { wxID_REVERT,			BU_TOOLBAR, BUILTIN(slick_document_revert_16_png) },
    { wxID_PREFERENCES,			BU_TOOLBAR, BUILTIN(slick_application_options_16_png) },
    { wxID_CLOSE,			BU_TOOLBAR, BUILTIN(slick_document_close_16_png) },

    { myID_MENU_CONTAINER_SHOWLIST,	BU_TOOLBAR, BUILTIN(slick_view_choose_16_png) },
    { wxID_PROPERTIES,			BU_TOOLBAR, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_CONTAINER_SETPASS,	BU_TOOLBAR, BUILTIN(slick_document_password_16_png) },
    { wxID_EXIT,			BU_TOOLBAR, BUILTIN(slick_application_exit_16_png) },

    { myID_MENU_SUBFILE_NEW,		BU_TOOLBAR, BUILTIN(slick_document_new_16_png) },
    { myID_MENU_SUBFILE_OPEN,		BU_TOOLBAR, BUILTIN(slick_document_open_16_png) },
    { myID_MENU_SUBFILE_PROPERTIES,	BU_TOOLBAR, BUILTIN(slick_document_properties_16_png) },
    { myID_MENU_SUBFILE_CLOSE,		BU_TOOLBAR, BUILTIN(slick_document_close_16_png) },
    { myID_MENU_SUBFILE_DELETE,		BU_TOOLBAR, BUILTIN(slick_document_delete_16_png) },

    { wxID_UNDO,			BU_TOOLBAR, BUILTIN(slick_edit_undo_16_png) },
    { wxID_REDO,			BU_TOOLBAR, BUILTIN(slick_edit_redo_16_png) },
    { wxID_CUT,				BU_TOOLBAR, BUILTIN(slick_edit_cut_16_png) },
    { wxID_COPY,			BU_TOOLBAR, BUILTIN(slick_edit_copy_16_png) },
    { wxID_PASTE,			BU_TOOLBAR, BUILTIN(slick_edit_paste_16_png) },
    { wxID_CLEAR,			BU_TOOLBAR, BUILTIN(slick_edit_clear_16_png) },
    { myID_MENU_EDIT_QUICKFIND,		BU_TOOLBAR, BUILTIN(slick_edit_find_16_png) },
    { wxID_FIND,			BU_TOOLBAR, BUILTIN(slick_edit_find_16_png) },
    { wxID_REPLACE,			BU_TOOLBAR, BUILTIN(slick_edit_find_16_png) },
    { myID_MENU_EDIT_GOTO,		BU_TOOLBAR, BUILTIN(slick_edit_goto_16_png) },

    { wxID_ADD,				BU_TOOLBAR, BUILTIN(slick_edit_add_16_png) },
    { wxID_REMOVE,			BU_TOOLBAR, BUILTIN(slick_edit_remove_16_png) },

    { wxID_ABOUT,			BU_TOOLBAR, BUILTIN(slick_application_info_16_png) },

    { myID_MENU_VIEW_BIGICONS,		BU_TOOLBAR, BUILTIN(slick_view_icon_16_png) },
    { myID_MENU_VIEW_LIST,		BU_TOOLBAR, BUILTIN(slick_view_multicolumn_16_png) },
    { myID_MENU_VIEW_REPORT,		BU_TOOLBAR, BUILTIN(slick_view_detailed_16_png) },

    // Other Dialogs

    { myID_QUICKFIND_CLOSE,		BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKFIND_NEXT,		BU_GENERAL, BUILTIN(slick_go_down_16_png) },
    { myID_QUICKFIND_PREV,		BU_GENERAL, BUILTIN(slick_go_up_16_png) },

    { myID_QUICKGOTO_CLOSE,		BU_GENERAL, BUILTIN(slick_window_close_16_png) },
    { myID_QUICKGOTO_GO,		BU_GENERAL, BUILTIN(slick_go_next_16_png) },

    // Message Dialogs

    { wxICON_ERROR,			BU_GENERAL, BUILTIN(slick_messagebox_error_48_png) },
    { wxICON_WARNING,			BU_GENERAL, BUILTIN(slick_messagebox_warning_48_png) },
    { wxICON_INFORMATION,		BU_GENERAL, BUILTIN(slick_messagebox_info_48_png) },

    // FileType Icons

    { myID_FILETYPE_BINARY,		BU_FILETYPE, BUILTIN(slick_file_binary_16_png) },
    { myID_FILETYPE_TEXT,		BU_FILETYPE, BUILTIN(slick_file_text_16_png) },
    { myID_FILETYPE_IMAGE,		BU_FILETYPE, BUILTIN(slick_file_image_16_png) },

    { myID_FILETYPE_BINARY_LARGE,	BU_FILETYPE, BUILTIN(slick_file_binary_32_png) },
    { myID_FILETYPE_TEXT_LARGE,		BU_FILETYPE, BUILTIN(slick_file_text_32_png) },
    { myID_FILETYPE_IMAGE_LARGE,	BU_FILETYPE, BUILTIN(slick_file_image_32_png) },

    { 0, BU_GENERAL, NULL, 0 }
};

const BitmapCatalog::Theme BitmapCatalog::theme_slick_large =
{
    _("Slick with large toolbar icons"),
    BUILTIN(slick_snapshot_png),
    bitmaplist_slick_large
};

const BitmapCatalog::Theme BitmapCatalog::theme_slick_small =
{
    _("Slick with small toolbar icons"),
    BUILTIN(slick_snapshot_png),
    bitmaplist_slick_small
};

