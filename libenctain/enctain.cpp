// $Id$

#include "enctain.h"

#include <vector>
#include <map>
#include <memory>

#include <zlib.h>

#include "bytebuff.h"

#include "botan-1.6/include/base.h"
#include "botan-1.6/include/init.h"
#include "botan-1.6/include/rng.h"
#include "botan-1.6/include/lookup.h"
#include "botan-1.6/include/pkcs5.h"
#include "botan-1.6/include/crc32.h"
#include "botan-1.6/include/pipe.h"
#include "botan-1.6/include/buf_filt.h"
#include "botan-1.6/include/zlib.h"
#include "botan-1.6/include/bzip2.h"

namespace Enctain {
namespace internal {

// *** ContainerImpl ***

class ContainerImpl
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

	/// Random encryption key, if needed.
	Botan::SecureBuffer<Botan::byte, 32>	enckey;

	/// Encryption CBC initialization vector, if needed.
	Botan::SecureBuffer<Botan::byte, 16>	cbciv;
    
	/// User-defined properties of the subfile.
	propertymap_type properties;

	/// Compressed and encrypted data of subfile.
	Botan::SecureVector<Botan::byte>	data;

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

    /// Structure of the disk file's header
    struct Header2
    {
	uint32_t	keyslots;	// Number of key slots following.
	uint32_t	mkd_iterations;	// PBKDF2 digest iterations
	uint8_t		mkd_salt[32];	// PBKDF2 digest salt
	uint8_t		mkd_digest[32];	// PBKDF2 digest
	uint32_t	mkk_iterations;	// PBKDF2 metadata key iterations
	uint8_t		mkk_salt[32];	// PBKDF2 metadata key salt
	uint32_t	mki_iterations;	// PBKDF2 metadata iv iterations
	uint8_t		mki_salt[32];	// PBKDF2 metadata iv salt

    } __attribute__((packed));

    struct KeySlot
    {
	uint32_t	iterations;	// PBKDF2 iterations
	uint8_t		salt[32];	// PBKDF2 random salt
	uint8_t		emasterkey[64];	// Encrypted master key

    } __attribute__((packed));

    /// Structure of the encrypted part of the header
    struct Header3
    {
	uint32_t	test123;	// = 0x12345678 to quick-test if
					// decryption worked.
	uint32_t	metacomplen;	// Length of following compressed
					// variable header holding all subfile
					// metadata.
	uint32_t	metacrc32;	// CRC32 of the following variable
					// subfile metadata header
	uint32_t	padding1;	// Padding

    } __attribute__((packed));

    // *** Status and Container Contents Variables ***

    /// Signature possibly changed by SetSignature()
    static char		fsignature[8];

    /// Number of references to this object
    unsigned int	references;

    /// True if one of the subfiles was changed using SetSubFileData() and the
    /// container file was not saved yet.
    bool		modified;

    /// Header2 either loaded or filled on first user key added.
    Header2		header2;

    /// Master key material used for metadata encryption.
    Botan::SecureBuffer<Botan::byte, 64> masterkey;

    /// Vector of user key slots.
    std::vector<KeySlot> keyslots;

    /// Number of the correct key slot used while loading.
    int			usedkeyslot;

    /// Unencrypted global properties, completely user-defined.
    propertymap_type	unc_properties;

    /// Encrypted global properties, completely user-defined.
    propertymap_type	enc_properties;

    /// Vector of subfiles
    std::vector<SubFile> subfiles;

    /// Bytes written to file during last Save() operation
    size_t		written;

    /// Progress indicator object receiving notifications.
    ProgressIndicator*	progressindicator;

    /// Friend class to access the progressindicator variable
    friend class ProgressTicker;

public:
    // *** Constructor, Destructor and Reference Counter  ***

    ContainerImpl();

    ~ContainerImpl();

    /// Reset all structures in the container
    void		Clear();

    /// Increase reference counter by one.
    void		IncReference();

    /// Decrease reference counter by one and return the new value.
    unsigned int 	DecReference();

    // *** Settings and Error Strings ***

    /// Change the signature used by Enctain which defaults to "CryptoTE". The
    /// signature is always 8 characters long and will be truncated or padded
    /// with zeros. The signature is shared between all instances.
    static void		SetSignature(const char* sign);

    /// Return a one-line English description of the error code.
    static const char*	GetErrorString(error_type e);

    // *** Load/Save Operations ***

    /// Save the current container by outputting all data to the data sink.
    void		Save(DataOutput& dataout);

    /// Load a new container from an input stream and parse the subfile index.
    void		Load(DataInput& datain, const std::string& userkey);

    /// Load a container version v1.0
    void		Loadv00010000(DataInput& datain, const std::string& userkey, const Header1& header1, class ProgressTicker& progress);


    // *** Container Info Operations ***

    /// Return number of bytes written to data sink during last Save()
    /// operation.
    size_t		GetLastWritten() const;

    /// Set the Progress Indicator object which receives progress notifications
    void		SetProgressIndicator(ProgressIndicator* pi);


    // *** Container User Keys Operations ***

    /// Return number of user key slots used.
    unsigned int	CountKeySlots() const;
    
    /// Add a new user key string. The string will be hashed and used to store
    /// a copy of the master key. SubFiles do not need to be
    /// reencrypted. Returns number of new key slot.
    unsigned int	AddKeySlot(const std::string& key);

    /// Replace a key slot with a new key string. Requires that the container
    /// was opened using one of the previously existing user key slots.
    void		ChangeKeySlot(unsigned int slot, const std::string& key);

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
    void		SetSubFileEncryption(unsigned int subfileindex, encryption_type c);

    /// Set data compression flag of a subfile. This can be an expensive
    /// operation as the memory buffer may need to be decompressed/compressed.
    void		SetSubFileCompression(unsigned int subfileindex, compression_type c);

    /// Set both data compression and encryption flags of a subfile. This can
    /// be an expensive operation as the memory buffer may need to be
    /// decompressed/compressed and reencrypted.
    void		SetSubFileCompressionEncryption(unsigned int subfileindex, compression_type comp, encryption_type enc);


    // * Subfile data operations *

    /// Return the data of a subfile: decrypt and uncompress it. The data is
    /// sent block-wise to the DataOutput object.
    void		GetSubFileData(unsigned int subfileindex, class DataOutput& dataout) const;

    /// Return the data of a subfile: decrypt and uncompress it. Return
    /// complete data in a memory string.
    void		GetSubFileData(unsigned int subfileindex, std::string& data) const;

    /// Set/change the data of a subfile, it will be compressed and encrypted
    /// but not written to disk, yet.
    void		SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen);
};

// *** Assert which throws an InternalError  ***

#define AssertException(x)	do { if (!(x)) { throw(InternalError(ETE_TEXT, "Assertion failed: " #x)); } } while(0)

// *** Constructor, Destructor and Reference Counter  ***

ContainerImpl::ContainerImpl()
    : references(1),
      modified(false),
      usedkeyslot(-1),
      written(0),
      progressindicator(NULL)
{
    memset(&header2, 0, sizeof(header2));
}

ContainerImpl::~ContainerImpl()
{
    masterkey.clear();
    keyslots.clear();
    memset(&header2, 0, sizeof(header2));
}

