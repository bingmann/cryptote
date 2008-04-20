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
    metasettings.show_filename
	= metasettings.show_size
	= metasettings.show_compressed
	= metasettings.show_compression
	= metasettings.show_encryption
	= metasettings.show_mtime
	= metasettings.show_ctime
	= metasettings.show_author
	= metasettings.show_subject = 50;

    UpdateDisplayMode(0);

    droptarget = new WFileListDropTarget(parent);
    SetDropTarget(droptarget);

    BuildImageList();

    LoadProperties();
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

void WFileList::UpdateItemColumns(unsigned int fi)
{
    Enctain::Container cnt = wmain->container;

    if (displaymode == 2)
    {
	static const wxString choiceCompression_choices[] = {
	    _("None"), _("ZLib"), _("BZ2")
	};

	static const wxString choiceEncryption_choices[] = {
	    _("None"), _("Serpent")
	};

	int col = 0;

	if (metasettings.show_filename >= 0) {
	    SetItem(fi, col++, strSTL2WX(cnt.GetSubFileProperty(fi, "Name")));
	}

	if (metasettings.show_size >= 0) {
	    SetItem(fi, col++, wxString::Format(_T("%u"), cnt.GetSubFileSize(fi)));
	}

	if (metasettings.show_compressed >= 0) {
	    SetItem(fi, col++, wxString::Format(_T("%u"), cnt.GetSubFileStorageSize(fi)));
	}

	if (metasettings.show_compression >= 0) {
	    unsigned int comp = cnt.GetSubFileCompression(fi);
	    if (comp < sizeof(choiceCompression_choices))
		SetItem(fi, col++, choiceCompression_choices[comp]);
	    else
		SetItem(fi, col++, _("<unknown>") );
	}

	if (metasettings.show_encryption >= 0) {
	    unsigned int encr = cnt.GetSubFileEncryption(fi);
	    if (encr < sizeof(choiceEncryption_choices))
		SetItem(fi, col++, choiceEncryption_choices[encr]);
	    else
		SetItem(fi, col++, _("<unknown>") );
	}

	if (metasettings.show_mtime >= 0) {
	    std::string timestr = cnt.GetSubFileProperty(fi, "MTime");
	    if (timestr.size() == sizeof(time_t)) {
		wxDateTime ctime (*(time_t*)timestr.data());
		SetItem(fi, col++, ctime.Format(_("%c")) );
	    }
	}

	if (metasettings.show_ctime >= 0) {
	    std::string timestr = cnt.GetSubFileProperty(fi, "CTime");
	    if (timestr.size() == sizeof(time_t)) {
		wxDateTime ctime (*(time_t*)timestr.data());
		SetItem(fi, col++, ctime.Format(_("%c")) );
	    }
	}

	if (metasettings.show_author >= 0) {
	    SetItem(fi, col++, strSTL2WX(cnt.GetSubFileProperty(fi, "Author")));
	}
	if (metasettings.show_subject >= 0) {
	    SetItem(fi, col++, strSTL2WX(cnt.GetSubFileProperty(fi, "Subject")));
	}
    }
    else
    {
	SetItem(fi, 0, strSTL2WX(cnt.GetSubFileProperty(fi, "Name")));
    }
}

void WFileList::ResetItems()
{
    DeleteAllItems();

    Enctain::Container cnt = wmain->container;

    for(unsigned int fi = 0; fi < cnt.CountSubFile(); ++fi)
    {
	const std::string& filetype = cnt.GetSubFileProperty(fi, "Filetype");
	int filetypeimage = 0;
	if (filetype == "text") {
	    filetypeimage = 1;
	}

	InsertItem(fi, filetypeimage);

	UpdateItemColumns(fi);
    }
}

void WFileList::UpdateItem(unsigned int sfid)
{
    Enctain::Container cnt = wmain->container;

    const std::string& filetype = cnt.GetSubFileProperty(sfid, "Filetype");
    int filetypeimage = 0;
    if (filetype == "text") {
	filetypeimage = 1;
    }

    SetItemImage(sfid, filetypeimage);

    UpdateItemColumns(sfid);

    wmain->UpdateSubFileCaption(sfid);
}

