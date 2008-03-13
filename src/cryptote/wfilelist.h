// $Id$

#ifndef WFILELIST_H
#define WFILELIST_H

#include <wx/wx.h>
#include <wx/listctrl.h>

class WFileList : public wxListCtrl
{
public:

    WFileList(class WCryptoTE* wmain);

    // *** Identifiers ***

    enum ids {
	myID_FIRST = wxID_HIGHEST + 1000,
	myID_LISTCTRL,

	// Popup Menu Items
	
	myID_FILE_OPEN,
	myID_FILE_EXPORT,
	myID_FILE_DELETE,
	myID_FILE_RENAME,
	myID_FILE_PROPERTIES,

	myID_VIEW_BIGICONS,
	myID_VIEW_LIST,
	myID_VIEW_REPORT,
    };

protected:

    /// Reference to main window
    class WCryptoTE* wmain;

public:
    // *** Operations ***

    /// Reload all items from the Container's list
    void	ResetItems();

    /// Update a single subfile index
    void	UpdateItem(unsigned int sfid);

    // *** Event Handlers ***

    void	OnContextMenu(wxContextMenuEvent& event);

    void	OnItemSelected(wxListEvent& event);
    void	OnItemActivated(wxListEvent& event);

    void	OnBeginLabelEdit(wxListEvent& event);
    void	OnEndLabelEdit(wxListEvent& event);

    void	OnBeginDrag(wxListEvent& event);

    // Menu Items

    void	OnMenuFileOpen(wxCommandEvent& event);
    void	OnMenuFileExport(wxCommandEvent& event);
    void	OnMenuFileDelete(wxCommandEvent& event);
    void	OnMenuFileRename(wxCommandEvent& event);
    void	OnMenuFileProperties(wxCommandEvent& event);

    void	OnMenuView(wxCommandEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

#endif // WFILELIST_H