void ContainerImpl::Clear()
{
    modified = false;
    memset(&header2, 0, sizeof(header2));
    masterkey.clear();
    keyslots.clear();
    usedkeyslot = -1;
    unc_properties.clear();
    enc_properties.clear();
    subfiles.clear();
    written = 0;
}

void ContainerImpl::IncReference()
{
    ++references;
}

unsigned int ContainerImpl::DecReference()
{
    return --references;
}

// *** Progress Indicator Wrappers ***

/**
 * Little class which starts a progress by calling ProgressStart() on it's
 * references container object. When it gets destroyed the progress ends.
 */
class ProgressTicker
{
private:
    /// Reference to the container implementation
    const ContainerImpl& 	cnt;

    /// Current counter value
    size_t			current;

public:
    ProgressTicker(const ContainerImpl& c,
		   const char* pitext, progress_indicator_type pitype,
		   size_t value, size_t limit)
	: cnt(c)
    {
	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressStart(pitext, pitype, value, limit);
	current = value;
    }

    void Restart(const char* pitext, progress_indicator_type pitype,
		 size_t value, size_t limit)
    {
	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressStart(pitext, pitype, value, limit);
	current = value;
    }

    void Update(size_t value)
    {
	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressUpdate(value);
	current = value;
    }
    
    void Add(size_t increment)
    {
	current += increment;

	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressUpdate(current);
    }

    ~ProgressTicker()
    {
	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressStop();
    }
};

// *** Settings ***

char ContainerImpl::fsignature[8] =
{ 'C', 'r', 'y', 'p', 't', 'o', 'T', 'E' };

/* static */ void ContainerImpl::SetSignature(const char* sign)
{
    unsigned int i = 0;
    while(sign[i] != 0 && i < 8) {
	fsignature[i] = sign[i];
	++i;
    }
    while(i < 8) {
	fsignature[i] = 0;
	++i;
    }
}

/* static */ const char* ContainerImpl::GetErrorString(error_type e)
{
    switch(e)
    {
    case ETE_SUCCESS:
	return "Success.";

    case ETE_TEXT:
	return "Generic text message error.";

    case ETE_OUTPUT_ERROR:
	return "DataOutput stream error.";

    case ETE_INPUT_ERROR:
	return "DataInput stream error.";

    case ETE_SAVE_NO_KEYSLOTS:
	return "Error saving container: no encryption key slots set!";

    case ETE_LOAD_HEADER1:
	return "Error loading container: could not read primary header.";

    case ETE_LOAD_HEADER1_SIGNATURE:
	return "Error loading container: could not read primary header, invalid signature.";

    case ETE_LOAD_HEADER1_VERSION:
	return "Error loading container: could not read primary header, invalid version.";

    case ETE_LOAD_HEADER1_METADATA:
	return "Error loading container: could not read primary header, invalid metadata.";

    case ETE_LOAD_HEADER1_METADATA_PARSE:
	return "Error loading container: could not read primary header, metadata parse failed.";

    case ETE_LOAD_HEADER2:
	return "Error loading container: could not read key slots header.";

    case ETE_LOAD_HEADER2_NO_KEYSLOTS:
	return "Error loading container: file contains no key slots for decryption.";

    case ETE_LOAD_HEADER2_KEYSLOTS:
	return "Error loading container: could not read key slots header, error reading key slot material.";

    case ETE_LOAD_HEADER2_INVALID_KEY:
	return "Error loading container: supplied key matches no key slot in header, check password.";

    case ETE_LOAD_HEADER3:
	return "Error loading container: could not read data header.";

    case ETE_LOAD_HEADER3_ENCRYPTION:
	return "Error loading container: could not read data header, check encryption key.";

    case ETE_LOAD_HEADER3_METADATA:
	return "Error loading container: could not read data header, invalid metadata.";

    case ETE_LOAD_HEADER3_METADATA_CHECKSUM:
	return "Error loading container: could not read data header, metadata CRC32 checksum mismatch.";

    case ETE_LOAD_HEADER3_METADATA_PARSE:
	return "Error loading container: could not read data header, metadata parse failed.";

    case ETE_LOAD_SUBFILE:
	return "Error loading container: could not read encrypted subfile data.";

    case ETE_LOAD_CHECKSUM:
	return "Error loading container: CRC32 checksum mismatch, file possibly corrupt.";

    case ETE_KEYSLOT_INVALID_INDEX:
	return "Invalid encryption key slot index.";

    case ETE_SUBFILE_INVALID_INDEX:
	return "Invalid subfile index.";

    case ETE_SUBFILE_CHECKSUM:
	return "Error in subfile: CRC32 checksum mismatch, data possibly corrupt.";

    case ETE_SUBFILE_INVALID_COMPRESSION:
	return "Unknown subfile compression algorithm.";

    case ETE_SUBFILE_INVALID_ENCRYPTION:
	return "Unknown subfile encryption cipher.";

    case ETE_Z_UNKNOWN:
	return "Error in zlib: unknown error.";

    case ETE_Z_OK:
	return "Error in zlib: success.";

    case ETE_Z_NEED_DICT:
	return "Error in zlib: need dictionary.";
	
    case ETE_Z_STREAM_END:
	return "Error in zlib: stream end.";

    case ETE_Z_ERRNO:
	return "Error in zlib: file error.";

    case ETE_Z_STREAM_ERROR:
	return "Error in zlib: stream error.";

    case ETE_Z_DATA_ERROR:
	return "Error in zlib: data error.";

    case ETE_Z_MEM_ERROR:
	return "Error in zlib: insufficient memory.";

    case ETE_Z_BUF_ERROR:
	return "Error in zlib: buffer error.";

    case ETE_Z_VERSION_ERROR:
	return "Error in zlib: incompatible version.";
    }

    return "Unknown error code.";
}

static error_type ErrorCodeFromZLibError(int ret)
{
    switch(ret)
    {
    default:			return ETE_Z_UNKNOWN;
    case Z_NEED_DICT:		return ETE_Z_NEED_DICT;
    case Z_STREAM_END:		return ETE_Z_STREAM_END;
    case Z_OK:			return ETE_Z_OK;
    case Z_ERRNO:		return ETE_Z_ERRNO;
    case Z_STREAM_ERROR:	return ETE_Z_STREAM_ERROR;
    case Z_DATA_ERROR:		return ETE_Z_DATA_ERROR;
    case Z_MEM_ERROR:		return ETE_Z_MEM_ERROR;
    case Z_BUF_ERROR:		return ETE_Z_BUF_ERROR;
    case Z_VERSION_ERROR:	return ETE_Z_VERSION_ERROR;
    }
}

// *** Utilities Operations ***

static inline uint32_t random_iterations()
{
    uint16_t iterations;
    Botan::Global_RNG::randomize((Botan::byte*)&iterations, sizeof(iterations));
    return (iterations % 10000) + 1000;
}

static inline uint32_t botan_crc32(Botan::byte* data, uint32_t datalen)
{
    Botan::SecureVector<Botan::byte> crc = Botan::CRC32().process(data, datalen);
    AssertException(crc.size() == 4);
    return *(uint32_t*)crc.begin();
}

// *** Botan Library Helpers ***

class DataSinkSecureVector : public Botan::DataSink
{
private:
    Botan::SecureVector<Botan::byte>&	secmem;

public:
    DataSinkSecureVector(Botan::SecureVector<Botan::byte>& _secmem)
	: secmem(_secmem)
    { }

