// $Id$

/*
 * CryptoTE v0.5.390
 * Copyright (C) 2008-2009 Timo Bingmann
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef WFILELIST_H
#define WFILELIST_H

#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/listctrl.h>

class WFileListDropTarget : public wxFileDropTarget
{
private:
    /// Reference to main window
    class WCryptoTE* wmain;

public:

    WFileListDropTarget(class WCryptoTE* wmain);

    // Virtual Callback from wxFileDropTarget
    
    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
};

class WFileList : public wxListCtrl
{
public:

    WFileList(class WCryptoTE* wmain);

    // *** Identifiers ***

    enum ids {
	myID_FIRST = wxID_HIGHEST + 1000,
	myID_LISTCTRL,
    };

protected:

    /// Reference to main window
    class WCryptoTE* wmain;

    /// Drop Target object
    WFileListDropTarget* droptarget;

    /// Current Display Mode, 0 = Icons, 1 = List, 2 = Report
    int		displaymode;

    /// Structure holding the currently displayed columns
    struct MetaSettingsv00000001
    {
	int	show_filename;
	int	show_size;
	int	show_compressed;
	int	show_compression;
	int	show_encryption;
	int	show_mtime;
	int	show_ctime;
	int	show_author;
	int	show_subject;
    }
        __attribute__((packed));

    struct MetaSettingsv00000001 metasettings;    

public:
    // *** Operations ***

    /// Rebuild the image list of file icons
    void	BuildImageList();
    
    /// Reload all items from the Container's list
    void	ResetItems();

    /// Update the column texts of a subfile
    void	UpdateItemColumns(unsigned int sfid);

    /// Update a single subfile index
    void	UpdateItem(unsigned int sfid);

    /// Update the wxListCtrl's display mode
    void	UpdateDisplayMode(int newmode);

    /// Save Display Settings to Container Properties
    void	SaveProperties();

    /// Load Display Settings to Container Properties
    void	LoadProperties();

    // *** Event Handlers ***

    void	OnContextMenu(wxContextMenuEvent& event);

    void	OnItemActivated(wxListEvent& event);

    void	OnColumnEndDrag(wxListEvent& event);
    void	OnColumnRightClick(wxListEvent& event);

    void	OnBeginLabelEdit(wxListEvent& event);
    void	OnEndLabelEdit(wxListEvent& event);

    // Menu Items

    void	OnMenuFileOpen(wxCommandEvent& event);
    void	OnMenuFileExport(wxCommandEvent& event);
    void	OnMenuFileDelete(wxCommandEvent& event);
    void	OnMenuFileRename(wxCommandEvent& event);
    void	OnMenuFileProperties(wxCommandEvent& event);

    void	OnMenuView(wxCommandEvent& event);

    void	OnMenuShowColumn(wxCommandEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

#endif // WFILELIST_H
