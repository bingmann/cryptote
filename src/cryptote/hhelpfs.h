// $Id$

#ifndef HHELPFS_H
#define HHELPFS_H

#include <wx/filesys.h>

/**
 * FileSystemHandler serving only the help: protocol with files from compressed
 * compiled-in data.
 */
class BuiltinHtmlHelpFSHandler : public wxFileSystemHandler
{
public:
    /// Does nothing.
    BuiltinHtmlHelpFSHandler();

    /// Required for virtual functions.
    ~BuiltinHtmlHelpFSHandler();

    /// Check that the location start with help: protocol.
    virtual bool CanOpen(const wxString& location);

    /// Tries to open the file specified.
    virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location);

protected:

    struct BuiltinFile
    {
        const wxString  path;
        const char*     compressed_data;
        unsigned int    compressed_size;
        unsigned int    uncompressed_size;
        std::string     decompressed_data;
    };

    static struct BuiltinFile   filelist[];
    static unsigned int         filelistsize;

private:
    /// No copy construction allowed.
    BuiltinHtmlHelpFSHandler(const BuiltinHtmlHelpFSHandler&);
    
    /// No assignment allowed.
    BuiltinHtmlHelpFSHandler& operator=(const BuiltinHtmlHelpFSHandler&);
};

#endif // HHELPFS_H
