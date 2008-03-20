// $Id$

#include "wfilelist.h"
#include "wcryptote.h"
#include "wfileprop.h"
#include "bmpcat.h"

#include <wx/imaglist.h>
#include <wx/wfstream.h>
#include <wx/dirdlg.h>

#include "common/tools.h"

WFileList::WFileList(class WCryptoTE* parent)
    : wxListCtrl(parent, wxID_ANY),
      wmain(parent)
{
    SetWindowStyleFlag(wxLC_ICON | wxLC_EDIT_LABELS);

    droptarget = new WFileListDropTarget(parent);
    SetDropTarget(droptarget);

    BuildImageList();
}

void WFileList::BuildImageList()
{
    {
	wxImageList* imagelist = new wxImageList(16, 16);
	imagelist->Add( BitmapCatalog::GetFileTypeBitmap(myID_FILETYPE_BINARY) );
	imagelist->Add( BitmapCatalog::GetFileTypeBitmap(myID_FILETYPE_TEXT) );
	imagelist->Add( BitmapCatalog::GetFileTypeBitmap(myID_FILETYPE_IMAGE) );

	AssignImageList(imagelist, wxIMAGE_LIST_SMALL);
    }

    {
	wxImageList* imagelist = new wxImageList(32, 32);
	imagelist->Add( BitmapCatalog::GetFileTypeBitmap(myID_FILETYPE_BINARY_LARGE) );
	imagelist->Add( BitmapCatalog::GetFileTypeBitmap(myID_FILETYPE_TEXT_LARGE) );
	imagelist->Add( BitmapCatalog::GetFileTypeBitmap(myID_FILETYPE_IMAGE_LARGE) );

	AssignImageList(imagelist, wxIMAGE_LIST_NORMAL);
    }
}

void WFileList::ResetItems()
{
    DeleteAllItems();
    if (!wmain->container) return;

    Enctain::Container &cnt = *wmain->container;

    for(unsigned int fi = 0; fi < cnt.CountSubFile(); ++fi)
    {
	unsigned long filetype;
	if ( !strSTL2WX(wmain->container->GetSubFileProperty(fi, "Filetype")).ToULong(&filetype) ) {
	    filetype = 0;
	}

	InsertItem(fi, strSTL2WX(cnt.GetSubFileProperty(fi, "Name")), filetype);
    }
}

void WFileList::UpdateItem(unsigned int sfid)
{
    if (!wmain->container) return;
    Enctain::Container &cnt = *wmain->container;

    SetItemText(sfid, strSTL2WX(cnt.GetSubFileProperty(sfid, "Name")));

    unsigned long filetype;
    if ( !strSTL2WX(wmain->container->GetSubFileProperty(sfid, "Filetype")).ToULong(&filetype) ) {
	filetype = 0;
    }

    SetItemImage(sfid, filetype);

    wmain->UpdateSubFileCaption(sfid);
}

static inline wxMenuItem* appendMenuItem(class wxMenu* parentMenu, int id,
					 const wxString& text, const wxString& helpString)
{
    wxMenuItem* mi = new wxMenuItem(parentMenu, id, text, helpString);
    mi->SetBitmap( BitmapCatalog::GetMenuBitmap(id) );
    parentMenu->Append(mi);
    return mi;
}

