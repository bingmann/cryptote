// $Id$

#ifndef WTEXTPAGE_H
#define WTEXTPAGE_H

#include <wx/wx.h>
#include <wx/filename.h>
#include <stc.h>

#include "wcryptote.h"

class WTextPage : public WNotePage
{
public:

    WTextPage(class WCryptoTE* parent);

    /// Return the text to display in the notebook
    virtual wxString	GetCaption();

#if 0
/*****************************************************************************/

    /// Currently opened file name
    class wxFileName	currentfilename;

    /// True if buffer modified in editor
    bool		modified;

    /// Margins Enumeration
    enum margin_num {
	MARGIN_LINENUMBER = 0
    };

public:
    // *** File operations ***

    /// Clear the currently loaded file and start a new one
    void	FileNew();

    /// Attempt to open and load an existing file
    bool	FileOpen(const wxString& filename);

    /// Save currently opened buffer into the associated file name
    bool	FileSave();

    /// Save buffer into new file name
    bool	FileSaveAs(const wxString& filename);

    /// Reload the associated file
    bool	FileRevert();

    /// Load a file from the given wxInputStream
    bool	LoadInputStream(wxInputStream& stream);

    /// Has an associated file, used by FileSave()
    bool	HasFilename() const;

    /// Return full path the associated file
    wxString	GetFileFullpath() const;

    /// Return only the file name component with extension.
    wxString	GetFileBasename() const;

    // _Updateable_ reference to modification flag of current file.
    bool&	ModifiedFlag();

    // *** Display Settings ***

    void	ShowLineNumber(bool on);

    // *** Event Handlers ***

    // Edit Menu
    void	OnMenuEditUndo(wxCommandEvent& event);
    void	OnMenuEditRedo(wxCommandEvent& event);

    void	OnMenuEditCut(wxCommandEvent& event);
    void	OnMenuEditCopy(wxCommandEvent& event);
    void	OnMenuEditPaste(wxCommandEvent& event);
    void	OnMenuEditDelete(wxCommandEvent& event);

    void	OnMenuEditSelectAll(wxCommandEvent &event);
    void	OnMenuEditSelectLine(wxCommandEvent &event);

/*****************************************************************************/
#endif

private:
    DECLARE_EVENT_TABLE()
};

#endif // WTEXTPAGE_H
