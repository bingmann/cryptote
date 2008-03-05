// $Id$

#include "wfilelist.h"
#include "wcryptote.h"
#include "wfileprop.h"

#include <wx/imaglist.h>
#include <wx/wfstream.h>
#include <wx/dirdlg.h>

#include "common/tools.h"

WFileList::WFileList(class WCryptoTE* parent)
    : wxListCtrl(parent, wxID_ANY),
      wmain(parent)
{
    SetWindowStyleFlag(wxLC_ICON | wxLC_EDIT_LABELS);

    {
        #include "art/file_binary-16.h"
        #include "art/file_text-16.h"
        #include "art/file_image-16.h"

	wxImageList* imagelist = new wxImageList(16, 16);
	imagelist->Add( wxBitmapFromMemory(file_binary_16_png) );
	imagelist->Add( wxBitmapFromMemory(file_text_16_png) );
	imagelist->Add( wxBitmapFromMemory(file_image_16_png) );

	AssignImageList(imagelist, wxIMAGE_LIST_SMALL);
    }

    {
        #include "art/file_binary-32.h"
        #include "art/file_text-32.h"
        #include "art/file_image-32.h"

	wxImageList* imagelist = new wxImageList(32, 32);
	imagelist->Add( wxBitmapFromMemory(file_binary_32_png) );
	imagelist->Add( wxBitmapFromMemory(file_text_32_png) );
	imagelist->Add( wxBitmapFromMemory(file_image_32_png) );

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
	InsertItem(fi, strSTL2WX(cnt.GetSubFileProperty(fi, "Name")), 1);
    }
}

void WFileList::UpdateItem(unsigned int sfid)
{
    if (!wmain->container) return;
    Enctain::Container &cnt = *wmain->container;

    SetItemText(sfid, strSTL2WX(cnt.GetSubFileProperty(sfid, "Name")));

    wmain->UpdateSubFileCaption(sfid);
}

void WFileList::OnItemSelected(wxListEvent& WXUNUSED(event))
{
}

void WFileList::OnItemActivated(wxListEvent& event)
{
    wmain->OpenSubFile( event.GetIndex() );
}

static inline wxMenuItem* createMenuItem(class wxMenu* parentMenu, int id,
					 const wxString& text, const wxString& helpString,
					 const wxBitmap& bmp)
{
    wxMenuItem* mi = new wxMenuItem(parentMenu, id, text, helpString);
    mi->SetBitmap(bmp);
    return mi;
}