void WFileList::OnContextMenu(wxContextMenuEvent& WXUNUSED(event))
{
    wxMenu* menu = new wxMenu;

    appendMenuItem(menu, myID_MENU_SUBFILE_OPEN,
		   _("&Open SubFile"),
		   _("Open subfile in editor."));

    appendMenuItem(menu, myID_MENU_SUBFILE_NEW,
		   _("&Add New SubFile"),
		   _("Add new text subfile to container and open it in the editor."));

    appendMenuItem(menu, myID_MENU_SUBFILE_IMPORT,
		   _("&Import New SubFile"),
		   _("Import any file from the disk into the container."));

    appendMenuItem(menu, myID_MENU_SUBFILE_EXPORT,
		   _("&Export SubFiles"),
		   _("Export subfiles from encrypted container to disk."));

    appendMenuItem(menu, myID_MENU_SUBFILE_DELETE,
		   _("&Delete SubFiles"),
		   _("Delete selected subfiles from encrypted container."));

    menu->AppendSeparator();

    wxMenu* viewmenu = new wxMenu;

    appendMenuItem(viewmenu, myID_MENU_VIEW_BIGICONS,
		   _("Big &Icons"),
		   wxEmptyString);

    appendMenuItem(viewmenu, myID_MENU_VIEW_LIST,
		   _("&List"),
		   wxEmptyString);

    appendMenuItem(viewmenu, myID_MENU_VIEW_REPORT,
		   _("&Report"),	
		   wxEmptyString);

    menu->AppendSubMenu(viewmenu, _("&View"));

    menu->AppendSeparator();

    appendMenuItem(menu, myID_MENU_SUBFILE_RENAME,
		   _("&Rename"),
		   _("Rename selected subfile."));

    appendMenuItem(menu, myID_MENU_SUBFILE_PROPERTIES,
		   _("&Properties"),
		   _("Show metadata properties of selected subfile."));

    // disable items not applicable
    int si = GetSelectedItemCount();

    menu->Enable(myID_MENU_SUBFILE_OPEN, (si > 0));
    menu->Enable(myID_MENU_SUBFILE_EXPORT, (si > 0));
    menu->Enable(myID_MENU_SUBFILE_DELETE, (si > 0));
    menu->Enable(myID_MENU_SUBFILE_RENAME, (si == 1));
    menu->Enable(myID_MENU_SUBFILE_PROPERTIES, (si == 1));

    PopupMenu(menu);
}

void WFileList::OnItemSelected(wxListEvent& WXUNUSED(event))
{
}

void WFileList::OnItemActivated(wxListEvent& event)
{
    wmain->OpenSubFile( event.GetIndex() );
}

void WFileList::OnBeginLabelEdit(wxListEvent& WXUNUSED(event))
{
    // don't Veto()
}

void WFileList::OnEndLabelEdit(wxListEvent& event)
{
    wmain->container->SetSubFileProperty( event.GetIndex(), "Name", strWX2STL(event.GetLabel()) );
    
    wmain->UpdateSubFileCaption( event.GetIndex() );
    wmain->SetModified();
}

void WFileList::OnBeginDrag(wxListEvent& WXUNUSED(event))
{
    printf("drag event\n");
}

void WFileList::OnMenuFileOpen(wxCommandEvent& WXUNUSED(event))
{
    long item = -1;
    while(1)
    {
        item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1) break;

	wmain->OpenSubFile(item);
    }
}

