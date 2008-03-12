// $Id$

#ifndef ENCTAIN_H
#define ENCTAIN_H

#include <wx/stream.h>
#include <wx/buffer.h>

#include <vector>
#include <map>
#include <string>

#include "serpent.h"

namespace Enctain {

/// Enumeration of different supported encryption algorithms which can be
/// applied to individual files.
enum encryption_t {
    ENCRYPTION_NONE = 0,
    ENCRYPTION_SERPENT256 = 1
};

/// Enumeration of different supported compression algorithms which can be
/// applied to individual files.
enum compression_t {
    COMPRESSION_NONE = 0,
    COMPRESSION_ZLIB = 1,
    COMPRESSION_BZIP2 = 2
};

/**
 * Abstract interface class which receives data during loading operations.
 */
class DataAcceptor
{
public:

    /// Required.
    virtual ~DataAcceptor();

    /// Pure virtual function which gets data block-wise.
    virtual void	Append(const void* data, size_t datalen) = 0;
};

/**
 * Class holding all data loaded from an encrypted container.
 */
class Container
{
protected:

    typedef std::map<std::string, std::string> propertymap_type;

    // *** Structures ***

    /// Memory structure used to represent a subfile of the container, not
    /// directly used for disk storage.
    class SubFile
    {
    public:
	/// Size of subfile when saved.
	uint32_t	storagesize;

	/// Size of subfile when read by user program.
	uint32_t	realsize;

	/// CRC32 of subfile after decryption and decompression.
	uint32_t	crc32;

	union {
	    uint32_t	flags;
	    struct {
		uint8_t	compression;
		uint8_t	encryption;
		uint8_t	reserved1;
		uint8_t	reserved2;
	    };
	};

	/// Encryption CBC initialization vector, if needed.
	unsigned char	cbciv[16];
    
	/// User-defined properties of the subfile.
	propertymap_type properties;

	/// Compressed and encrypted data of subfile.
	wxMemoryBuffer	data;

	/// Constructor initializing everything to zero.
	SubFile()
	    : storagesize(0), realsize(0), crc32(0), flags(0)
	{
	}
    };

    /// Structure of the disk file's header
    struct Header1
    {
	char    	signature[8];	// "CryptoTE"
	uint32_t	version;	// Currently 0x00010000 = v1.0
	uint32_t	unc_metalen;	// Unencrypted Metadata Length

    } __attribute__((packed));

    /// Structure of the encrypted part of the header
    struct Header2
    {
	uint32_t	test123;	// = 0x12345678 to quick-test if
					// decryption worked.
	uint32_t	metacomplen;	// Length of following compressed
					// variable header holding all subfile
					// metadata.
	uint32_t	metacrc32;	// CRC32 of the following variable
					// subfile metadata header
	uint32_t	subfilenum;	// Number of subfiles in the container
					// excluding the structure for
					// globalproperties.

    } __attribute__((packed));

    // *** Status and Container Contents Variables ***

    /// Signature possibly changed by SetSignature()
    static char		fsignature[8];

    /// True if a container is loaded correctly.
    bool		opened;

    /// True if one of the subfiles was changed using saveSubFile() and the
    /// container file was not saved yet.
    bool		modified;

    /// 256-bit raw encryption key.
    bool		iskeyset;

    /// Serpent256 keybit encryption context
    mutable SerpentCBC	serpentctx;

    /// Unencrypted global properties, completely user-defined.
    propertymap_type	unc_properties;

    /// Encrypted global properties, completely user-defined.
    propertymap_type	enc_properties;

    /// Vector of subfiles
    std::vector<SubFile> subfiles;

    /// Bytes written to file during last Save() operation
    size_t		written;

public:
    // *** Constructor and Destructor ***

    Container();

    ~Container();

    // *** Settings ***

    /// Change the signature used by Enctain which defaults to "enctain\0". The
    /// signature is always 8 characters long and will be truncated or padded
    /// with zeros. The signature is shared between all instances.
    static void		SetSignature(const char* sign);

    // *** Load/Save Operations ***

    /// Save the current container into the newly defined path and encryption
    /// key.
    bool		Save(wxOutputStream& outstream);

    /// Load a new container from a wxInputStream and parse the subfile index.
    bool		Load(wxInputStream& instream, const std::string& filekey);

    /// Load a container version v1.0
    bool		Loadv00010000(wxInputStream& instream, const std::string& filekey, const Header1& header1);


    // *** Container Info and Key Operations ***

    /// Returns true if the object contains a correctly opened container.
    bool		IsOpen() const;
    
    /// Set a new password string. The string will be hashed and transformed
    /// into an encryption context. This is a very expensive operation as all
    /// subfiles need to be reencrypted.
    void		SetKey(const std::string& keystr);

