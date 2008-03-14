// $Id$

#ifndef WBINARYPAGE_H
#define WBINARYPAGE_H

#include <wx/wx.h>
#include <wx/listctrl.h>

#include "wcryptote.h"

class WBinaryPage : public WNotePage
{
public:

    WBinaryPage(class WCryptoTE* parent);

    // *** Identifiers ***

    enum {
	myID_FIRST = wxID_HIGHEST + 1,
	myID_LISTCTRL,
    };

    // *** Variables ***

    wxMemoryBuffer	bindata;

    // *** Operations ***
 
    /// Return the text to display in the notebook
    virtual wxString	GetCaption();

    /// Load a SubFile from the current container.
    bool		LoadSubFile(unsigned int sfid);

    /// Clear buffer and load all data from a file
    virtual size_t	ImportFile(wxFile& file);

    /// Write buffer to the output stream
    virtual void	ExportBuffer(wxOutputStream& outstream);

    // *** Virtual Callbacks via WNotePage ***

    /// Called when the notebook page is activated/focused.
    virtual void	PageFocused();

    /// Called when the notebook page is deactivated.
    virtual void	PageBlurred();

    /// Called when the notebook page should save it's data.
    virtual void	PageSaveData();

    /// Called when the notebook page is closed.
    virtual void	PageClosed();

    virtual void	PrepareQuickFind(bool backwards, bool reset);
    virtual void	DoQuickFind(bool backwards, const wxString& findtext);
    virtual bool	DoQuickGoto(const wxString& gototext);

    // *** Control ***

    /// a list control showing the data in hexadecimal
    class WBinaryPageList* listctrl;

private:
    DECLARE_EVENT_TABLE();

    DECLARE_ABSTRACT_CLASS(WBinaryPage);
};

class WBinaryPageList : public wxListCtrl
{
public:
    /// Reference to the page object holding the data.
    WBinaryPage&	binpage;

    WBinaryPageList(class WBinaryPage* parent, wxWindowID id);

    /// Return the string containing the text of the given column for the
    /// specified item. Callback for the Virtual ListCtrl.
    virtual wxString	OnGetItemText(long item, long column) const;

    /// Update listctrl to reflect changed data.
    void		UpdateData();
};

#endif // WBINARYPAGE_H
