///////////////////////////////////////////////////////////////////////////////
// Name:        imglbox.h
// Purpose:     wxImageListBox is a listbox whose items contain an image and
//              multi-line text. Largely based on htmlbox.h and bmpcboxg.h.
// Author:      Timo Bingmann
// Created:     2008-07-21
// RCS-ID:      $Id$
// License:     wxWindows license
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGLBOX_H_
#define _WX_IMAGLBOX_H_

#include "wx/defs.h"
#include "wx/vlbox.h"               // base class
#include "wx/ctrlsub.h"

extern const wxChar wxImageListBoxNameStr[];
extern const wxChar wxSimpleImageListBoxNameStr[];

// ----------------------------------------------------------------------------
// wxImageListBox
// ----------------------------------------------------------------------------

class wxImageListBox : public wxVListBox
{
    DECLARE_ABSTRACT_CLASS(wxImageListBox);

public:
    // constructors and such
    // ---------------------

    // default constructor, you must call Create() later
    wxImageListBox();

    // normal constructor which calls Create() internally
    wxImageListBox(wxWindow *parent,
		   wxWindowID id = wxID_ANY,
		   const wxPoint& pos = wxDefaultPosition,
		   const wxSize& size = wxDefaultSize,
		   long style = 0,
		   const wxString& name = wxImageListBoxNameStr);

    // really creates the control and sets the initial number of items in it
    // (which may be changed later with SetItemCount())
    //
    // the only special style which may be specified here is wxLB_MULTIPLE
    //
    // returns true on success or false if the control couldn't be created
    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxImageListBoxNameStr);

    // destructor cleans up whatever resources we use
    virtual ~wxImageListBox();

    // appearance
    // ---------------------

    /// Override wxWindow method to recalculate all items.
    virtual bool	SetFont(const wxFont&);

    // Set spacing parameters
    void		SetImageSpacing(wxCoord right, wxCoord left, wxCoord vertical);

    void		SetTextSpacing(wxCoord vertical);
    
protected:
    // this method must be implemented in the derived class and should return
    // the string for the given item
    virtual wxString OnGetItemString(size_t n) const = 0;

    // again implemented by derived class and returns a bitmap
    virtual wxBitmap* OnGetItemBitmap(size_t n) const = 0;


    // Implement abstract function of wxVListBox. Both of these functions work
    // in terms of OnGetItem(), OnGetItemBitmap(), they are not supposed to be
    // overridden by our descendants
    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;

    virtual wxCoord OnMeasureItem(size_t n) const;

    // common part of all ctors
    void Init();

    // Space left of image
    wxCoord m_image_spacing_right;

    // Space right of image, left of text
    wxCoord m_image_spacing_left;

    // Space top and bottom of image
    wxCoord m_image_spacing_vertical;

    // Space top and bottom of text
    wxCoord m_text_spacing_vertical;
    
private:

    DECLARE_NO_COPY_CLASS(wxImageListBox);
};


// ----------------------------------------------------------------------------
// wxSimpleImageListBox
// ----------------------------------------------------------------------------

#define wxILB_DEFAULT_STYLE     wxBORDER_SUNKEN