void WFileList::UpdateDisplayMode(int newmode)
{
    switch(newmode)
    {
    case 0:
	SetWindowStyleFlag(wxLC_ICON | wxLC_EDIT_LABELS);
	displaymode = 0;
	break;

    case 1:
	SetWindowStyleFlag(wxLC_LIST | wxLC_EDIT_LABELS);
	displaymode = 1;
	break;

    case 2: {
	SetWindowStyleFlag(wxLC_REPORT | wxLC_EDIT_LABELS);
	displaymode = 2;

	int col = 0;

	if (metasettings.show_filename >= 0) {
	    InsertColumn(col++, _("Filename"), wxLIST_FORMAT_LEFT, metasettings.show_filename);
	}
	if (metasettings.show_size >= 0) {
	    InsertColumn(col++, _("Size"), wxLIST_FORMAT_RIGHT, metasettings.show_size);
	}
	if (metasettings.show_compressed >= 0) {
	    InsertColumn(col++, _("Compressed"), wxLIST_FORMAT_RIGHT, metasettings.show_compressed);
	}
	if (metasettings.show_compression >= 0) {
	    InsertColumn(col++, _("Compression"), wxLIST_FORMAT_LEFT, metasettings.show_compression);
	}
	if (metasettings.show_encryption >= 0) {
	    InsertColumn(col++, _("Encryption"), wxLIST_FORMAT_LEFT, metasettings.show_encryption);
	}
	if (metasettings.show_mtime >= 0) {
	    InsertColumn(col++, _("MTime"), wxLIST_FORMAT_LEFT, metasettings.show_mtime);
	}
	if (metasettings.show_ctime >= 0) {
	    InsertColumn(col++, _("CTime"), wxLIST_FORMAT_LEFT, metasettings.show_ctime);
	}
	if (metasettings.show_author >= 0) {
	    InsertColumn(col++, _("Author"), wxLIST_FORMAT_LEFT, metasettings.show_author);
	}
	if (metasettings.show_subject >= 0) {
	    InsertColumn(col++, _("Subject"), wxLIST_FORMAT_LEFT, metasettings.show_subject);
	}
    }
	break;

    default:
	displaymode = newmode;
	break;
    }
}

void WFileList::SaveProperties()
{
    Enctain::Container cnt = wmain->container;

    cnt.SetGlobalEncryptedProperty("FileListColumns",
				   std::string((char*)&metasettings, sizeof(metasettings)));

    cnt.SetGlobalEncryptedProperty("FileListDisplayMode",
				   strWX2STL(wxString::Format(_T("%d"), displaymode)));
}

void WFileList::LoadProperties()
{
    Enctain::Container cnt = wmain->container;

    std::string strmetasettings = cnt.GetGlobalEncryptedProperty("FileListColumns"); 
    if (strmetasettings.size() == sizeof(metasettings))
    {
	memcpy(&metasettings, strmetasettings.data(), sizeof(metasettings));
    }

    wxString strdisplaymode = strSTL2WX(cnt.GetGlobalEncryptedProperty("FileListDisplayMode"));
    unsigned long newdisplaymode;
    if (strdisplaymode.ToULong(&newdisplaymode))
	UpdateDisplayMode(newdisplaymode);
    else
	UpdateDisplayMode(0);

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

void WFileList::OnItemActivated(wxListEvent& event)
{
    wmain->OpenSubFile( event.GetIndex() );
}

void WFileList::OnColumnEndDrag(wxListEvent& event)
{
    int dragcol = event.GetColumn();

    if (metasettings.show_filename >= 0) {
	if (dragcol == 0)
	    metasettings.show_filename = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_size >= 0) {
	if (dragcol == 0)
	    metasettings.show_size = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_compressed >= 0) {
	if (dragcol == 0)
	    metasettings.show_compressed = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_compression >= 0) {
	if (dragcol == 0)
	    metasettings.show_compression = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_encryption >= 0) {
	if (dragcol == 0)
	    metasettings.show_encryption = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_mtime >= 0) {
	if (dragcol == 0)
	    metasettings.show_mtime = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_ctime >= 0) {
	if (dragcol == 0)
	    metasettings.show_ctime = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_author >= 0) {
	if (dragcol == 0)
	    metasettings.show_author = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
    if (metasettings.show_subject >= 0) {
	if (dragcol == 0)
	    metasettings.show_subject = GetColumnWidth( event.GetColumn() );

	dragcol--;
    }
}

void WFileList::OnColumnRightClick(wxListEvent& WXUNUSED(event))
{
    wxMenu* menu = new wxMenu;

    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 1, _("Size"));
    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 2, _("Compressed"));
    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 3, _("Compression"));
    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 4, _("Encryption"));
    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 5, _("MTime"));
    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 6, _("CTime"));
    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 7, _("Author"));
    menu->AppendCheckItem(myID_MENU_SHOW_COLUMN0 + 8, _("Subject"));

    menu->Check(myID_MENU_SHOW_COLUMN0 + 1, (metasettings.show_size > 0));
    menu->Check(myID_MENU_SHOW_COLUMN0 + 2, (metasettings.show_compressed > 0));
    menu->Check(myID_MENU_SHOW_COLUMN0 + 3, (metasettings.show_compression > 0));
    menu->Check(myID_MENU_SHOW_COLUMN0 + 4, (metasettings.show_encryption > 0));
    menu->Check(myID_MENU_SHOW_COLUMN0 + 5, (metasettings.show_mtime > 0));
    menu->Check(myID_MENU_SHOW_COLUMN0 + 6, (metasettings.show_ctime > 0));
    menu->Check(myID_MENU_SHOW_COLUMN0 + 7, (metasettings.show_author > 0));
    menu->Check(myID_MENU_SHOW_COLUMN0 + 8, (metasettings.show_subject > 0));

    PopupMenu(menu);
}

