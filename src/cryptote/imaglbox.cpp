///////////////////////////////////////////////////////////////////////////////
// Name:        imagllbox.cpp
// Purpose:     implementation of wxImageListBox
// Author:      Timo Bingmann
// Created:     2008-07-21
// RCS-ID:      $Id$
// License:     wxWindows license
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "imaglbox.h"

#include "wx/settings.h"
#include "wx/dc.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// Space left of image
static const wxCoord default_image_spacing_right = 4;

// Space right of image, left of text
static const wxCoord default_image_spacing_left = 4;

// Space top and bottom of image
static const wxCoord default_image_spacing_vertical = 2;

// Space top and bottom of text
static const wxCoord default_text_spacing_vertical = 4;

// Window class strings    
const wxChar wxImageListBoxNameStr[] = wxT("imageListBox");
const wxChar wxSimpleImageListBoxNameStr[] = wxT("simpleImageListBox");

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_ABSTRACT_CLASS(wxImageListBox, wxVListBox)

// ----------------------------------------------------------------------------
// wxImageListBox creation
// ----------------------------------------------------------------------------

wxImageListBox::wxImageListBox()
    : wxVListBox()
{
    Init();
}

// normal constructor which calls Create() internally
wxImageListBox::wxImageListBox(wxWindow *parent,
			       wxWindowID id,
			       const wxPoint& pos,
			       const wxSize& size,
			       long style,
			       const wxString& name)
    : wxVListBox()
{
    Init();

    (void)Create(parent, id, pos, size, style, name);
}

void wxImageListBox::Init()
{
    m_image_spacing_right = default_image_spacing_right;
    m_image_spacing_left = default_image_spacing_left;
    m_image_spacing_vertical = default_image_spacing_vertical;
    m_text_spacing_vertical = default_text_spacing_vertical;
}

bool wxImageListBox::Create(wxWindow *parent,
			    wxWindowID id,
			    const wxPoint& pos,
			    const wxSize& size,
			    long style,
			    const wxString& name)
{
    return wxVListBox::Create(parent, id, pos, size, style, name);
}

wxImageListBox::~wxImageListBox()
{
}

// ----------------------------------------------------------------------------
// wxImageListBox appearance
// ----------------------------------------------------------------------------

bool wxImageListBox::SetFont(const wxFont& font)
{
    bool res = wxVListBox::SetFont(font);
    if (res) RefreshAll();
    return res;
}

void wxImageListBox::SetImageSpacing(wxCoord right, wxCoord left, wxCoord vertical)
{
    m_image_spacing_right = right;
    m_image_spacing_left = left;
    m_image_spacing_vertical = vertical;

    RefreshAll();
}

void wxImageListBox::SetTextSpacing(wxCoord vertical)
{
    m_text_spacing_vertical = vertical;

    RefreshAll();
}

// ----------------------------------------------------------------------------
// wxImageListBox implementation of wxVListBox pure virtuals
// ----------------------------------------------------------------------------

void wxImageListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    // Set correct text colour for selected items
    if ( wxVListBox::GetSelection() == (int)n )
    {
        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );
    }
    else
    {
        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT) );
    }

    int textIndent = 0;

    wxBitmap* bmp = OnGetItemBitmap(n);

    if (bmp && bmp->IsOk())
    {
        wxCoord w = bmp->GetWidth();
        wxCoord h = bmp->GetHeight();

        // Draw the image centered
        dc.DrawBitmap(*bmp,
                      rect.x + m_image_spacing_left,
                      rect.y + (rect.height - h) / 2,
                      true);

        textIndent = w + m_image_spacing_left + m_image_spacing_right;
    }

    wxString str = OnGetItemString(n);

    int strHeight = 0, strWidth = 0;
    dc.GetTextExtent(str, &strWidth, &strHeight);

    dc.DrawLabel(str,
		 wxRect(rect.x + textIndent,
			rect.y,
			rect.width - textIndent,
			rect.height),
		 wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
}

wxCoord wxImageListBox::OnMeasureItem(size_t n) const
{
    int textHeight = 0, textWidth = 0;
    GetTextExtent( OnGetItemString(n), &textWidth, &textHeight);
    textHeight += 2 * m_text_spacing_vertical;

    int imgHeight = 0;

    wxBitmap* bmp = OnGetItemBitmap(n);
    if (bmp && bmp->IsOk())
	imgHeight = bmp->GetHeight() + 2 * m_image_spacing_vertical;

    return wxMax(imgHeight, textHeight);
}

// ----------------------------------------------------------------------------
// wxSimpleImageListBox
// ----------------------------------------------------------------------------

bool wxSimpleImageListBox::Create(wxWindow *parent, wxWindowID id,
				  const wxPoint& pos,
				  const wxSize& size,
				  int n, const wxString choices[],
				  long style,
				  const wxValidator& validator,
				  const wxString& name)
{
    if (!wxImageListBox::Create(parent, id, pos, size, style, name))
        return false;

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif
    for (int i=0; i<n; i++)
        Append(choices[i]);

    return true;
}

