// $Id$

#ifndef ENCTAIN_H
#define ENCTAIN_H

#include <string>

/// Holds all public declarations and classes of Enctain.
namespace Enctain {

/**
 * Enumeration of different error return codes. They can be resolved into
 * English strings using GetErrorString().
 */
enum error_t
{
    ETE_SUCCESS = 0,
  
    ETE_SAVE_NO_PASSWORD,
    ETE_SAVE_OUTPUT_ERROR,

    ETE_LOAD_HEADER1,
    ETE_LOAD_HEADER1_SIGNATURE,
    ETE_LOAD_HEADER1_VERSION,
    ETE_LOAD_HEADER1_METADATA,
    ETE_LOAD_HEADER1_METADATA_PARSE,

    ETE_LOAD_HEADER3,
    ETE_LOAD_HEADER3_ENCRYPTION,
    ETE_LOAD_HEADER3_METADATA,
    ETE_LOAD_HEADER3_METADATA_CRC32,
    ETE_LOAD_HEADER3_METADATA_PARSE,

    ETE_LOAD_SUBFILE,

    ETE_SUBFILE_COMPRESSION_INVALID,
    ETE_SUBFILE_ENCRYPTION_INVALID,
    ETE_SUBFILE_ENCRYPTION_LENGTH,    

    ETE_SUBFILE_UNEXPECTED_EOF,
    ETE_SUBFILE_CRC32,
    ETE_SUBFILE_OUTPUT_ERROR,

    ETE_Z_UNKNOWN,
    ETE_Z_OK,
    ETE_Z_NEED_DICT,
    ETE_Z_STREAM_END,
    ETE_Z_ERRNO,
    ETE_Z_STREAM_ERROR,
    ETE_Z_DATA_ERROR,
    ETE_Z_MEM_ERROR,
    ETE_Z_BUF_ERROR,
    ETE_Z_VERSION_ERROR,

    ETE_BZ_UNKNOWN,
    ETE_BZ_OK,
    ETE_BZ_RUN_OK,
    ETE_BZ_FLUSH_OK,
    ETE_BZ_FINISH_OK,
    ETE_BZ_STREAM_END,
    ETE_BZ_SEQUENCE_ERROR,
    ETE_BZ_PARAM_ERROR,
    ETE_BZ_MEM_ERROR,
    ETE_BZ_DATA_ERROR,
    ETE_BZ_DATA_ERROR_MAGIC,
    ETE_BZ_IO_ERROR,
    ETE_BZ_UNEXPECTED_EOF,
    ETE_BZ_OUTBUFF_FULL,
    ETE_BZ_CONFIG_ERROR,

};

/**
 * Enumeration of different supported encryption algorithms which can be
 * applied to individual files.
 */
enum encryption_type
{
    ENCRYPTION_NONE = 0,
    ENCRYPTION_SERPENT256 = 1
};

/**
 * Enumeration of different supported compression algorithms which can be
 * applied to individual files.
 */
enum compression_type
{
    COMPRESSION_NONE = 0,
    COMPRESSION_ZLIB = 1,
    COMPRESSION_BZIP2 = 2
};

/**
 * Enumeration of progress indicator (text) descriptions. Used instead of the
 * text string for localization.
 */
enum progress_indicator_type {
    PI_GENERIC = 0,
    PI_SAVE_CONTAINER,
    PI_LOAD_CONTAINER,
    PI_REENCRYPT,
    PI_SAVE_SUBFILE,
    PI_LOAD_SUBFILE
};

/**
 * Library Initialization and Shutdown Object. Create it in the main() function
 * or object. Loads and initializes the ciphers (in the Botan library).
 */
class LibraryInitializer
{
public:
    static void initialize(const std::string& args = "");
    static void deinitialize();

    LibraryInitializer(const std::string& args = "") { initialize(args); }
    ~LibraryInitializer() { deinitialize(); }
};

/**
 * Abstract interface class which receives data during container saving or
 * subfile decompression/decryption operations. It is a generic data receiver.
 */
class DataOutput
{
public:

    /// Required.
    virtual ~DataOutput() {};

    /// Pure virtual function which gets data block-wise. If it returns true
    /// the output process continues, on false it will abort.
    virtual bool	Output(const void* data, size_t datalen) = 0;
};

/**
 * Abstract interface class which requests data during container loading. It is
 * a generic stream interface and only have one function.
 */
class DataInput
{
public:

    /// Required.
    virtual ~DataInput() {};

    /// Pure virtual function which requests data block-wise. The function must
    /// return maxlen byte at once, if they are available (this is different
    /// from read()'s semantics). Must return the number of bytes retrieved.
    virtual unsigned int Input(void* data, size_t maxlen) = 0;
};

/**
 * Abstract interface class which can be used to accept progress information
 * during longer operations.
 */

class ProgressIndicator
{
public:

    /// Required.
    virtual ~ProgressIndicator() {};

    /// Pure virtual function called when the progress indicator should
    /// start. The current value and range is given in this call. This call may
    /// be repeated to adjust the text or range during a running process.
    virtual void	ProgressStart(const char* pitext,
				      progress_indicator_type pitype,
				      size_t value, size_t limit) = 0;

    /// Pure virtual function called when the progress indicator should be
    /// updated.
    virtual void	ProgressUpdate(size_t value) = 0;