    /// Checks whether a password key was set.
    bool		IsKeySet() const;

    /// Return number of bytes written to output stream during last Save()
    /// operation.
    size_t		GetLastWritten() const;

    // *** Container Unencrypted Global Properties ***

    /// Set (overwrite) an unencrypted global property.
    void		SetGlobalUnencryptedProperty(const std::string& key, const std::string& value);
    
    /// Get an unencrypted  global property by key.
    const std::string&	GetGlobalUnencryptedProperty(const std::string& key) const;

    /// Erase an unencrypted  global property key.
    bool		EraseGlobalUnencryptedProperty(const std::string& key);
    
    /// Get an unencrypted global property (key and value) by index. Returns
    /// false if the index is beyond the last property
    bool		GetGlobalUnencryptedPropertyIndex(unsigned int propindex,
							  std::string& key, std::string& value) const;


    // *** Container Encrypted Global Properties ***

    /// Set (overwrite) an encrypted global property.
    void		SetGlobalEncryptedProperty(const std::string& key, const std::string& value);
    
    /// Get an encrypted global property by key.
    const std::string&	GetGlobalEncryptedProperty(const std::string& key) const;

    /// Erase an encrypted global property key.
    bool		EraseGlobalEncryptedProperty(const std::string& key);
    
    /// Get an encrypted global property (key and value) by index. Returns
    /// false if the index is beyond the last property
    bool		GetGlobalEncryptedPropertyIndex(unsigned int propindex,
							std::string& key, std::string& value) const;


    // *** Container SubFiles ***


    // * Subfile array management *

    /// Return number of subfiles in the container.
    unsigned int 	CountSubFile() const;

    /// Append an empty uninitialized subfile at the end of the list. Returns
    /// the new subfile's index.
    unsigned int 	AppendSubFile();

    /// Insert an empty uninitialized subfile at the given position in the
    /// list. Returns the new subfile's index.
    unsigned int 	InsertSubFile(unsigned int subfileindex);

    /// Delete a subfile in the array. Returns true if it existed.
    bool		DeleteSubFile(unsigned int subfileindex);


    // * Subfile user-defined properties *

    /// Set (overwrite) a subfile's property.
    void		SetSubFileProperty(unsigned int subfileindex, const std::string& key, const std::string& value);

    /// Get a subfile's property by key. Returns an empty string if it is not set.
    const std::string&	GetSubFileProperty(unsigned int subfileindex, const std::string& key) const;

    /// Erase a subfile's property key.
    bool		EraseSubFileProperty(unsigned int subfileindex, const std::string& key);
    
    /// Get a subfile's property (key and value) by index. Returns false if the
    /// index is beyond the last property
    bool		GetSubFilePropertyIndex(unsigned int subfileindex, unsigned int propindex,
						std::string& key, std::string& value) const;


    // * Get operations of subfile header fields *
    
    /// Return number of bytes the subfile requires on disk, after compression
    /// and encryption.
    uint32_t		GetSubFileStorageSize(unsigned int subfileindex) const;

    /// Return number of bytes the subfile requires when decrypted and
    /// decompressed.
    uint32_t		GetSubFileSize(unsigned int subfileindex) const;

    /// Return encryption cipher of the subfile.
    encryption_t	GetSubFileEncryption(unsigned int subfileindex) const;

    /// Return compression method of the subfile.
    compression_t	GetSubFileCompression(unsigned int subfileindex) const;


    // * Set operations of subfile header fields *

    /// Set data encryption flag of a subfile. This can be an expensive
    /// operation as the memory buffer may need to be decrypted/encrypted.
    void		SetSubFileEncryption(unsigned int subfileindex, encryption_t c);

    /// Set data compression flag of a subfile. This can be an expensive
    /// operation as the memory buffer may need to be decompressed/compressed.
    void		SetSubFileCompression(unsigned int subfileindex, compression_t c);

    /// Set both data compression and encryption flags of a subfile. This can
    /// be an expensive operation as the memory buffer may need to be
    /// decompressed/compressed and reencrypted.
    void		SetSubFileCompressionEncryption(unsigned int subfileindex, compression_t comp, encryption_t enc);


    // * Subfile data operations *

    /// Return the data of a subfile: decrypt and uncompress it. The data is
    /// sent block-wise to the DataAcceptor object.
    bool		GetSubFileData(unsigned int subfileindex, class DataAcceptor& da) const;

    /// Return the data of a subfile: decrypt and uncompress it. Return
    /// complete data in a memory buffer.
    bool		GetSubFileData(unsigned int subfileindex, wxMemoryBuffer& data) const;

    /// Set/change the data of a subfile, it will be compressed and encrypted
    /// but not written to disk, yet.
    bool		SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen);
};

} // namespace Enctain

#endif // ENCTAIN_H