bool wxSimpleImageListBox::Create(wxWindow *parent, wxWindowID id,
				  const wxPoint& pos,
				  const wxSize& size,
				  const wxArrayString& choices,
				  long style,
				  const wxValidator& validator,
				  const wxString& name)
{
    if (!wxImageListBox::Create(parent, id, pos, size, style, name))
        return false;

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif
    Append(choices);

    return true;
}

wxSimpleImageListBox::~wxSimpleImageListBox()
{
    wxASSERT(m_items.GetCount() == m_bitmaps.GetCount());
    wxASSERT(m_items.GetCount() == m_clientData.GetCount());

    for (size_t i=0; i<m_bitmaps.GetCount(); i++)
	if (m_bitmaps[i]) delete (wxBitmap*)m_bitmaps[i];

    if (HasClientObjectData())
    {
        // clear the array of client data objects
        for (size_t i=0; i<m_items.GetCount(); i++)
            if (m_clientData[i]) delete DoGetItemClientObject(i);
    }

    m_items.Clear();
    m_bitmaps.Clear();
    m_clientData.Clear();
}

void wxSimpleImageListBox::Clear()
{
    wxASSERT(m_items.GetCount() == m_bitmaps.GetCount());
    wxASSERT(m_items.GetCount() == m_clientData.GetCount());

    for (size_t i=0; i<m_bitmaps.GetCount(); i++)
	if (m_bitmaps[i]) delete (wxBitmap*)m_bitmaps[i];

    if (HasClientObjectData())
    {
        // clear the array of client data objects
        for (size_t i=0; i<m_items.GetCount(); i++)
            if (m_clientData[i]) delete DoGetItemClientObject(i);
    }

    m_items.Clear();
    m_bitmaps.Clear();
    m_clientData.Clear();
    UpdateCount();
}

void wxSimpleImageListBox::Delete(unsigned int n)
{
    if (HasClientObjectData())
	if (m_clientData[n]) delete DoGetItemClientObject(n);

    if (m_bitmaps[n]) delete (wxBitmap*)m_bitmaps[n];

    m_items.RemoveAt(n);
    m_bitmaps.RemoveAt(n);
    m_clientData.RemoveAt(n);
    UpdateCount();
}

void wxSimpleImageListBox::Append(const wxArrayString& strings)
{
    // append all given items at once
    WX_APPEND_ARRAY(m_items, strings);

    m_bitmaps.Add(NULL, strings.GetCount());
    m_clientData.Add(NULL, strings.GetCount());

    UpdateCount();
}

int wxSimpleImageListBox::DoAppend(const wxString& item)
{
    m_items.Add(item);
    m_bitmaps.Add(NULL);
    m_clientData.Add(NULL);
    UpdateCount();
    return GetCount()-1;
}

int wxSimpleImageListBox::DoInsert(const wxString& item, unsigned int pos)
{
    m_items.Insert(item, pos);
    m_bitmaps.Insert(NULL, pos);
    m_clientData.Insert(NULL, pos);
    UpdateCount();
    return pos;
}

void wxSimpleImageListBox::SetString(unsigned int n, const wxString& s)
{
    wxCHECK_RET( IsValid(n),
                 wxT("invalid index in wxSimpleImageListBox::SetString") );

    m_items[n] = s; 
    RefreshLine(n);
}

wxString wxSimpleImageListBox::GetString(unsigned int n) const
{
    wxCHECK_MSG( IsValid(n), wxEmptyString,
                 wxT("invalid index in wxSimpleImageListBox::GetString") );

    return m_items[n];
}

void wxSimpleImageListBox::SetBitmap(unsigned int n, const wxBitmap& bmp)
{
    wxCHECK_RET( IsValid(n),
                 wxT("invalid index in wxSimpleImageListBox::SetBitmap") );

    if (!m_bitmaps[n]) m_bitmaps[n] = (void*)(new wxBitmap(bmp));
    else (*(wxBitmap*)m_bitmaps[n]) = bmp;

    RefreshLine(n);
}

wxBitmap* wxSimpleImageListBox::GetBitmap(unsigned int n)
{
    wxCHECK_MSG( IsValid(n), NULL,
                 wxT("invalid index in wxSimpleImageListBox::GetBitmap") );

    return (wxBitmap*)m_bitmaps[n];
}

void wxSimpleImageListBox::UpdateCount()
{
    wxASSERT(m_items.GetCount() == m_bitmaps.GetCount());
    wxASSERT(m_items.GetCount() == m_clientData.GetCount());

    wxImageListBox::SetItemCount(m_items.GetCount());

    // very small optimization: if you need to add lot of items to
    // a wxSimpleImageListBox be sure to use the
    // wxSimpleImageListBox::Append(const wxArrayString&) method instead!
    if (!this->IsFrozen())
        RefreshAll();
}