    void write(const Botan::byte out[], Botan::u32bit length)
    {
	secmem.append(out, length);
    }
};

class DataSink2DataOutput : public Botan::DataSink
{
private:
    DataOutput&		dataout;
    uint32_t		written;
    ProgressTicker&	progress;

public:
    DataSink2DataOutput(DataOutput& _do, ProgressTicker& _progress)
	: dataout(_do), written(0), progress(_progress)
    { }

    void write(const Botan::byte out[], Botan::u32bit length)
    {
	if (!dataout.Output(out, length))
	    throw(RuntimeError(ETE_OUTPUT_ERROR));

	written += length;
	progress.Add(length);
    }

    uint32_t get_written() const { return written; }
};

class NullBufferingFilter : public Botan::Buffering_Filter
{
public:
    const unsigned int block_size;

    NullBufferingFilter(uint32_t blocksize)
	: Buffering_Filter(blocksize), block_size(blocksize)
    { }

protected:
    virtual void main_block(const Botan::byte input[])
    {
	send(input, block_size);
    }
    virtual void final_block(const Botan::byte input[], Botan::u32bit length)
    {
	send(input, length);
    }
};

// *** DataOutput and DataInput Hashing Helpers ***

class DataOutputHashChain : public DataOutput
{
public:
    /// DataOutput to received data
    DataOutput&			chain;

    /// Hash object to update
    Botan::HashFunction&	hash;

    DataOutputHashChain(DataOutput& c, Botan::HashFunction& h)
	: chain(c), hash(h)
    { }

    bool Output(const void* data, size_t datalen)
    {
	hash.update((const Botan::byte*)data, datalen);
	return chain.Output(data, datalen);
    }
};

class DataInputHashChain : public DataInput
{
public:
    /// DataInput to received data from
    DataInput&			chain;

    /// Hash object to update
    Botan::HashFunction&	hash;

    DataInputHashChain(DataInput& c, Botan::HashFunction& h)
	: chain(c), hash(h)
    { }

    unsigned int Input(void* data, size_t maxlen)
    {
	unsigned int rb = chain.Input(data, maxlen);
	hash.update((Botan::byte*)data, rb);
	return rb;
    }
};

// *** Load/Save Operations ***

void ContainerImpl::Save(DataOutput& dataout_)
{
    written = 0;

    if (keyslots.empty())
	throw(ProgramError(ETE_SAVE_NO_KEYSLOTS));

    // Estimate amount of data written to file

    size_t subfiletotal = 0;

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	subfiletotal += subfiles[si].storagesize;
    }

    size_t esttotal = sizeof(Header1)
	+ 100 // estimate for unencrypted metadata
	+ sizeof(Header2)
	+ keyslots.size() * sizeof(KeySlot)
	+ sizeof(Header3)
	+ subfiles.size() * 50 // estimate for encrypted metadata
	+ subfiletotal;

    ProgressTicker progress(*this,
			    "Saving Container", PI_SAVE_CONTAINER,
			    0, esttotal);

    Botan::CRC32 crc32all;
    DataOutputHashChain dataout(dataout_, crc32all);

    // Write out unencrypted fixed Header1 and unencrypted metadata
    {
	// Prepare variable metadata header containing unencrypted global
	// properties.
	ByteBuffer unc_metadata;

	unc_metadata.put<unsigned int>(unc_properties.size());

	for (propertymap_type::const_iterator pi = unc_properties.begin();
	     pi != unc_properties.end(); ++pi)
	{
	    unc_metadata.put<std::string>(pi->first);
	    unc_metadata.put<std::string>(pi->second);
	}

	struct Header1 header1;
	memcpy(header1.signature, fsignature, 8);
	header1.version = 0x00010000;
	header1.unc_metalen = unc_metadata.size();

	if (!dataout.Output(&header1, sizeof(header1)))
	    throw(RuntimeError(ETE_OUTPUT_ERROR));

	if (!dataout.Output(unc_metadata.data(), unc_metadata.size()))
	    throw(RuntimeError(ETE_OUTPUT_ERROR));

	written += sizeof(header1) + unc_metadata.size();
	progress.Update(written);
    }

    // Write out master key digest and all key slots
    {
	header2.keyslots = keyslots.size();

	if (!dataout.Output(&header2, sizeof(header2)))
	    throw(RuntimeError(ETE_OUTPUT_ERROR));

	for (unsigned int i = 0; i < keyslots.size(); ++i)
	{
	    if (!dataout.Output(&keyslots[i], sizeof(keyslots[i])))
		throw(RuntimeError(ETE_OUTPUT_ERROR));
	}
    }

    // Prepare variable metadata header containing global properties and all
    // subfile properties.
    ByteBuffer metadata;

    // append global properties
    metadata.put<unsigned int>(enc_properties.size());

    for (propertymap_type::const_iterator pi = enc_properties.begin();
	 pi != enc_properties.end(); ++pi)
    {
	metadata.put<std::string>(pi->first);
	metadata.put<std::string>(pi->second);
    }

    // append subfile metadata
    metadata.put<unsigned int>(subfiles.size());

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	// fixed structure
	metadata.put<unsigned int>(subfiles[si].storagesize);
	metadata.put<unsigned int>(subfiles[si].realsize);
	metadata.put<unsigned int>(subfiles[si].flags);
	metadata.put<unsigned int>(subfiles[si].crc32);
	metadata.append(subfiles[si].enckey.begin(), 32);
	metadata.append(subfiles[si].cbciv.begin(), 16);

	// variable properties structure
	metadata.put<unsigned int>(subfiles[si].properties.size());

	for (propertymap_type::const_iterator pi = subfiles[si].properties.begin();
	     pi != subfiles[si].properties.end(); ++pi)
	{
	    metadata.put<std::string>(pi->first);
	    metadata.put<std::string>(pi->second);
	}
    }

    // Compress encrypted global properties using zlib
    ByteBuffer metadata_compressed;
    {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = deflateInit(&zs, 9);
	if (ret != Z_OK)
	    throw(InternalError( ErrorCodeFromZLibError(ret) ));

	zs.next_in = metadata.data();
	zs.avail_in = metadata.size();

	char buffer[65536];

	while (ret == Z_OK)
	{
	    zs.next_out = (Bytef*)buffer;
	    zs.avail_out = sizeof(buffer);

	    ret = deflate(&zs, Z_FINISH);

	    if (metadata_compressed.size() < zs.total_out)
	    {
		metadata_compressed.append(buffer,
					   zs.total_out - metadata_compressed.size());
	    }
	}

	deflateEnd(&zs);

	if (ret != Z_STREAM_END)
	    throw(InternalError( ErrorCodeFromZLibError(ret) ));

	// append zeros to make output length a multiple of 16
	metadata_compressed.align(16);
    }

    // Encrypt fixed header3 and variable metadata
    {
	struct Header3 header3;
	// memset(&header3, 0, sizeof(header3));

	header3.test123 = 0x12345678;
	header3.metacomplen = metadata_compressed.size();
	header3.metacrc32 = botan_crc32(metadata.data(), metadata.size());

	Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

	pbkdf.set_iterations(header2.mkk_iterations);
	pbkdf.change_salt(header2.mkk_salt, sizeof(header2.mkk_salt));
	Botan::OctetString enckey = pbkdf.derive_key(32, masterkey);

	pbkdf.set_iterations(header2.mki_iterations);
	pbkdf.change_salt(header2.mki_salt, sizeof(header2.mki_salt));
	Botan::OctetString enciv = pbkdf.derive_key(16, masterkey);

	DataSink2DataOutput* datasink;

	Botan::Pipe pipe(
	    Botan::get_cipher("Serpent/CBC/NoPadding", enckey, enciv, Botan::ENCRYPTION),
	    new NullBufferingFilter(4096),
	    (datasink = new DataSink2DataOutput(dataout, progress))
	    );

	pipe.process_msg((Botan::byte*)&header3, sizeof(header3));

	pipe.process_msg(metadata_compressed.data(), metadata_compressed.size());

	written += datasink->get_written();
    }

    // Refine file target size because it is now exactly known.
    esttotal = written + subfiletotal;
    progress.Restart("Saving Container", PI_SAVE_CONTAINER, written, esttotal);

    // Output data of all subfiles simply concatenated

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	AssertException(subfiles[si].storagesize == subfiles[si].data.size());

	if (!dataout.Output(subfiles[si].data.begin(), subfiles[si].data.size()))
	    throw(RuntimeError(ETE_OUTPUT_ERROR));

	written += subfiles[si].storagesize;

	progress.Update(written);
    }

    // Append 4 bytes CRC32 value at end of file
    {
	Botan::SecureVector<Botan::byte> crc32val = crc32all.final();
	AssertException(crc32val.size() == 4);
    
	if (!dataout.Output(crc32val.begin(), crc32val.size()))
	    throw(RuntimeError(ETE_OUTPUT_ERROR));

	written += 4;
    }
}

