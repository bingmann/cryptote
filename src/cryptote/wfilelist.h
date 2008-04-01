// $Id$

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