void WFileList::OnBeginLabelEdit(wxListEvent& WXUNUSED(event))
{
    // don't Veto()
}

void WFileList::OnEndLabelEdit(wxListEvent& event)
{
    wmain->container.SetSubFileProperty( event.GetIndex(), "Name", strWX2STL(event.GetLabel()) );
    
    wmain->UpdateSubFileCaption( event.GetIndex() );
    wmain->SetModified();
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

	wxString suggestname = strSTL2WX(wmain->container.GetSubFileProperty(sfid, "Name"));

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

	    wxString name = strSTL2WX(wmain->container.GetSubFileProperty(sfid, "Name"));

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
	wxString filelist = strSTL2WX( wmain->container.GetSubFileProperty(subfilelist[0], "Name") );
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
	UpdateDisplayMode(0);
	break;

    case myID_MENU_VIEW_LIST:
	UpdateDisplayMode(1);
	break;

    case myID_MENU_VIEW_REPORT:
	UpdateDisplayMode(2);
	break;
    }

    ResetItems();
}

void WFileList::OnMenuShowColumn(wxCommandEvent& event)
{
    switch(event.GetId())
    {
    case myID_MENU_SHOW_COLUMN0 + 1:
	metasettings.show_size = -metasettings.show_size;
	break;
    case myID_MENU_SHOW_COLUMN0 + 2:
	metasettings.show_compressed = -metasettings.show_compressed;
	break;
    case myID_MENU_SHOW_COLUMN0 + 3:
	metasettings.show_compression = -metasettings.show_compression;
	break;
    case myID_MENU_SHOW_COLUMN0 + 4:
	metasettings.show_encryption = -metasettings.show_encryption;
	break;
    case myID_MENU_SHOW_COLUMN0 + 5:
	metasettings.show_mtime = -metasettings.show_mtime;
	break;
    case myID_MENU_SHOW_COLUMN0 + 6:
	metasettings.show_ctime = -metasettings.show_ctime;
	break;
    case myID_MENU_SHOW_COLUMN0 + 7:
	metasettings.show_author = -metasettings.show_author;
	break;
    case myID_MENU_SHOW_COLUMN0 + 8:
	metasettings.show_subject = -metasettings.show_subject;
	break;
    }

    UpdateDisplayMode(displaymode);
    ResetItems();
}

WFileListDropTarget::WFileListDropTarget(class WCryptoTE* _wmain)
    : wmain(_wmain)
{
}

bool WFileListDropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString& filenames)
{
    wmain->ImportSubFiles(filenames, "", false);

    return true;
}

BEGIN_EVENT_TABLE(WFileList, wxListCtrl)

    EVT_CONTEXT_MENU(WFileList::OnContextMenu)

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, WFileList::OnItemActivated)

    EVT_LIST_BEGIN_LABEL_EDIT(wxID_ANY, WFileList::OnBeginLabelEdit)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY, WFileList::OnEndLabelEdit)

    EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, WFileList::OnColumnRightClick)
    EVT_LIST_COL_END_DRAG(wxID_ANY, WFileList::OnColumnEndDrag)
    
    // Popup Menu Items

    EVT_MENU(myID_MENU_SUBFILE_OPEN,		WFileList::OnMenuFileOpen)
    EVT_MENU(myID_MENU_SUBFILE_EXPORT,		WFileList::OnMenuFileExport)
    EVT_MENU(myID_MENU_SUBFILE_DELETE,		WFileList::OnMenuFileDelete)
    EVT_MENU(myID_MENU_SUBFILE_RENAME,		WFileList::OnMenuFileRename)
    EVT_MENU(myID_MENU_SUBFILE_PROPERTIES,	WFileList::OnMenuFileProperties)

    EVT_MENU(myID_MENU_VIEW_BIGICONS,	WFileList::OnMenuView)
    EVT_MENU(myID_MENU_VIEW_LIST,	WFileList::OnMenuView)
    EVT_MENU(myID_MENU_VIEW_REPORT,	WFileList::OnMenuView)

    EVT_MENU_RANGE(myID_MENU_SHOW_COLUMN0, myID_MENU_SHOW_COLUMN0 + 20, WFileList::OnMenuShowColumn)

END_EVENT_TABLE()