void ContainerImpl::Load(DataInput& datain, const std::string& userkey)
{
    ProgressTicker progress(*this,
			    "Loading Container", PI_LOAD_CONTAINER,
			    0, 10240);

    // Read unencrypted fixed Header1
    struct Header1 header1;

    if (datain.Input(&header1, sizeof(header1)) != sizeof(header1))
	throw(RuntimeError(ETE_LOAD_HEADER1));

    if (memcmp(header1.signature, fsignature, 8) != 0)
	throw(RuntimeError(ETE_LOAD_HEADER1_SIGNATURE));

    if (header1.version == 0x00010000) {
	Loadv00010000(datain, userkey, header1, progress);
    }
    else {
	throw(RuntimeError(ETE_LOAD_HEADER1_VERSION));
    }
}

void ContainerImpl::Loadv00010000(DataInput& datain_, const std::string& userkey, const Header1& header1, class ProgressTicker& progress)
{
    unsigned int readbyte = sizeof(Header1);

    Botan::CRC32 crc32all;
    crc32all.update((Botan::byte*)&header1, sizeof(header1)); // update hash with already read header bytes.

    DataInputHashChain datain(datain_, crc32all);

    // Read unencrypted metadata length
    {
	ByteBuffer unc_metadata;
	unc_metadata.alloc(header1.unc_metalen);

	if (datain.Input(unc_metadata.data(), header1.unc_metalen) != header1.unc_metalen)
	    throw(RuntimeError(ETE_LOAD_HEADER1_METADATA));

	readbyte += header1.unc_metalen;
	unc_metadata.set_size(header1.unc_metalen);

	try
	{
	    // parse global unencrypted properties
	    unsigned int gpropsize = unc_metadata.get<unsigned int>();
	    unc_properties.clear();

	    for (unsigned int pi = 0; pi < gpropsize; ++pi)
	    {
		std::string key = unc_metadata.get<std::string>();
		std::string val = unc_metadata.get<std::string>();

		unc_properties.insert( propertymap_type::value_type(key, val) );
	    }
	}
	catch (std::underflow_error& e)
	{
	    throw(RuntimeError(ETE_LOAD_HEADER1_METADATA_PARSE));
	}
    }

    progress.Update(readbyte);

    // Read master key digest and user key slots
    {
	if (datain.Input(&header2, sizeof(header2)) != sizeof(header2))
	    throw(RuntimeError(ETE_LOAD_HEADER2));
	
	if (header2.keyslots == 0)
	    throw(RuntimeError(ETE_LOAD_HEADER2_NO_KEYSLOTS));

	for (unsigned int i = 0; i < header2.keyslots; ++i)
	{
	    keyslots.push_back(KeySlot());
	    KeySlot& newkeyslot = keyslots.back();

	    if (datain.Input(&newkeyslot, sizeof(newkeyslot)) != sizeof(newkeyslot))
		throw(RuntimeError(ETE_LOAD_HEADER2_KEYSLOTS));
	}

	readbyte += sizeof(header2) + header2.keyslots * sizeof(KeySlot);
    }

    progress.Update(readbyte);

    // Try to retrieve master key by checking all user key slots
    {
	Botan::SecureVector<Botan::byte> userkeyvector((Botan::byte*)userkey.data(), userkey.size());
	unsigned int ks;

	for (ks = 0; ks < header2.keyslots; ++ks)
	{
	    KeySlot& keyslot = keyslots[ks];

	    // calculate user encryption key from password

	    Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

	    pbkdf.set_iterations(keyslot.iterations);
	    pbkdf.change_salt(keyslot.salt, sizeof(keyslot.salt));
	    Botan::OctetString enckey = pbkdf.derive_key(32, userkeyvector);

	    // decrypt master key copy

	    Botan::Pipe pipe( Botan::get_cipher("Serpent/ECB/NoPadding", enckey, Botan::DECRYPTION) );

	    pipe.process_msg((Botan::byte*)keyslot.emasterkey, sizeof(keyslot.emasterkey));

	    Botan::SecureVector<Botan::byte> testmasterkey = pipe.read_all();
	    AssertException(testmasterkey.size() == masterkey.size());

	    // digest master key and compare to stored digest

	    pbkdf.set_iterations(header2.mkd_iterations);
	    pbkdf.change_salt(header2.mkd_salt, sizeof(header2.mkd_salt));
	    Botan::OctetString digest = pbkdf.derive_key(32, testmasterkey);

	    AssertException(digest.length() == sizeof(header2.mkd_digest));

	    if (Botan::same_mem(digest.begin(), header2.mkd_digest, sizeof(header2.mkd_digest)))
	    {
		usedkeyslot = ks;
		masterkey.set(testmasterkey.begin(), testmasterkey.size());
		break;
	    }
	}

	// User supplied key does not match any slot.
	if (ks >= header2.keyslots)
	    throw(RuntimeError(ETE_LOAD_HEADER2_INVALID_KEY));
    }

    // Read encrypted fixed Header3
    struct Header3 header3;

    if (datain.Input(&header3, sizeof(header3)) != sizeof(header3))
	throw(RuntimeError(ETE_LOAD_HEADER3));

    readbyte += sizeof(header3);

    // Attempt to decrypt Header3

    Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

    pbkdf.set_iterations(header2.mkk_iterations);
    pbkdf.change_salt(header2.mkk_salt, sizeof(header2.mkk_salt));
    Botan::OctetString enckey = pbkdf.derive_key(32, masterkey);

    pbkdf.set_iterations(header2.mki_iterations);
    pbkdf.change_salt(header2.mki_salt, sizeof(header2.mki_salt));
    Botan::OctetString enciv = pbkdf.derive_key(16, masterkey);

    Botan::Pipe pipe( Botan::get_cipher("Serpent/CBC/NoPadding", enckey, enciv, Botan::DECRYPTION) );

    pipe.process_msg((Botan::byte*)&header3, sizeof(header3));

    if (pipe.read((Botan::byte*)&header3, sizeof(header3)) != sizeof(header3))
	throw(RuntimeError(ETE_LOAD_HEADER3_ENCRYPTION));

    if (header3.test123 != 0x12345678)
	throw(RuntimeError(ETE_LOAD_HEADER3_ENCRYPTION));

    // Read compressed variable length metadata

    ByteBuffer metadata_compressed;
    metadata_compressed.alloc(header3.metacomplen);

    if (datain.Input(metadata_compressed.data(), header3.metacomplen) != header3.metacomplen)
	throw(RuntimeError(ETE_LOAD_HEADER3_METADATA));

    readbyte += header3.metacomplen;
    metadata_compressed.set_size(header3.metacomplen);

    pipe.process_msg(metadata_compressed.data(), metadata_compressed.size());

    if (pipe.read(metadata_compressed.data(), metadata_compressed.size(), 1) != metadata_compressed.size())
	throw(RuntimeError(ETE_LOAD_HEADER3_ENCRYPTION));

    progress.Restart("Loading Container", PI_LOAD_CONTAINER,
		     readbyte, readbyte * 10);

    // Decompress variable encrypted metadata

    ByteBuffer metadata;

    {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = inflateInit(&zs);
	if (ret != Z_OK)
	    throw(RuntimeError( ErrorCodeFromZLibError(ret) ));

	zs.next_in = metadata_compressed.data();
	zs.avail_in = metadata_compressed.size();

	char buffer[65536];

	while (ret == Z_OK)
	{
	    zs.next_out = (Bytef*)buffer;
	    zs.avail_out = sizeof(buffer);

	    ret = inflate(&zs, 0);

	    if (metadata.size() < zs.total_out) {
		metadata.append(buffer, zs.total_out - metadata.size());
	    }
	}

	if (ret != Z_STREAM_END)
	    throw(RuntimeError( ErrorCodeFromZLibError(ret) ));

	inflateEnd(&zs);

	uint32_t testcrc32 = botan_crc32(metadata.data(), metadata.size());
	if (testcrc32 != header3.metacrc32)
	    throw(RuntimeError(ETE_LOAD_HEADER3_METADATA_CHECKSUM));
    }

    try
    {
	// parse global encrypted properties
	unsigned int gpropsize = metadata.get<unsigned int>();
	enc_properties.clear();

	for (unsigned int pi = 0; pi < gpropsize; ++pi)
	{
	    std::string key = metadata.get<std::string>();
	    std::string val = metadata.get<std::string>();

	    enc_properties.insert( propertymap_type::value_type(key, val) );
	}

	// parse subfile metadata
	unsigned int subfilenum = metadata.get<unsigned int>();
	subfiles.clear();

	for (unsigned int si = 0; si < subfilenum; ++si)
	{
	    subfiles.push_back(SubFile());
	    SubFile& subfile = subfiles.back();

	    // fixed structure
	    subfile.storagesize = metadata.get<unsigned int>();
	    subfile.realsize = metadata.get<unsigned int>();
	    subfile.flags = metadata.get<unsigned int>();
	    subfile.crc32 = metadata.get<unsigned int>();
	    metadata.get(subfile.enckey.begin(), 32);
	    metadata.get(subfile.cbciv.begin(), 16);

	    // variable properties structure
	    unsigned int fpropsize = metadata.get<unsigned int>();

	    for (unsigned int pi = 0; pi < fpropsize; ++pi)
	    {
		std::string key = metadata.get<std::string>();
		std::string val = metadata.get<std::string>();

		subfile.properties.insert( propertymap_type::value_type(key, val) );
	    }
	}
    }
    catch (std::underflow_error& e)
    {
	throw(InternalError(ETE_LOAD_HEADER3_METADATA_PARSE));
    }

    // Now we can say how large the file actually is.

    size_t subfiletotal = 0;

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	subfiletotal += subfiles[si].storagesize;
    }

    progress.Restart("Loading Container", PI_LOAD_CONTAINER,
		     readbyte, readbyte + subfiletotal);

    // load data of all subfiles which are simply concatenated

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	SubFile& subfile = subfiles[si];

	subfile.data.create( subfile.storagesize );
	unsigned int rb = datain.Input(subfile.data.begin(), subfile.storagesize);

	if (rb != subfile.storagesize)
	    throw(RuntimeError(ETE_LOAD_SUBFILE));

	readbyte += rb;
	progress.Update(readbyte);
    }

    // check overall CRC32 value at end
    {
	Botan::SecureVector<Botan::byte> crc32val = crc32all.final();
	AssertException(crc32val.size() == 4);

	uint32_t crc32file;
	if (datain.Input(&crc32file, sizeof(crc32file)) != sizeof(crc32file))
	    throw(RuntimeError(ETE_LOAD_CHECKSUM));

	if (crc32file != *(uint32_t*)crc32val.begin())
	    throw(RuntimeError(ETE_LOAD_CHECKSUM));
    }
}