void WFileList::OnItemRightClick(wxListEvent& WXUNUSED(event))
{
    #include "art/document_open.h"
    #include "art/document_new.h"
    #include "art/document_import.h"
    #include "art/document_export.h"
    #include "art/document_delete.h"
    #include "art/document_properties.h"

    wxMenu* menu = new wxMenu;

    menu->Append(
	createMenuItem(menu, myID_FILE_OPEN,
		       _("&Open SubFile"),
		       _("Open subfile in editor."),
		       wxBitmapFromMemory(document_open_png))
	);
    menu->Append(
	createMenuItem(menu, WCryptoTE::myID_MENU_SUBFILE_NEW,
		       _("&Add New SubFile"),
		       _("Add new text subfile to container and open it in the editor."),
		       wxBitmapFromMemory(document_new_png))
	);
    menu->Append(
	createMenuItem(menu, WCryptoTE::myID_MENU_SUBFILE_IMPORT,
		       _("&Import New SubFile"),
		       _("Import any file from the disk into the container."),
		       wxBitmapFromMemory(document_import_png))
	);
    menu->Append(
	createMenuItem(menu, myID_FILE_EXPORT,
		       _("&Export SubFiles"),
		       _("Export subfiles from encrypted container to disk."),
		       wxBitmapFromMemory(document_export_png))
	);
    menu->Append(
	createMenuItem(menu, myID_FILE_DELETE,
		       _("&Delete SubFiles"),
		       _("Delete selected subfiles from encrypted container."),
		       wxBitmapFromMemory(document_delete_png))
	);
    menu->AppendSeparator();

    #include "art/view_icon.h"
    #include "art/view_multicolumn.h"
    #include "art/view_detailed.h"

    wxMenu* viewmenu = new wxMenu;

    viewmenu->Append(
	createMenuItem(menu, myID_VIEW_BIGICONS,
		       _("Big &Icons"),
		       wxEmptyString,
		       wxBitmapFromMemory(view_icon_png))
	);
    viewmenu->Append(
	createMenuItem(menu, myID_VIEW_LIST,
		       _("&List"),
		       wxEmptyString,
		       wxBitmapFromMemory(view_multicolumn_png))
	);
    viewmenu->Append(
	createMenuItem(menu, myID_VIEW_REPORT,
		       _("&Report"),	
		       wxEmptyString,
		       wxBitmapFromMemory(view_detailed_png))
	);

    menu->AppendSubMenu(viewmenu, _("&View"));

    menu->AppendSeparator();

    menu->Append(
	createMenuItem(menu, myID_FILE_RENAME,
		       _("&Rename"),
		       _("Rename selected subfile."),
		       wxNullBitmap)
	);

    menu->Append(
	createMenuItem(menu, myID_FILE_PROPERTIES,
		       _("&Properties"),
		       _("Show metadata properties of selected subfile."),
		       wxBitmapFromMemory(document_properties_png))
	);

    // disable items not applicable
    int si = GetSelectedItemCount();

    menu->Enable(myID_FILE_OPEN, (si > 0));
    menu->Enable(myID_FILE_EXPORT, (si > 0));
    menu->Enable(myID_FILE_DELETE, (si > 0));
    menu->Enable(myID_FILE_RENAME, (si == 1));
    menu->Enable(myID_FILE_PROPERTIES, (si == 1));

    PopupMenu(menu);
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
#if wxCHECK_VERSION(2,8,0)
			 wxFD_SAVE | wxFD_OVERWRITE_PROMPT
#else
			 wxSAVE | wxOVERWRITE_PROMPT
#endif
	    );

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

	wmain->UpdateStatusBar(wxString::Format(_("Exported %u subfile(s) to %s"),
						filesok, dlg.GetPath().c_str()));
    }
}

void WFileList::OnMenuFileDelete(wxCommandEvent& event)
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

    for(std::vector<int>::const_reverse_iterator sfi = subfilelist.rbegin();
	sfi != subfilelist.rend(); ++sfi)
    {
	printf("sfid: %d\n", *sfi);
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
    case myID_VIEW_BIGICONS:
	SetWindowStyleFlag(wxLC_ICON | wxLC_EDIT_LABELS);
	break;

    case myID_VIEW_LIST:
	SetWindowStyleFlag(wxLC_LIST | wxLC_EDIT_LABELS);
	break;

    case myID_VIEW_REPORT:
	SetWindowStyleFlag(wxLC_REPORT | wxLC_EDIT_LABELS);
	InsertColumn(0, _("Filename"));
	break;
    }

    ResetItems();
}

BEGIN_EVENT_TABLE(WFileList, wxListCtrl)

    EVT_LIST_ITEM_SELECTED(wxID_ANY, WFileList::OnItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, WFileList::OnItemActivated)

    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, WFileList::OnItemRightClick)

    EVT_LIST_BEGIN_LABEL_EDIT(wxID_ANY, WFileList::OnBeginLabelEdit)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY, WFileList::OnEndLabelEdit)

    EVT_LIST_BEGIN_DRAG(wxID_ANY, WFileList::OnBeginDrag)
    
    // Popup Menu Items

    EVT_MENU(myID_FILE_OPEN,		WFileList::OnMenuFileOpen)
    EVT_MENU(myID_FILE_EXPORT,		WFileList::OnMenuFileExport)
    EVT_MENU(myID_FILE_DELETE,		WFileList::OnMenuFileDelete)
    EVT_MENU(myID_FILE_RENAME,		WFileList::OnMenuFileRename)
    EVT_MENU(myID_FILE_PROPERTIES,	WFileList::OnMenuFileProperties)

    EVT_MENU(myID_VIEW_BIGICONS,	WFileList::OnMenuView)
    EVT_MENU(myID_VIEW_LIST,		WFileList::OnMenuView)
    EVT_MENU(myID_VIEW_REPORT,		WFileList::OnMenuView)

END_EVENT_TABLE()