    /// Pure virtual function called when the progress indicator should be
    /// hidden.
    virtual void	ProgressStop() = 0;
};

/// Hold internal declarations implementing the encrypted container class.
namespace internal {
// Forward declaration for internal implementation class.
class ContainerImpl;
}

/**
 * Class holding all data loaded from an encrypted container. The classes uses
 * a reference-counted pointer implementation. This means you can safely assign
 * and copy objects, where a copy does not create a new object.
 */
class Container
{
protected:

    class internal::ContainerImpl*	pimpl;

public:
    // *** Constructors, Destructor, Assignment ***

    /// Default construction: creates a new empty container object and holds a
    /// reference to it.
    Container();

    /// Decreases the reference counter and eventually deletes the container
    /// object.
    ~Container();

    /// Copy-Constructor: create another reference to the same container
    /// object.
    Container(const Container &cnt);

    /// Assignment-Operator: copy reference to the same container object.
    Container& operator=(const Container &cnt);

    // *** Settings and Error Strings ***

    /// Change the signature used by Enctain which defaults to "CryptoTE". The
    /// signature is always 8 characters long and will be truncated or padded
    /// with zeros. The signature is shared between all instances.
    static void		SetSignature(const char* sign);

    /// Return a one-line English description of the error code.
    static const char*	GetErrorString(error_t e);

    // *** Load/Save/Clear Operations ***

    /// Save the current container by outputting all data to the data sink.
    error_t		Save(DataOutput& dataout);

    /// Load a new container from an input stream and parse the subfile index.
    error_t		Load(DataInput& datain, const std::string& userkey);

    /// Reset all structures in the container
    void		Clear();


    // *** Container Info Operations ***

    /// Return number of bytes written to data sink during last Save()
    /// operation.
    size_t		GetLastWritten() const;

    /// Set the Progress Indicator object which receives progress notifications
    void		SetProgressIndicator(ProgressIndicator* pi);


    // *** Container User KeySlots Operations ***

    /// Return number of user key slots used.
    unsigned int	CountKeySlots() const;
    
    /// Add a new user key string. The string will be hashed and used to store
    /// a copy of the master key. SubFiles do not need to be
    /// reencrypted. Returns number of new key slot.
    unsigned int	AddKeySlot(const std::string& key);

    /// Replace a key slot with a new key string. Requires that the container
    /// was opened using one of the previously existing user key slots.
    void		ChangeKeySlot(unsigned int slot, const std::string& newkey);

    /// Remove a user key slot. When all user key slots are removed and a new
    /// key is added, a new master key is also generated.
    void		DeleteKeySlot(unsigned int slot);

    /// Return the number of the key slot which matched while loading the
    /// file. Returns -1 if it was deleted.
    int			GetUsedKeySlot() const;


    // *** Container Unencrypted Global Properties ***

    /// Set (overwrite) an unencrypted global property.
    void		SetGlobalUnencryptedProperty(const std::string& key, const std::string& value);
    
    /// Get an unencrypted  global property by key.
    const std::string&	GetGlobalUnencryptedProperty(const std::string& key) const;

    /// Delete an unencrypted  global property key.
    bool		DeleteGlobalUnencryptedProperty(const std::string& key);
    
    /// Get an unencrypted global property (key and value) by index. Returns
    /// false if the index is beyond the last property
    bool		GetGlobalUnencryptedPropertyIndex(unsigned int propindex,
							  std::string& key, std::string& value) const;


    // *** Container Encrypted Global Properties ***

    /// Set (overwrite) an encrypted global property.
    void		SetGlobalEncryptedProperty(const std::string& key, const std::string& value);
    
    /// Get an encrypted global property by key.
    const std::string&	GetGlobalEncryptedProperty(const std::string& key) const;

    /// Delete an encrypted global property key.
    bool		DeleteGlobalEncryptedProperty(const std::string& key);
    
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

    /// Delete a subfile's property key.
    bool		DeleteSubFileProperty(unsigned int subfileindex, const std::string& key);
    
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
    encryption_type	GetSubFileEncryption(unsigned int subfileindex) const;

    /// Return compression method of the subfile.
    compression_type	GetSubFileCompression(unsigned int subfileindex) const;


    // * Set operations of subfile header fields *

    /// Set data encryption flag of a subfile. This can be an expensive
    /// operation as the memory buffer may need to be decrypted/encrypted.
    void		SetSubFileEncryption(unsigned int subfileindex,
					     encryption_type c);

    /// Set data compression flag of a subfile. This can be an expensive
    /// operation as the memory buffer may need to be decompressed/compressed.
    void		SetSubFileCompression(unsigned int subfileindex,
					      compression_type c);

    /// Set both data compression and encryption flags of a subfile. This can
    /// be an expensive operation as the memory buffer may need to be
    /// decompressed/compressed and reencrypted.
    void		SetSubFileCompressionEncryption(unsigned int subfileindex,
							compression_type comp,
							encryption_type enc);


    // * Subfile data operations *

    /// Return the data of a subfile: decrypt and uncompress it. The data is
    /// sent block-wise to the DataOutput object.
    error_t		GetSubFileData(unsigned int subfileindex, class DataOutput& dataout) const;

    /// Return the data of a subfile: decrypt and uncompress it. Return
    /// complete data in a memory string.
    error_t		GetSubFileData(unsigned int subfileindex, std::string& data) const;

    /// Set/change the data of a subfile, it will be compressed and encrypted
    /// but not written to disk, yet.
    error_t		SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen);
};

} // namespace Enctain

#endif // ENCTAIN_H