// *** Container Info Operations ***

size_t ContainerImpl::GetLastWritten() const
{
    return written;
}

void ContainerImpl::SetProgressIndicator(ProgressIndicator* pi)
{
    progressindicator = pi;
}

// *** Container User KeySlots Operations ***

unsigned int ContainerImpl::CountKeySlots() const
{
    return keyslots.size();
}

unsigned int ContainerImpl::AddKeySlot(const std::string& key)
{
    if (keyslots.empty())
    {
	// Generate new master key and salt/iter for digest, enckey and enciv.

	Botan::Global_RNG::randomize(masterkey.begin(), masterkey.size());

	header2.mkd_iterations = random_iterations();
	Botan::Global_RNG::randomize(header2.mkd_salt, sizeof(header2.mkd_salt));

	header2.mkk_iterations = random_iterations();
	Botan::Global_RNG::randomize(header2.mkk_salt, sizeof(header2.mkk_salt));

	header2.mki_iterations = random_iterations();
	Botan::Global_RNG::randomize(header2.mki_salt, sizeof(header2.mki_salt));

	// Calculate master key digest

	Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

	pbkdf.set_iterations(header2.mkd_iterations);
	pbkdf.change_salt(header2.mkd_salt, sizeof(header2.mkd_salt));
	Botan::OctetString digest = pbkdf.derive_key(32, masterkey);

	memcpy(header2.mkd_digest, digest.begin(), sizeof(header2.mkd_digest));
    }

    // Create cipher key from new random salt and iterations.

    KeySlot newslot;

    Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

    newslot.iterations = random_iterations();
    Botan::Global_RNG::randomize(newslot.salt, sizeof(newslot.salt));

    pbkdf.set_iterations(newslot.iterations);
    pbkdf.change_salt(newslot.salt, sizeof(newslot.salt));
    Botan::OctetString enckey = pbkdf.derive_key(32, Botan::SecureVector<Botan::byte>((Botan::byte*)key.data(), key.size()));

    // Encrypt the known (or new) master key material with the derived key.

    Botan::Pipe pipe( Botan::get_cipher("Serpent/ECB/NoPadding", enckey, Botan::ENCRYPTION) );

    pipe.process_msg(masterkey);

    Botan::SecureVector<Botan::byte> ciphertext = pipe.read_all();
    AssertException(ciphertext.size() == sizeof(newslot.emasterkey));

    memcpy(newslot.emasterkey, ciphertext.begin(), sizeof(newslot.emasterkey));
    
    keyslots.push_back(newslot);

    return keyslots.size() - 1;
}