void WFileList::OnMenuFileExport(wxCommandEvent& WXUNUSED(event))
{
    if (GetSelectedItemCount() == 0)
    {
	return;
    }
    else if (GetSelectedItemCount() == 1)
    {
	long sfid = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	wxString suggestname = strSTL2WX(wmain->container->GetSubFileProperty(sfid, "Name"));

	wxFileDialog dlg(this,
			 _("Save SubFile"), wxEmptyString, suggestname,
			 _("Any file (*)|*"),
			 wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (dlg.ShowModal() != wxID_OK) return;

	wxFile outfile(dlg.GetPath(), wxFile::write);
	if (!outfile.IsOpened()) return;

	{
	    wxFileOutputStream outstream(outfile);
	    wmain->ExportSubFile(sfid, outstream);
	}

	wmain->UpdateStatusBar(wxString::Format(_("Wrote %u bytes from subfile \"%s\" to %s"),
						(unsigned int)(outfile.Tell()),
						suggestname.c_str(),
						dlg.GetPath().c_str()));
    }
    else
    {
	wxString dlgtitle = wxString::Format(_("Select directory to export %u files to."), GetSelectedItemCount());
	wxDirDialog dlg(this, dlgtitle, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (dlg.ShowModal() != wxID_OK) return;

	int filesok = 0;

	long sfid = -1;
	while(1)
	{
	    sfid = GetNextItem(sfid, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	    if (sfid == -1) break;

	    wxString name = strSTL2WX(wmain->container->GetSubFileProperty(sfid, "Name"));

	    wxFileName filename(dlg.GetPath(), name);

	    if (filename.FileExists())
	    {
		wxString overstr = wxString::Format(_("The export filename \"%s\" already exists.\nDo you wish to overwrite the existing file?"), filename.GetFullPath().c_str());

		wxMessageDialog overdlg(this, overstr, _("Overwrite existing file?"),
					wxYES_NO | wxNO_DEFAULT);
		
		if (overdlg.ShowModal() == wxID_NO) continue;
	    }
	
	    wxFile outfile(filename.GetFullPath(), wxFile::write);
	    if (!outfile.IsOpened()) continue;

	    {
		wxFileOutputStream outstream(outfile);
		wmain->ExportSubFile(sfid, outstream);
	    }

	    filesok++;
	}

	wmain->UpdateStatusBar(wxString::Format(wxPLURAL("Exported %u subfile to %s",
							 "Exported %u subfiles to %s", filesok),
						filesok, dlg.GetPath().c_str()));
    }
}

void WFileList::OnMenuFileDelete(wxCommandEvent& WXUNUSED(event))
{
    std::vector<int> subfilelist;

    long sfid = -1;
    while(1)
    {
	sfid = GetNextItem(sfid, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (sfid == -1) break;

	subfilelist.push_back(sfid);
    }

    wxString surestr;

    if (subfilelist.empty()) return;
    else if (subfilelist.size() == 1)
    {
	wxString filelist = strSTL2WX( wmain->container->GetSubFileProperty(subfilelist[0], "Name") );
	surestr = wxString::Format(_("Going to permanently delete \"%s\". This cannot be undone, are you sure?"), filelist.c_str());
    }
    else {
	surestr = wxString::Format(_("Going to permanently delete %u files. This cannot be undone, are you sure?"), subfilelist.size());
    }

    wxMessageDialog suredlg(this, surestr, _("Delete files?"),
			    wxYES_NO | wxNO_DEFAULT);

    if (suredlg.ShowModal() != wxID_YES) return;

    for(std::vector<int>::reverse_iterator sfi = subfilelist.rbegin();
	sfi != subfilelist.rend(); ++sfi)
    {
	wmain->DeleteSubFile(*sfi, false);
    }
    
    ResetItems();
}

void WFileList::OnMenuFileRename(wxCommandEvent& WXUNUSED(event))
{
    long item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item == -1) return;

    EditLabel(item);
}

void WFileList::OnMenuFileProperties(wxCommandEvent& WXUNUSED(event))
{
    long item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item == -1) return;

    WFileProperties dlg(wmain, item);
    if (dlg.ShowModal() == wxID_OK)
    {
	UpdateItem(item);
	wmain->SetModified();
    }
}

void WFileList::OnMenuView(wxCommandEvent& event)
{
    switch(event.GetId())
    {
    case myID_MENU_VIEW_BIGICONS:
	SetWindowStyleFlag(wxLC_ICON | wxLC_EDIT_LABELS);
	break;

    case myID_MENU_VIEW_LIST:
	SetWindowStyleFlag(wxLC_LIST | wxLC_EDIT_LABELS);
	break;

    case myID_MENU_VIEW_REPORT:
	SetWindowStyleFlag(wxLC_REPORT | wxLC_EDIT_LABELS);
	InsertColumn(0, _("Filename"));
	break;
    }

    ResetItems();
}

WFileListDropTarget::WFileListDropTarget(class WCryptoTE* _wmain)
    : wmain(_wmain)
{
}

bool WFileListDropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString& filenames)
{
    wmain->ImportSubFiles(filenames, -1, false);

    return true;
}

BEGIN_EVENT_TABLE(WFileList, wxListCtrl)

    EVT_CONTEXT_MENU(WFileList::OnContextMenu)

    EVT_LIST_ITEM_SELECTED(wxID_ANY, WFileList::OnItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, WFileList::OnItemActivated)

    EVT_LIST_BEGIN_LABEL_EDIT(wxID_ANY, WFileList::OnBeginLabelEdit)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY, WFileList::OnEndLabelEdit)

    EVT_LIST_BEGIN_DRAG(wxID_ANY, WFileList::OnBeginDrag)
    
    // Popup Menu Items

    EVT_MENU(myID_MENU_SUBFILE_OPEN,		WFileList::OnMenuFileOpen)
    EVT_MENU(myID_MENU_SUBFILE_EXPORT,		WFileList::OnMenuFileExport)
    EVT_MENU(myID_MENU_SUBFILE_DELETE,		WFileList::OnMenuFileDelete)
    EVT_MENU(myID_MENU_SUBFILE_RENAME,		WFileList::OnMenuFileRename)
    EVT_MENU(myID_MENU_SUBFILE_PROPERTIES,	WFileList::OnMenuFileProperties)

    EVT_MENU(myID_MENU_VIEW_BIGICONS,	WFileList::OnMenuView)
    EVT_MENU(myID_MENU_VIEW_LIST,	WFileList::OnMenuView)
    EVT_MENU(myID_MENU_VIEW_REPORT,	WFileList::OnMenuView)

END_EVENT_TABLE()