class wxSimpleImageListBox : public wxImageListBox,
			     public wxItemContainer
{
public:
    // wxListBox-compatible constructors
    // ---------------------------------

    wxSimpleImageListBox() { }

    wxSimpleImageListBox(wxWindow *parent,
			 wxWindowID id,
			 const wxPoint& pos = wxDefaultPosition,
			 const wxSize& size = wxDefaultSize,
			 int n = 0, const wxString choices[] = NULL,
			 long style = wxILB_DEFAULT_STYLE,
			 const wxValidator& validator = wxDefaultValidator,
			 const wxString& name = wxSimpleImageListBoxNameStr)
    {
        Create(parent, id, pos, size, n, choices, style, validator, name);
    }

    wxSimpleImageListBox(wxWindow *parent,
			 wxWindowID id,
			 const wxPoint& pos,
			 const wxSize& size,
			 const wxArrayString& choices,
			 long style = wxILB_DEFAULT_STYLE,
			 const wxValidator& validator = wxDefaultValidator,
			 const wxString& name = wxSimpleImageListBoxNameStr)
    {
        Create(parent, id, pos, size, choices, style, validator, name);
    }

    bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                int n = 0, const wxString choices[] = NULL,
                long style = wxILB_DEFAULT_STYLE,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxSimpleImageListBoxNameStr);

    bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const wxArrayString& choices,
                long style = wxILB_DEFAULT_STYLE,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxSimpleImageListBoxNameStr);

    virtual ~wxSimpleImageListBox();

    // these must be overloaded otherwise the compiler will complain
    // about wxItemContainerImmutable::[G|S]etSelection being pure virtuals...
    void SetSelection(int n)
    { wxVListBox::SetSelection(n); }

    int GetSelection() const
    { return wxVListBox::GetSelection(); }

    // see ctrlsub.h for more info about this:
    wxCONTROL_ITEMCONTAINER_CLIENTDATAOBJECT_RECAST


    // accessing strings
    // -----------------

    virtual unsigned int GetCount() const
    { return m_items.GetCount(); }

    virtual wxString GetString(unsigned int n) const;

    // override default unoptimized wxItemContainer::GetStrings() function
    wxArrayString GetStrings() const
    { return m_items; }

    virtual void SetString(unsigned int n, const wxString& s);

    virtual void Clear();
    virtual void Delete(unsigned int n);

    void	SetBitmap(unsigned int n, const wxBitmap& bmp);
    wxBitmap*	GetBitmap(unsigned int n);

    // override default unoptimized wxItemContainer::Append() function
    void Append(const wxArrayString& strings);

    // since we override one Append() overload, we need to overload all others too
    int Append(const wxString& item)
    { return wxItemContainer::Append(item); }
    int Append(const wxString& item, void *clientData)
    { return wxItemContainer::Append(item, clientData); }
    int Append(const wxString& item, wxClientData *clientData)
    { return wxItemContainer::Append(item, clientData); }

protected:
    // Abstract functions called by wxItemContainer

    virtual int DoAppend(const wxString& item);
    virtual int DoInsert(const wxString& item, unsigned int pos);

    virtual void DoSetItemClientData(unsigned int n, void *clientData)
    { m_clientData[n] = clientData; }
    virtual void *DoGetItemClientData(unsigned int n) const
    { return m_clientData[n]; }

    virtual void DoSetItemClientObject(unsigned int n, wxClientData *clientData)
    { m_clientData[n] = (void *)clientData; }
    virtual wxClientData *DoGetItemClientObject(unsigned int n) const
    { return (wxClientData *)m_clientData[n]; }

    // calls wxImageListBox::SetItemCount() and RefreshAll()
    void	UpdateCount();

    // overload these functions just to change their visibility: users of
    // wxSimpleImageListBox shouldn't be allowed to call them directly!
    virtual void SetItemCount(size_t count)
    { wxImageListBox::SetItemCount(count); }
    virtual void SetLineCount(size_t count)
    { wxImageListBox::SetLineCount(count); }

    // Callbacks from wxImageListBox to retrieve data

    virtual wxString OnGetItemString(size_t n) const
    { return m_items[n]; }

    virtual wxBitmap* OnGetItemBitmap(size_t n) const
    { return (wxBitmap*)m_bitmaps[n]; }


    wxArrayString   m_items;		// Strings associated with items
    wxArrayPtrVoid  m_bitmaps;		// Images associated with items
    wxArrayPtrVoid  m_clientData;	// User data associated

    // Note: For the benefit of old compilers (like gcc-2.8) this should
    // not be named m_clientdata as that clashes with the name of an
    // anonymous struct member in wxEvtHandler, which we derive from.

    DECLARE_NO_COPY_CLASS(wxSimpleImageListBox);
};

#endif // _WX_IMAGLBOX_H_