void ContainerImpl::ChangeKeySlot(unsigned int slot, const std::string& key)
{
    // Check that the user key slot is valid.
    if (slot >= keyslots.size())
	throw(ProgramError(ETE_KEYSLOT_INVALID_INDEX));

    KeySlot& keyslot = keyslots[slot];

    // Create cipher key from new random salt and iterations.

    Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

    keyslot.iterations = random_iterations();
    Botan::Global_RNG::randomize(keyslot.salt, sizeof(keyslot.salt));

    pbkdf.set_iterations(keyslot.iterations);
    pbkdf.change_salt(keyslot.salt, sizeof(keyslot.salt));
    Botan::OctetString enckey = pbkdf.derive_key(32, Botan::SecureVector<Botan::byte>((Botan::byte*)key.data(), key.size()));

    // Encrypt the known master key material with the derived key.

    Botan::Pipe pipe( Botan::get_cipher("Serpent/ECB/NoPadding", enckey, Botan::ENCRYPTION) );

    pipe.process_msg(masterkey);

    Botan::SecureVector<Botan::byte> ciphertext = pipe.read_all();
    AssertException(ciphertext.size() == sizeof(keyslot.emasterkey));

    memcpy(keyslot.emasterkey, ciphertext.begin(), sizeof(keyslot.emasterkey));
}

void ContainerImpl::DeleteKeySlot(unsigned int slot)
{
    // Check that the user key slot is valid.
    if (slot >= keyslots.size())
	throw(ProgramError(ETE_KEYSLOT_INVALID_INDEX));

    // fixup numbering of used key slot
    if (usedkeyslot == (int)slot)
	usedkeyslot = -1;
    else if (usedkeyslot > (int)slot)
	--usedkeyslot;

    keyslots.erase(keyslots.begin() + slot);
}

int ContainerImpl::GetUsedKeySlot() const
{
    return usedkeyslot;
}

// *** Container Global Unencrypted Properties ***

void ContainerImpl::SetGlobalUnencryptedProperty(const std::string& key, const std::string& value)
{
    unc_properties[key] = value;
}

const std::string& ContainerImpl::GetGlobalUnencryptedProperty(const std::string& key) const
{
    propertymap_type::const_iterator pi = unc_properties.find(key);

    if (pi != unc_properties.end()) {
	return pi->second;
    }
    else {
	static const std::string zerostring;
	return zerostring;
    }
}

bool ContainerImpl::DeleteGlobalUnencryptedProperty(const std::string& key)
{
    return (unc_properties.erase(key) > 0);
}

bool ContainerImpl::GetGlobalUnencryptedPropertyIndex(unsigned int propindex, std::string& key, std::string& value) const
{
    if (propindex >= unc_properties.size()) return false;

    propertymap_type::const_iterator pi = unc_properties.begin();

    for(unsigned int i = 0; i < propindex; ++i)	++pi;

    key = pi->first;
    value = pi->second;

    return true;
}

// *** Container Global Encrypted Properties ***

void ContainerImpl::SetGlobalEncryptedProperty(const std::string& key, const std::string& value)
{
    enc_properties[key] = value;
}

const std::string& ContainerImpl::GetGlobalEncryptedProperty(const std::string& key) const
{
    propertymap_type::const_iterator pi = enc_properties.find(key);

    if (pi != enc_properties.end()) {
	return pi->second;
    }
    else {
	static const std::string zerostring;
	return zerostring;
    }
}

bool ContainerImpl::DeleteGlobalEncryptedProperty(const std::string& key)
{
    return (enc_properties.erase(key) > 0);
}

bool ContainerImpl::GetGlobalEncryptedPropertyIndex(unsigned int propindex, std::string& key, std::string& value) const
{
    if (propindex >= enc_properties.size()) return false;

    propertymap_type::const_iterator pi = enc_properties.begin();

    for(unsigned int i = 0; i < propindex; ++i)	++pi;

    key = pi->first;
    value = pi->second;

    return true;
}

// *** Container SubFiles - Subfile array management ***

unsigned int ContainerImpl::CountSubFile() const
{
    return subfiles.size();
}

unsigned int ContainerImpl::AppendSubFile()
{
    unsigned int si = subfiles.size();
    subfiles.push_back( SubFile() );
    return si;
}

unsigned int ContainerImpl::InsertSubFile(unsigned int subfileindex)
{
    if (subfileindex < subfiles.size())
    {
	subfiles.insert(subfiles.begin() + subfileindex, SubFile());
	return subfileindex;
    }
    else
    {
	return AppendSubFile();
    }
}

bool ContainerImpl::DeleteSubFile(unsigned int subfileindex)
{
    if (subfileindex < subfiles.size())
    {
	subfiles.erase(subfiles.begin() + subfileindex);
	return true;
    }
    else
    {
	return false;
    }
}

// *** Container SubFiles - Subfile user-defined properties ***

void ContainerImpl::SetSubFileProperty(unsigned int subfileindex, const std::string& key, const std::string& value)
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    SubFile& subfile = subfiles[subfileindex];
    subfile.properties[ key ] = value;
}

const std::string& ContainerImpl::GetSubFileProperty(unsigned int subfileindex, const std::string& key) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    const SubFile& subfile = subfiles[subfileindex];
    propertymap_type::const_iterator pi = subfile.properties.find(key);

    if (pi != subfile.properties.end()) {
	return pi->second;
    }
    else {
	static const std::string zerostring;
	return zerostring;
    }
}

bool ContainerImpl::DeleteSubFileProperty(unsigned int subfileindex, const std::string& key)
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    SubFile& subfile = subfiles[subfileindex];
    return (subfile.properties.erase(key) > 0);
}

bool ContainerImpl::GetSubFilePropertyIndex(unsigned int subfileindex, unsigned int propindex, std::string& key, std::string& value) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    const SubFile& subfile = subfiles[subfileindex];

    if (propindex >= subfile.properties.size()) return false;

    propertymap_type::const_iterator pi = subfile.properties.begin();

    for(unsigned int i = 0; i < propindex; ++i)	++pi;

    key = pi->first;
    value = pi->second;

    return true;
}

// *** Container SubFiles - Get operations of subfile header fields ***

uint32_t ContainerImpl::GetSubFileStorageSize(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    return subfiles[subfileindex].storagesize;
}

uint32_t ContainerImpl::GetSubFileSize(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    return subfiles[subfileindex].realsize;
}

encryption_type ContainerImpl::GetSubFileEncryption(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    return (encryption_type)subfiles[subfileindex].encryption;
}

compression_type ContainerImpl::GetSubFileCompression(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    return (compression_type)subfiles[subfileindex].compression;
}

// *** Container SubFiles - Set operations of subfile header fields ***

void ContainerImpl::SetSubFileEncryption(unsigned int subfileindex, encryption_type c)
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    if (c < 0 || c > ENCRYPTION_SERPENT256)
	throw(ProgramError(ETE_SUBFILE_INVALID_ENCRYPTION));

    // reencrypt if necessary
    if (subfiles[subfileindex].encryption != c)
    {
	std::string data;

	GetSubFileData(subfileindex, data);

	subfiles[subfileindex].encryption = c;

	SetSubFileData(subfileindex, data.data(), data.size());
    }
}

