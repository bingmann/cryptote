// $Id$

/*
 * CryptoTE v0.5.377
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

    /// Buffer holding the binary data
    wxMemoryBuffer	bindata;

    /// Flag if the data needs to be saved, this is the case after importing a
    /// file directly into the page.
    bool		needsave;

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

    virtual bool	DoQuickGoto(const wxString& gototext);
    virtual void        StopQuickFind();

    // *** Event Handlers ***

    void		OnListItemFocused(wxListEvent& event);

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
