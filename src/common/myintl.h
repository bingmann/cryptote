// $Id$

// Custom Implementation of a locale class based on wxLocale's source from
// wxWidgets 2.8.7. The class MyLocale is derived from wxLocale and implements
// lookups from gettext catalogs stored in memory. This way no external .mo
// files are required, which are problematic on Windows.

/////////////////////////////////////////////////////////////////////////////
// Name:        wx/intl.h
// Purpose:     Internationalization and localisation for wxWidgets
// Author:      Vadim Zeitlin
// Modified by: Michael N. Filippov <michael@idisys.iae.nsk.su>
//              (2003/09/30 - plural forms support)
// Created:     29/01/98
// RCS-ID:      $Id$
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef MYINTL_H
#define MYINTL_H

#include <wx/intl.h>

/// List of message catalogs for different languages: binary .mo files stored
/// in memory (passed by msgCatalogData / msgCatalogDataLen)
struct MyLocaleMemoryCatalog
{
    const wxChar*	msgIdLanguage;	// language e.g. "us_GB"
    const wxChar*	msgIdCharset;	// character set
    const unsigned char* msgCatalogData;	// ptr to binary catalog
    unsigned int	msgCatalogDataLen; // size of data at ptr
    unsigned int	msgCatalogUncompLen; // uncompressed size len, if 0 ->
					     // not compressed.
};

class MyLocale : public wxLocale
{
public:
    // ctor & dtor
    // -----------

    // call Init() if you use this ctor
    MyLocale();

    // the ctor has a side effect of changing current locale
    MyLocale(const wxChar *szName,                               // name (for messages)
             const wxChar *szShort = (const wxChar *) NULL,      // dir prefix (for msg files)
             const wxChar *szLocale = (const wxChar *) NULL,     // locale (for setlocale)
             bool bLoadDefault = true,                           // preload wxstd.mo?
             bool bConvertEncoding = false);                      // convert Win<->Unix if necessary?

    MyLocale(int language, // wxLanguage id or custom language
             int flags = wxLOCALE_LOAD_DEFAULT | wxLOCALE_CONV_ENCODING);

    virtual ~MyLocale();

    // add a catalog from memory: load the desired language from the catalog
    // list.
    //
    // The loaded catalog will be used for message lookup by GetString(). List
    // of language catalogs. The list is terminated with a msgIdLanguage ==
    // NULL.
    //
    // Returns 'true' if it was successfully loaded
    bool AddCatalogFromMemory(const wxChar *szDomain, const MyLocaleMemoryCatalog* msgCatalogMemory);

    // check if the given catalog is loaded
    bool IsLoaded(const wxChar *szDomain) const;

    // retrieve the translation for a string in all loaded domains unless
    // the szDomain parameter is specified (and then only this domain is
    // searched)
    // n - additional parameter for PluralFormsParser
    //
    // return original string if translation is not available
    // (in this case an error message is generated the first time
    //  a string is not found; use wxLogNull to suppress it)
    //
    // domains are searched in the last to first order, i.e. catalogs
    // added later override those added before.
    virtual const wxChar *GetString(const wxChar *szOrigString,
                                    const wxChar *szDomain = NULL) const;
    // plural form version of the same:
    virtual const wxChar *GetString(const wxChar *szOrigString,
                                    const wxChar *szOrigString2,
                                    size_t n,
                                    const wxChar *szDomain = NULL) const;

    // return the contents of .po file header
    wxString GetHeaderValue( const wxChar* szHeader,
                             const wxChar* szDomain = NULL ) const;

private:
    // find catalog by name in a linked list, return NULL if !found
    class MyMsgCatalog  *FindCatalog(const wxChar *szDomain) const;

    // initialize the member fields to default values
    void DoMyCommonInit();

    class MyMsgCatalog *m_pMsgCat;         // pointer to linked list of catalogs

    DECLARE_NO_COPY_CLASS(MyLocale);
};

#endif // MYINTL_H