void ContainerImpl::SetSubFileCompression(unsigned int subfileindex, compression_type c)
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    if (c < 0 || c > COMPRESSION_BZIP2)
	throw(ProgramError(ETE_SUBFILE_INVALID_COMPRESSION));

    // recompress if necessary
    if (subfiles[subfileindex].compression != c)
    {
	std::string data;

	GetSubFileData(subfileindex, data);

	subfiles[subfileindex].compression = c;

	SetSubFileData(subfileindex, data.data(), data.size());
    }
}

void ContainerImpl::SetSubFileCompressionEncryption(unsigned int subfileindex, compression_type comp, encryption_type enc)
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    if (comp < 0 || comp > COMPRESSION_BZIP2)
	throw(ProgramError(ETE_SUBFILE_INVALID_COMPRESSION));

    if (enc < 0 || enc > ENCRYPTION_SERPENT256)
	throw(ProgramError(ETE_SUBFILE_INVALID_ENCRYPTION));

    // reencrypt and recompress if necessary
    if (subfiles[subfileindex].encryption != enc || subfiles[subfileindex].compression != comp)
    {
	std::string data;

	GetSubFileData(subfileindex, data);

	subfiles[subfileindex].encryption = enc;
	subfiles[subfileindex].compression = comp;

	SetSubFileData(subfileindex, data.data(), data.size());
    }
}

// *** Container SubFiles - Subfile data operations ***

void ContainerImpl::SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen)
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    SubFile& subfile = subfiles[subfileindex];

    subfile.realsize = datalen;

    ProgressTicker progress(*this,
			    "Saving SubFile", PI_SAVE_SUBFILE,
			    0, datalen);

    // Setup encryption context if requested

    std::auto_ptr<Botan::Filter> encryption_filter;

    if (subfile.encryption == ENCRYPTION_NONE)
    {
    }
    else if (subfile.encryption == ENCRYPTION_SERPENT256)
    {
	// Generate a new encryption ley and CBC IV and save it in the encrypted metadata

	Botan::Global_RNG::randomize(subfile.enckey.begin(), subfile.enckey.size());
	Botan::Global_RNG::randomize(subfile.cbciv.begin(), subfile.cbciv.size());

	encryption_filter = std::auto_ptr<Botan::Filter>(
	    Botan::get_cipher("Serpent/CBC/PKCS7", subfile.enckey, subfile.cbciv, Botan::ENCRYPTION)
	    );
    }
    else
    {
	throw(RuntimeError(ETE_SUBFILE_INVALID_ENCRYPTION));
    }

    // Setup compression context if requested

    std::auto_ptr<Botan::Filter> compression_filter;

    if (subfile.compression == COMPRESSION_NONE)
    {
    }
    else if (subfile.compression == COMPRESSION_ZLIB)
    {
	compression_filter =  std::auto_ptr<Botan::Filter>(
	    new Botan::Zlib_Compression(9)
	    );
    }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	compression_filter =  std::auto_ptr<Botan::Filter>(
	    new Botan::Bzip_Compression(9)
	    );
    }
    else
    {
	throw(RuntimeError(ETE_SUBFILE_INVALID_COMPRESSION));
    }

    // Setup processing pipe

    subfile.data.create(datalen / 16);	// this allocates memory
    subfile.data.create(0);		// this clears the append cursor

    // Pipe setup note: the encryption filter is chained to the _first_ filter
    // in the preceding fork!
    Botan::Filter* filters[4] = {
	new Botan::Fork(compression_filter.release(),
			new Botan::Hash_Filter("CRC32")),
	encryption_filter.release(),
	new NullBufferingFilter( std::max<unsigned int>(4096, datalen / 10) ),
	new DataSinkSecureVector(subfile.data)
    };

    Botan::Pipe pipe(filters, 4);

    pipe.start_msg();
    for(unsigned int dataptr = 0; dataptr < datalen; dataptr += 8192)
    {
	pipe.write((const Botan::byte*)data + dataptr, std::min<unsigned int>(datalen - dataptr, 8192));
	progress.Update(dataptr);
    }
    pipe.end_msg();
    progress.Update(datalen);

    Botan::SecureVector<Botan::byte> crc32 = pipe.read_all(1);
    AssertException(crc32.size() == 4);
    subfile.crc32 = *(const uint32_t*)crc32.begin();

    subfile.storagesize = subfile.data.size();
}

struct DataOutputString : public DataOutput
{
    std::string&	str;

    DataOutputString(std::string& s)
	: str(s)
    {
    }

    virtual bool Output(const void* data, size_t datalen)
    {
	str.append(static_cast<const char*>(data), datalen);
	return true;
    }
};

void ContainerImpl::GetSubFileData(unsigned int subfileindex, std::string& outstr) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    const SubFile& subfile = subfiles[subfileindex];

    outstr.clear();
    outstr.reserve(subfile.realsize);

    DataOutputString dataout(outstr);

    GetSubFileData(subfileindex, dataout);
}

void ContainerImpl::GetSubFileData(unsigned int subfileindex, class DataOutput& dataout) const
{
    if (subfileindex >= subfiles.size())
	throw(ProgramError(ETE_SUBFILE_INVALID_INDEX));

    const SubFile& subfile = subfiles[subfileindex];

    if (subfile.data.size() == 0) return;

    AssertException(subfile.data.size() == subfile.storagesize);

    ProgressTicker progress(*this,
			    "Loading SubFile", PI_LOAD_SUBFILE,
			    0, subfile.realsize);

    // Setup decryption context if requested

    std::auto_ptr<Botan::Filter> decryption_filter;

    if (subfile.encryption == ENCRYPTION_NONE)
    {
    }
    else if (subfile.encryption == ENCRYPTION_SERPENT256)
    {
	decryption_filter = std::auto_ptr<Botan::Filter>(
	    Botan::get_cipher("Serpent/CBC/PKCS7", subfile.enckey, subfile.cbciv, Botan::DECRYPTION)
	    );
    }
    else
    {
	throw(RuntimeError(ETE_SUBFILE_INVALID_ENCRYPTION));
    }

    // Setup decompression context if requested

    std::auto_ptr<Botan::Filter> decompression_filter;

    if (subfile.compression == COMPRESSION_NONE)
    {
    }
    else if (subfile.compression == COMPRESSION_ZLIB)
    {
	decompression_filter =  std::auto_ptr<Botan::Filter>(
	    new Botan::Zlib_Decompression()
	    );
    }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	decompression_filter =  std::auto_ptr<Botan::Filter>(
	    new Botan::Bzip_Decompression()
	    );
    }
    else
    {
	throw(RuntimeError(ETE_SUBFILE_INVALID_COMPRESSION));
    }

    // Setup processing pipe

    Botan::Filter* filters[5] = {
	decryption_filter.release(),
	decompression_filter.release(),
	new Botan::Fork(NULL,
			new Botan::Hash_Filter("CRC32")),
	new NullBufferingFilter(4096),
	new DataSink2DataOutput(dataout, progress)
    };

    Botan::Pipe pipe(filters, 5);
    pipe.process_msg(subfile.data);

    Botan::SecureVector<Botan::byte> crc32v = pipe.read_all(1);
    AssertException(crc32v.size() == 4);

    uint32_t crc32 = *(const uint32_t*)crc32v.begin();
    if (subfile.crc32 != crc32)
	throw(RuntimeError(ETE_SUBFILE_CHECKSUM));
}

} // namespace internal

// *** Exception Constructors ***

Exception::Exception(error_type ec, const std::string& m)
    : ecode(ec), msg("Enctain: " + m)
{ }

RuntimeError::RuntimeError(error_type ec, const std::string& m)
    : Exception(ec, "<RuntimeError> " + m)
{
}

RuntimeError::RuntimeError(error_type ec)
    : Exception(ec, std::string("<RuntimeError> ") + Container::GetErrorString(ec))
{
}

ProgramError::ProgramError(error_type ec, const std::string& m)
    : Exception(ec, "<ProgramError> " + m)
{
}

ProgramError::ProgramError(error_type ec)
    : Exception(ec, std::string("<ProgramError> ") + Container::GetErrorString(ec))
{
}

InternalError::InternalError(error_type ec, const std::string& m)
    : Exception(ec, "<InternalError> " + m)
{
}

InternalError::InternalError(error_type ec)
    : Exception(ec, std::string("<InternalError> ") + Container::GetErrorString(ec))
{
}

// *** Pimpl Stubs for Container ***

Container::Container()
{
    pimpl = new internal::ContainerImpl();
}

Container::~Container()
{
    if (pimpl->DecReference() == 0)
	delete pimpl;
}

Container::Container(const Container &cnt)
{
    pimpl = cnt.pimpl;
    pimpl->IncReference();
}

Container& Container::operator=(const Container &cnt)
{
    if (&cnt == this) return *this;

    if (pimpl->DecReference() == 0)
	delete pimpl;
    
    pimpl = cnt.pimpl;
    pimpl->IncReference();

    return *this;
}

/* static */ void Container::SetSignature(const char* sign)
{
    internal::ContainerImpl::SetSignature(sign);
}

/* static */ const char* Container::GetErrorString(error_type e)
{
    return internal::ContainerImpl::GetErrorString(e);
}

void Container::Save(DataOutput& dataout)
{
    return pimpl->Save(dataout);
}

void Container::Load(DataInput& datain, const std::string& userkey)
{
    return pimpl->Load(datain, userkey);
}

void Container::Clear()
{
    return pimpl->Clear();
}

size_t Container::GetLastWritten() const
{
    return pimpl->GetLastWritten();
}

void Container::SetProgressIndicator(ProgressIndicator* pi)
{
    return pimpl->SetProgressIndicator(pi);
}

unsigned int Container::CountKeySlots() const
{
    return pimpl->CountKeySlots();
}

unsigned int Container::AddKeySlot(const std::string& key)
{
    return pimpl->AddKeySlot(key);
}

void Container::ChangeKeySlot(unsigned int slot, const std::string& key)
{
    return pimpl->ChangeKeySlot(slot, key);
}

void Container::DeleteKeySlot(unsigned int slot)
{
    return pimpl->DeleteKeySlot(slot);
}

int Container::GetUsedKeySlot() const
{
    return pimpl->GetUsedKeySlot();
}

void Container::SetGlobalUnencryptedProperty(const std::string& key, const std::string& value)
{
    return pimpl->SetGlobalUnencryptedProperty(key, value);
}

const std::string& Container::GetGlobalUnencryptedProperty(const std::string& key) const
{
    return pimpl->GetGlobalUnencryptedProperty(key);
}

bool Container::DeleteGlobalUnencryptedProperty(const std::string& key)
{
    return pimpl->DeleteGlobalUnencryptedProperty(key);
}

bool Container::GetGlobalUnencryptedPropertyIndex(unsigned int propindex, std::string& key, std::string& value) const
{
    return pimpl->GetGlobalUnencryptedPropertyIndex(propindex, key, value);
}

void Container::SetGlobalEncryptedProperty(const std::string& key, const std::string& value)
{
    return pimpl->SetGlobalEncryptedProperty(key, value);
}

const std::string& Container::GetGlobalEncryptedProperty(const std::string& key) const
{
    return pimpl->GetGlobalEncryptedProperty(key);
}

bool Container::DeleteGlobalEncryptedProperty(const std::string& key)
{
    return pimpl->DeleteGlobalEncryptedProperty(key);
}

bool Container::GetGlobalEncryptedPropertyIndex(unsigned int propindex, std::string& key, std::string& value) const
{
    return pimpl->GetGlobalEncryptedPropertyIndex(propindex, key, value);
}

unsigned int Container::CountSubFile() const
{
    return pimpl->CountSubFile();
}

unsigned int Container::AppendSubFile()
{
    return pimpl->AppendSubFile();
}

unsigned int Container::InsertSubFile(unsigned int subfileindex)
{
    return pimpl->InsertSubFile(subfileindex);
}

bool Container::DeleteSubFile(unsigned int subfileindex)
{
    return pimpl->DeleteSubFile(subfileindex);
}

void Container::SetSubFileProperty(unsigned int subfileindex, const std::string& key, const std::string& value)
{
    return pimpl->SetSubFileProperty(subfileindex, key, value);
}

const std::string& Container::GetSubFileProperty(unsigned int subfileindex, const std::string& key) const
{
    return pimpl->GetSubFileProperty(subfileindex, key);
}

bool Container::DeleteSubFileProperty(unsigned int subfileindex, const std::string& key)
{
    return pimpl->DeleteSubFileProperty(subfileindex, key);
}

bool Container::GetSubFilePropertyIndex(unsigned int subfileindex, unsigned int propindex, std::string& key, std::string& value) const
{
    return pimpl->GetSubFilePropertyIndex(subfileindex, propindex, key, value);
}

uint32_t Container::GetSubFileStorageSize(unsigned int subfileindex) const
{
    return pimpl->GetSubFileStorageSize(subfileindex);
}

uint32_t Container::GetSubFileSize(unsigned int subfileindex) const
{
    return pimpl->GetSubFileSize(subfileindex);
}

encryption_type Container::GetSubFileEncryption(unsigned int subfileindex) const
{
    return pimpl->GetSubFileEncryption(subfileindex);
}

compression_type Container::GetSubFileCompression(unsigned int subfileindex) const
{
    return pimpl->GetSubFileCompression(subfileindex);
}

void Container::SetSubFileEncryption(unsigned int subfileindex, encryption_type c)
{
    return pimpl->SetSubFileEncryption(subfileindex, c);
}

void Container::SetSubFileCompression(unsigned int subfileindex, compression_type c)
{
    return pimpl->SetSubFileCompression(subfileindex, c);
}

void Container::SetSubFileCompressionEncryption(unsigned int subfileindex, compression_type comp, encryption_type enc)
{
    return pimpl->SetSubFileCompressionEncryption(subfileindex, comp, enc);
}

void Container::SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen)
{
    return pimpl->SetSubFileData(subfileindex, data, datalen);
}

void Container::GetSubFileData(unsigned int subfileindex, std::string& outstr) const
{
    return pimpl->GetSubFileData(subfileindex, outstr);
}

void Container::GetSubFileData(unsigned int subfileindex, class DataOutput& dataout) const
{
    return pimpl->GetSubFileData(subfileindex, dataout);
}

} // namespace Enctain
