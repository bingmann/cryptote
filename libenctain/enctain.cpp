// $Id$

#include "enctain.h"

#include "mycrc32.h"
#include "bytebuff.h"
#include "mysha256.h"
#include "myserpent.h"

#include <vector>
#include <map>

#include <stdlib.h>
#include <time.h>
#include <zlib.h>
#include <bzlib.h>

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

	/// Encryption CBC initialization vector, if needed.
	unsigned char	cbciv[16];
    
	/// User-defined properties of the subfile.
	propertymap_type properties;

	/// Compressed and encrypted data of subfile.
	ByteBuffer	data;

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

    /// Number of references to this object
    unsigned int	references;

    /// True if one of the subfiles was changed using saveSubFile() and the
    /// container file was not saved yet.
    bool		modified;

    /// Whether the 256-bit encryption context is initialized;
    bool		iskeyset;

    /// Serpent256 keybit encryption context. It is mutable because
    /// GetSubFileData() is const and changes the CBC IV.
    mutable SerpentCBC	serpentctx;

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
    static const char*	GetErrorString(error_t e);

    // *** Load/Save Operations ***

    /// Save the current container by outputting all data to the data sink.
    error_t		Save(DataOutput& dataout);

    /// Load a new container from an input stream and parse the subfile index.
    error_t		Load(DataInput& datain, const std::string& filekey);

    /// Load a container version v1.0
    error_t		Loadv00010000(DataInput& datain, const std::string& filekey, const Header1& header1, class ProgressTicker& progress);

    /// Reset all structures in the container
    void		Clear();


    // *** Container Info and Key Operations ***

    /// Set a new password string. The string will be hashed and transformed
    /// into an encryption context. This is a very expensive operation as all
    /// subfiles need to be reencrypted.
    void		SetKey(const std::string& keystr);

    /// Checks whether a password key was set.
    bool		IsKeySet() const;

    /// Return number of bytes written to data sink during last Save()
    /// operation.
    size_t		GetLastWritten() const;

    /// Set the Progress Indicator object which receives progress notifications
    void		SetProgressIndicator(ProgressIndicator* pi);


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
    error_t		GetSubFileData(unsigned int subfileindex, class DataOutput& dataout) const;

    /// Return the data of a subfile: decrypt and uncompress it. Return
    /// complete data in a memory string.
    error_t		GetSubFileData(unsigned int subfileindex, std::string& data) const;

    /// Set/change the data of a subfile, it will be compressed and encrypted
    /// but not written to disk, yet.
    error_t		SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen);
};

// *** Constructor, Destructor and Reference Counter  ***

ContainerImpl::ContainerImpl()
    : references(1),
      modified(false),
      iskeyset(false), written(0),
      progressindicator(NULL)
{
}

ContainerImpl::~ContainerImpl()
{
    serpentctx.wipe();
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
    const ContainerImpl& 	cnt;

public:
    ProgressTicker(const ContainerImpl& c,
		   const char* pitext, progress_indicator_type pitype,
		   size_t value, size_t limit)
	: cnt(c)
    {
	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressStart(pitext, pitype, value, limit);
    }

    void Restart(const char* pitext, progress_indicator_type pitype,
		 size_t value, size_t limit)
    {
	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressStart(pitext, pitype, value, limit);
    }

    void Update(size_t value)
    {
	if (cnt.progressindicator)
	    cnt.progressindicator->ProgressUpdate(value);
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

/* static */ const char* ContainerImpl::GetErrorString(error_t e)
{
    switch(e)
    {
    case ETE_SUCCESS:
	return "Success.";

    case ETE_SAVE_NO_PASSWORD:
	return "Error saving container: no encryption password set!";

    case ETE_SAVE_OUTPUT_ERROR:
	return "Error saving container: output stream error.";

    case ETE_LOAD_HEADER1:
	return "Error loading container: could not read header.";

    case ETE_LOAD_HEADER1_SIGNATURE:
	return "Error loading container: could not read header, invalid signature.";

    case ETE_LOAD_HEADER1_VERSION:
	return "Error loading container: could not read header, invalid version.";

    case ETE_LOAD_HEADER1_METADATA:
	return "Error loading container: could not read header, invalid metadata.";

    case ETE_LOAD_HEADER1_METADATA_PARSE:
	return "Error loading container: could not read header, metadata parse failed.";

    case ETE_LOAD_HEADER2:
	return "Error loading container: could not read secondary header.";

    case ETE_LOAD_HEADER2_ENCRYPTION:
	return "Error loading container: could not read secondary header, check encryption key.";

    case ETE_LOAD_HEADER2_METADATA:
	return "Error loading container: could not read secondary header, invalid metadata.";

    case ETE_LOAD_HEADER2_METADATA_CRC32:
	return "Error loading container: could not read secondary header, metadata crc32 mismatch.";

    case ETE_LOAD_HEADER2_METADATA_PARSE:
	return "Error loading container: could not read secondary header, metadata parse failed.";

    case ETE_LOAD_SUBFILE:
	return "Error loading container: could not read encrypted subfile data.";

    case ETE_SUBFILE_COMPRESSION_INVALID:
	return "Error in subfile: unknown compression algorithm.";

    case ETE_SUBFILE_ENCRYPTION_INVALID:
	return "Error in subfile: unknown encryption cipher.";

    case ETE_SUBFILE_ENCRYPTION_LENGTH:
	return "Error in subfile: invalid encrypted data length.";

    case ETE_SUBFILE_UNEXPECTED_EOF:
	return "Error in subfile: read beyond end of stream.";

    case ETE_SUBFILE_CRC32:
	return "Error in subfile: crc32 mismatch, data possibly corrupt.";

    case ETE_SUBFILE_OUTPUT_ERROR:
	return "Error in subfile: output stream error.";

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

    case ETE_BZ_UNKNOWN:
	return "Error in bzip2: unknown error.";

    case ETE_BZ_OK:
	return "Error in bzip2: success.";

    case ETE_BZ_RUN_OK:
	return "Error in bzip2: successful run.";

    case ETE_BZ_FLUSH_OK:
	return "Error in bzip2: successful flush.";

    case ETE_BZ_FINISH_OK:
	return "Error in bzip2: successful finish.";

    case ETE_BZ_STREAM_END:
	return "Error in bzip2: stream end.";

    case ETE_BZ_SEQUENCE_ERROR:
	return "Error in bzip2: sequence error.";

    case ETE_BZ_PARAM_ERROR:
	return "Error in bzip2: parameter error.";

    case ETE_BZ_MEM_ERROR:
	return "Error in bzip2: insufficient memory.";

    case ETE_BZ_DATA_ERROR:
	return "Error in bzip2: data error.";

    case ETE_BZ_DATA_ERROR_MAGIC:
	return "Error in bzip2: magic header error.";

    case ETE_BZ_IO_ERROR:
	return "Error in bzip2: file system error.";

    case ETE_BZ_UNEXPECTED_EOF:
	return "Error in bzip2: unexpected end of file.";

    case ETE_BZ_OUTBUFF_FULL:
	return "Error in bzip2: output buffer full.";

    case ETE_BZ_CONFIG_ERROR:
	return "Error in bzip2: platform config error.";
    }

    return "Unknown error code.";
}

static error_t ErrorFromZLibError(int ret)
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

static error_t ErrorFromBZLibError(int ret)
{
    switch (ret)
    {
    default:			return ETE_BZ_UNKNOWN;
    case BZ_OK:			return ETE_BZ_OK;
    case BZ_RUN_OK:		return ETE_BZ_RUN_OK;
    case BZ_FLUSH_OK:		return ETE_BZ_FLUSH_OK;
    case BZ_FINISH_OK:		return ETE_BZ_FINISH_OK;
    case BZ_STREAM_END:		return ETE_BZ_STREAM_END;
    case BZ_SEQUENCE_ERROR:	return ETE_BZ_SEQUENCE_ERROR;
    case BZ_PARAM_ERROR:	return ETE_BZ_PARAM_ERROR;
    case BZ_MEM_ERROR:		return ETE_BZ_MEM_ERROR;
    case BZ_DATA_ERROR:		return ETE_BZ_DATA_ERROR;
    case BZ_DATA_ERROR_MAGIC:	return ETE_BZ_DATA_ERROR_MAGIC;
    case BZ_IO_ERROR:		return ETE_BZ_IO_ERROR;
    case BZ_UNEXPECTED_EOF:	return ETE_BZ_UNEXPECTED_EOF;
    case BZ_OUTBUFF_FULL:	return ETE_BZ_OUTBUFF_FULL;
    case BZ_CONFIG_ERROR:	return ETE_BZ_CONFIG_ERROR;
    }
}

// *** Load/Save Operations ***

error_t ContainerImpl::Save(DataOutput& dataout)
{
    written = 0;

    if (!iskeyset)
	return ETE_SAVE_NO_PASSWORD;

    srand(time(NULL));

    // Estimate amount of data written to file

    size_t subfiletotal = 0;

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	subfiletotal += subfiles[si].storagesize;
    }

    size_t esttotal = sizeof(Header1)
	+ 100 // estimate for unencrypted metadata
	+ sizeof(Header2)
	+ subfiles.size() * 50 // estimate for encrypted metadata
	+ subfiletotal;

    ProgressTicker progress(*this,
			    "Saving Container", PI_SAVE_CONTAINER,
			    0, esttotal);

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
	    return ETE_SAVE_OUTPUT_ERROR;

	if (!dataout.Output(unc_metadata.data(), unc_metadata.size()))
	    return ETE_SAVE_OUTPUT_ERROR;

	written += sizeof(header1) + unc_metadata.size();
	progress.Update(written);
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
    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	// fixed structure
	metadata.put<unsigned int>(subfiles[si].storagesize);
	metadata.put<unsigned int>(subfiles[si].realsize);
	metadata.put<unsigned int>(subfiles[si].flags);
	metadata.put<unsigned int>(subfiles[si].crc32);
	metadata.append(subfiles[si].cbciv, 16);

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
	    return ErrorFromZLibError(ret);

	zs.next_in = (Bytef*)metadata.data();
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
	    return ErrorFromZLibError(ret);

	// append zeros to make output length a multiple of 16
	metadata_compressed.align(16);
    }

    struct Header2 header2;
    header2.test123 = 0x12345678;
    header2.metacomplen = metadata_compressed.size();
    header2.metacrc32 = crc32((unsigned char*)metadata.data(), metadata.size());
    header2.subfilenum = subfiles.size();

    // Encrypt fixed header2 and variable metadata

    static const uint8_t header2cbciv[16] =
	{ 0x14, 0x8B, 0x13, 0x5A, 0xEF, 0xF3, 0x01, 0x2B,
	  0x8F, 0x61, 0x0A, 0xC9, 0x43, 0x22, 0x82, 0x87 };

    serpentctx.set_cbciv(header2cbciv);

    serpentctx.encrypt(&header2, sizeof(header2));
    serpentctx.encrypt(metadata_compressed.data(), metadata_compressed.size());

    if (!dataout.Output(&header2, sizeof(header2)))
	return ETE_SAVE_OUTPUT_ERROR;

    if (!dataout.Output(metadata_compressed.data(), metadata_compressed.size()))
	return ETE_SAVE_OUTPUT_ERROR;

    written += sizeof(header2) + metadata_compressed.size();

    // Refine file target size because it is now exactly known.
    esttotal = written + subfiletotal;
    progress.Restart("Saving Container", PI_SAVE_CONTAINER, written, esttotal);

    // Output data of all subfiles simply concatenated

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	assert(subfiles[si].storagesize == subfiles[si].data.size());

	if (!dataout.Output(subfiles[si].data.data(), subfiles[si].storagesize))
	    return ETE_SAVE_OUTPUT_ERROR;

	written += subfiles[si].storagesize;

	progress.Update(written);
    }

    return ETE_SUCCESS;
}

error_t ContainerImpl::Load(DataInput& datain, const std::string& filekey)
{
    ProgressTicker progress(*this,
			    "Loading Container", PI_LOAD_CONTAINER,
			    0, 1000);

    // Read unencrypted fixed Header1
    struct Header1 header1;

    if (datain.Input(&header1, sizeof(header1)) != sizeof(header1))
	return ETE_LOAD_HEADER1;

    if (memcmp(header1.signature, fsignature, 8) != 0)
	return ETE_LOAD_HEADER1_SIGNATURE;

    if (header1.version == 0x00010000) {
	error_t e = Loadv00010000(datain, filekey, header1, progress);
	return e;
    }
    else {
	return ETE_LOAD_HEADER1_VERSION;
    }
}

error_t ContainerImpl::Loadv00010000(DataInput& datain, const std::string& filekey, const Header1& header1, class ProgressTicker& progress)
{
    unsigned int readbyte = sizeof(Header1);

    // Read unencrypted metadata length
    {
	ByteBuffer unc_metadata;
	unc_metadata.alloc(header1.unc_metalen);

	if (datain.Input(unc_metadata.data(), header1.unc_metalen) != header1.unc_metalen)
	    return ETE_LOAD_HEADER1_METADATA;

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
	    return ETE_LOAD_HEADER1_METADATA_PARSE;
	}
    }

    // Read encrypted fixed Header2
    struct Header2 header2;

    if (datain.Input(&header2, sizeof(header2)) != sizeof(header2))
	return ETE_LOAD_HEADER1;

    readbyte += sizeof(header2);

    // decrypt header2

    std::string deckey = SHA256::digest( std::string(fsignature, 8) + filekey );
    assert( deckey.size() == 32 );

    static const uint8_t header2cbciv[16] =
	{ 0x14, 0x8B, 0x13, 0x5A, 0xEF, 0xF3, 0x01, 0x2B,
	  0x8F, 0x61, 0x0A, 0xC9, 0x43, 0x22, 0x82, 0x87 };

    SerpentCBC decctx;
    decctx.set_key((const uint8_t*)deckey.data(), deckey.size());
    decctx.set_cbciv(header2cbciv);

    decctx.decrypt(&header2, sizeof(header2));

    if (header2.test123 != 0x12345678)
	return ETE_LOAD_HEADER2_ENCRYPTION;

    // Read compressed variable length metadata

    ByteBuffer metadata_compressed;
    metadata_compressed.alloc(header2.metacomplen);

    if (datain.Input(metadata_compressed.data(), header2.metacomplen) != header2.metacomplen)
	return ETE_LOAD_HEADER2_METADATA;

    readbyte += header2.metacomplen;
    metadata_compressed.set_size(header2.metacomplen);

    decctx.decrypt(metadata_compressed.data(), metadata_compressed.size());

    // Decompress variable encrypted metadata

    ByteBuffer metadata;

    {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = inflateInit(&zs);
	if (ret != Z_OK)
	    return ErrorFromZLibError(ret);

	zs.next_in = (Bytef*)(metadata_compressed.data());
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
	    return ErrorFromZLibError(ret);

	inflateEnd(&zs);

	uint32_t metacrc32 = crc32((unsigned char*)metadata.data(), metadata.size());

	if (metacrc32 != header2.metacrc32)
	    return ETE_LOAD_HEADER2_METADATA_CRC32;
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
	subfiles.clear();

	for (unsigned int si = 0; si < header2.subfilenum; ++si)
	{
	    subfiles.push_back(SubFile());
	    SubFile& subfile = subfiles.back();

	    // fixed structure
	    subfile.storagesize = metadata.get<unsigned int>();
	    subfile.realsize = metadata.get<unsigned int>();
	    subfile.flags = metadata.get<unsigned int>();
	    subfile.crc32 = metadata.get<unsigned int>();
	    metadata.get(subfile.cbciv, 16);

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
	return ETE_LOAD_HEADER2_METADATA_PARSE;
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

	subfile.data.alloc( subfile.storagesize );

	unsigned int rb = datain.Input(subfile.data.data(), subfile.storagesize);
	subfile.data.set_size(rb);

	if (rb != subfile.storagesize)
	    return ETE_LOAD_SUBFILE;

	readbyte += rb;
	progress.Update(readbyte);
    }

    iskeyset = true;
    serpentctx = decctx;

    decctx.wipe();

    return ETE_SUCCESS;
}

/// Reset all structures in the container
void ContainerImpl::Clear()
{
    modified = false;
    iskeyset = false;
    serpentctx.wipe();
    unc_properties.clear();
    enc_properties.clear();
    subfiles.clear();
    written = 0;
}

// *** Container Info Operations ***

void ContainerImpl::SetKey(const std::string& keystr)
{
    std::string enckey = SHA256::digest( std::string(fsignature, 8) + keystr );

    SerpentCBC newctx;
    newctx.set_key((const uint8_t*)enckey.data(), enckey.size());

    size_t totalsize = 0;

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	SubFile& subfile = subfiles[si];
	totalsize += subfile.storagesize;
    }

    ProgressTicker progress(*this,
			    "Reencrypting", PI_REENCRYPT,
			    0, totalsize);

    // reencrypt all subfile data

    totalsize = 0;
    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	SubFile& subfile = subfiles[si];

	assert(subfile.storagesize == subfile.data.size());

	if (subfile.encryption == ENCRYPTION_NONE)
	{
	    totalsize += subfile.storagesize;
	    progress.Update(totalsize);
	}
	else if (subfile.encryption == ENCRYPTION_SERPENT256)
	{
	    if (iskeyset)
		serpentctx.set_cbciv(subfile.cbciv);

	    newctx.set_cbciv(subfile.cbciv);

	    const size_t batch = 65536;

	    for (size_t offset = 0; offset < subfile.data.size(); offset += batch)
	    {
		size_t len = std::min<size_t>(batch, subfile.data.size() - offset);

		if (iskeyset)
		    serpentctx.decrypt(subfile.data.data() + offset, len);
	
		newctx.encrypt(subfile.data.data() + offset, len);

		totalsize += len;
		progress.Update(totalsize);
	    }
	}
    }

    serpentctx = newctx;
    iskeyset = true;

    newctx.wipe();
}

bool ContainerImpl::IsKeySet() const
{
    return iskeyset;
}

size_t ContainerImpl::GetLastWritten() const
{
    return written;
}

void ContainerImpl::SetProgressIndicator(ProgressIndicator* pi)
{
    progressindicator = pi;
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

bool ContainerImpl::EraseGlobalUnencryptedProperty(const std::string& key)
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

bool ContainerImpl::EraseGlobalEncryptedProperty(const std::string& key)
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
	throw(std::runtime_error("Invalid subfile index"));

    SubFile& subfile = subfiles[subfileindex];
    subfile.properties[ key ] = value;
}

const std::string& ContainerImpl::GetSubFileProperty(unsigned int subfileindex, const std::string& key) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

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

bool ContainerImpl::EraseSubFileProperty(unsigned int subfileindex, const std::string& key)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    SubFile& subfile = subfiles[subfileindex];
    return (subfile.properties.erase(key) > 0);
}

bool ContainerImpl::GetSubFilePropertyIndex(unsigned int subfileindex, unsigned int propindex, std::string& key, std::string& value) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

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
	throw(std::runtime_error("Invalid subfile index"));

    return subfiles[subfileindex].storagesize;
}

uint32_t ContainerImpl::GetSubFileSize(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    return subfiles[subfileindex].realsize;
}

encryption_type ContainerImpl::GetSubFileEncryption(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    return (encryption_type)subfiles[subfileindex].encryption;
}

compression_type ContainerImpl::GetSubFileCompression(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    return (compression_type)subfiles[subfileindex].compression;
}

// *** Container SubFiles - Set operations of subfile header fields ***

void ContainerImpl::SetSubFileEncryption(unsigned int subfileindex, encryption_type c)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    if (c < 0 || c > ENCRYPTION_SERPENT256)
	throw(std::runtime_error("Invalid encryption cipher index"));

    // reencrypt if necessary
    if (subfiles[subfileindex].encryption != c)
    {
	std::string data;

	error_t e1 = GetSubFileData(subfileindex, data);
	if (e1 != ETE_SUCCESS)
	    throw(std::runtime_error(GetErrorString(e1)));

	subfiles[subfileindex].encryption = c;

	error_t e2 = SetSubFileData(subfileindex, data.data(), data.size());
	if (e2 != ETE_SUCCESS)
	    throw(std::runtime_error(GetErrorString(e2)));
    }
}

void ContainerImpl::SetSubFileCompression(unsigned int subfileindex, compression_type c)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    if (c < 0 || c > COMPRESSION_BZIP2)
	throw(std::runtime_error("Invalid compression algorithm index"));

    // recompress if necessary
    if (subfiles[subfileindex].compression != c)
    {
	std::string data;

	error_t e1 = GetSubFileData(subfileindex, data);
	if (e1 != ETE_SUCCESS)
	    throw(std::runtime_error(GetErrorString(e1)));

	subfiles[subfileindex].compression = c;

	error_t e2 = SetSubFileData(subfileindex, data.data(), data.size());
	if (e2 != ETE_SUCCESS)
	    throw(std::runtime_error(GetErrorString(e2)));
    }
}

void ContainerImpl::SetSubFileCompressionEncryption(unsigned int subfileindex, compression_type comp, encryption_type enc)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    if (comp < 0 || comp > COMPRESSION_BZIP2)
	throw(std::runtime_error("Invalid compression algorithm index"));

    if (enc < 0 || enc > ENCRYPTION_SERPENT256)
	throw(std::runtime_error("Invalid encryption cipher index"));

    // reencrypt and recompress if necessary
    if (subfiles[subfileindex].encryption != enc || subfiles[subfileindex].compression != comp)
    {
	std::string data;

	error_t e1 = GetSubFileData(subfileindex, data);
	if (e1 != ETE_SUCCESS)
	    throw(std::runtime_error(GetErrorString(e1)));

	subfiles[subfileindex].encryption = enc;
	subfiles[subfileindex].compression = comp;

	error_t e2 = SetSubFileData(subfileindex, data.data(), data.size());
	if (e2 != ETE_SUCCESS)
	    throw(std::runtime_error(GetErrorString(e2)));
    }
}

// *** Container SubFiles - Subfile data operations ***

error_t ContainerImpl::SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    SubFile& subfile = subfiles[subfileindex];

    subfile.realsize = datalen;

    ProgressTicker progress(*this,
			    "Saving SubFile", PI_SAVE_SUBFILE,
			    0, datalen);

    // Setup encryption context if requested

    if (subfile.encryption == ENCRYPTION_NONE)
    {
    }
    else if (subfile.encryption == ENCRYPTION_SERPENT256)
    {
	// Generate a new CBC IV and save it in the metadata

	for (unsigned int i = 0; i < 16; ++i)
	    subfile.cbciv[i] = (unsigned char)rand();

	// Prepare data block encryption
	if (iskeyset)
	{
	    serpentctx.set_cbciv(subfile.cbciv);
	}
    }
    else
    {
	return ETE_SUBFILE_ENCRYPTION_INVALID;
    }

    // Copy or compress data into the ByteBuffer

    uint32_t crc32run = 0;

    if (subfile.compression == COMPRESSION_NONE)
    {
	subfile.data.alloc(datalen);
	subfile.data.set_size(0);

	const size_t batch = 65536;
	size_t offset = 0;

	while(offset < datalen)
	{
	    size_t currlen = std::min<size_t>(batch, datalen - offset);

	    subfile.data.append((char*)data + offset, currlen);

	    crc32run = update_crc32(crc32run, (uint8_t*)data + offset, currlen);

	    if (subfile.encryption == ENCRYPTION_NONE)
	    {
	    }
	    else if (subfile.encryption == ENCRYPTION_SERPENT256)
	    {
		size_t enclen = currlen;

		// pad until length is multiple of 16, this must only
		// happen at the end of the data
		if (enclen % 16 != 0)
		{
		    while(enclen % 16 != 0) {
			subfile.data.put<char>(0);
			++enclen;
		    }
		}

		if (iskeyset)
		{
		    serpentctx.encrypt(subfile.data.data() + offset, enclen);
		}
	    }

	    offset += currlen;
	    progress.Update(offset);
	}
    }
    else if (subfile.compression == COMPRESSION_ZLIB)
    {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = deflateInit(&zs, Z_BEST_COMPRESSION);
	if (ret != Z_OK)
	    return ErrorFromZLibError(ret);

	zs.next_in = (Bytef*)(data);
	zs.avail_in = datalen;

	crc32run = update_crc32(crc32run, zs.next_in, zs.avail_in);

	const size_t batch = 65536;
	size_t offset = 0;
	size_t encoffset = 0;

	while (ret == Z_OK)
	{
	    subfile.data.alloc(offset + batch); // extend output buffer

	    zs.next_out = (Bytef*)(subfile.data.data()) + offset;
	    zs.avail_out = subfile.data.buffsize() - offset;

	    ret = deflate(&zs, Z_FINISH);

	    if (offset < zs.total_out)
	    {
		if (subfile.encryption == ENCRYPTION_NONE)
		{
		}
		else if (subfile.encryption == ENCRYPTION_SERPENT256 && iskeyset)
		{
		    size_t encsize = zs.total_out - encoffset;
		    encsize -= (encsize % 16);

		    serpentctx.encrypt(subfile.data.data() + encoffset, encsize);

		    encoffset += encsize;
		}

		offset = zs.total_out;
		progress.Update(datalen - zs.avail_in);
	    }
	}

	if (ret != Z_STREAM_END) // an error occurred that was not EOF
	    return ErrorFromZLibError(ret);

	deflateEnd(&zs);

	subfile.data.set_size(offset);

	if (subfile.encryption == ENCRYPTION_SERPENT256)
	{
	    if (encoffset < subfile.data.size())
	    {
		subfile.data.align(16);

		if (iskeyset)
		{
		    size_t encsize = subfile.data.size() - encoffset;
		    serpentctx.encrypt(subfile.data.data() + encoffset, encsize);
		}
	    }
	}
    }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	bz_stream bz;
	memset(&bz, 0, sizeof(bz));

	int ret = BZ2_bzCompressInit(&bz, 9, 0, 0);
	if (ret != BZ_OK)
	    return ErrorFromBZLibError(ret);

	bz.next_in = (char*)data;
	bz.avail_in = datalen;

	crc32run = update_crc32(crc32run, (uint8_t*)bz.next_in, bz.avail_in);

	const size_t batch = 65536;
	size_t offset = 0;
	size_t encoffset = 0;

	while (ret == BZ_OK || ret == BZ_FINISH_OK)
	{
	    subfile.data.alloc(offset + batch); // extend output buffer

	    bz.next_out = subfile.data.data() + offset;
	    bz.avail_out = subfile.data.buffsize() - offset;

	    ret = BZ2_bzCompress(&bz, BZ_FINISH);

	    if (offset < bz.total_out_lo32)
	    {
		if (subfile.encryption == ENCRYPTION_NONE)
		{
		}
		else if (subfile.encryption == ENCRYPTION_SERPENT256 && iskeyset)
		{
		    size_t encsize = bz.total_out_lo32 - encoffset;
		    encsize -= (encsize % 16);

		    serpentctx.encrypt(subfile.data.data() + encoffset, encsize);

		    encoffset += encsize;
		}

		offset = bz.total_out_lo32;
		progress.Update(datalen - bz.avail_in);
	    }
	}

	if (ret != BZ_STREAM_END)
	    return ErrorFromBZLibError(ret);

	BZ2_bzCompressEnd(&bz);

	subfile.data.set_size(offset);

	if (subfile.encryption == ENCRYPTION_SERPENT256)
	{
	    if (encoffset < subfile.data.size())
	    {
		subfile.data.align(16);

		if (iskeyset)
		{
		    size_t encsize = subfile.data.size() - encoffset;
		    serpentctx.encrypt(subfile.data.data() + encoffset, encsize);
		}
	    }
	}
    }
    else
    {
	return ETE_SUBFILE_COMPRESSION_INVALID;
    }

    subfile.crc32 = crc32run;
    subfile.storagesize = subfile.data.size();

    return ETE_SUCCESS;
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

error_t ContainerImpl::GetSubFileData(unsigned int subfileindex, std::string& outstr) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    const SubFile& subfile = subfiles[subfileindex];

    outstr.clear();
    outstr.reserve(subfile.realsize);

    DataOutputString dataout(outstr);

    return GetSubFileData(subfileindex, dataout);
}

error_t ContainerImpl::GetSubFileData(unsigned int subfileindex, class DataOutput& dataout) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    const SubFile& subfile = subfiles[subfileindex];

    if (subfile.data.size() == 0) return ETE_SUCCESS;

    assert(subfile.data.size() == subfile.storagesize);

    ProgressTicker progress(*this,
			    "Loading SubFile", PI_LOAD_SUBFILE,
			    0, subfile.storagesize);

    // Setup decryption context if requested

    if (subfile.encryption == ENCRYPTION_NONE)
    {
    }
    else if (subfile.encryption == ENCRYPTION_SERPENT256)
    {
	if (iskeyset)
	{
	    // Ensure that the data buffer has a length multiple of 16
	    if (subfile.data.size() % 16 != 0)
		return ETE_SUBFILE_ENCRYPTION_LENGTH;

	    // Prepare data block decryption
	    serpentctx.set_cbciv(subfile.cbciv);
	}
    }
    else
    {
	return ETE_SUBFILE_ENCRYPTION_INVALID;
    }

    // Copy or decompress subfile data and send output to DataOutput

    uint32_t crc32run = 0;

    if (subfile.compression == COMPRESSION_NONE)
    {
	size_t offset = 0;
	char buffer[65536];

	assert(subfile.data.size() >= subfile.realsize);

	while(offset < subfile.data.size())
	{
	    size_t currlen = std::min<size_t>(sizeof(buffer), subfile.data.size() - offset);

	    memcpy(buffer, subfile.data.data() + offset, currlen);

	    if (subfile.encryption == ENCRYPTION_NONE)
	    {
	    }
	    else if (subfile.encryption == ENCRYPTION_SERPENT256)
	    {
		if (iskeyset)
		    serpentctx.decrypt(buffer, currlen);
	    }

	    // because encrypted blocks can be padded, the real data length
	    // might be shorter than the buffer len.
	    size_t reallen = currlen;
	    if (offset + reallen > subfile.realsize) reallen = subfile.realsize - offset;

	    if (!dataout.Output(buffer, reallen))
		return ETE_SUBFILE_OUTPUT_ERROR;

	    crc32run = update_crc32(crc32run, (uint8_t*)buffer, reallen);

	    offset += currlen;
	    progress.Update(offset);
	}
    }
    else if (subfile.compression == COMPRESSION_ZLIB)
    {
	// z_stream is zlib's control structure
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = inflateInit(&zs);
	if (ret != Z_OK)
	    return ErrorFromZLibError(ret);

	size_t inoffset = 0;
	char inbuffer[65536];

	size_t outoffset = 0;
	char outbuffer[65536];

	while(ret == Z_OK)
	{
	    if (zs.avail_in == 0)
	    {
		if (inoffset >= subfile.data.size())
		{
		    inflateEnd(&zs);
		    return ETE_SUBFILE_UNEXPECTED_EOF;
		}

		size_t currlen = std::min<size_t>(sizeof(inbuffer), subfile.data.size() - inoffset);

		memcpy(inbuffer, subfile.data.data() + inoffset, currlen);

		if (subfile.encryption == ENCRYPTION_NONE)
		{
		}
		else if (subfile.encryption == ENCRYPTION_SERPENT256)
		{
		    if (iskeyset)
		    {
			serpentctx.decrypt(inbuffer, currlen);
		    }
		}

		zs.next_in = (Bytef*)(inbuffer);
		zs.avail_in = currlen;

		inoffset += currlen;
	    }

	    zs.next_out = (Bytef*)outbuffer;
	    zs.avail_out = sizeof(outbuffer);

	    ret = inflate(&zs, 0);

	    if (outoffset < zs.total_out)
	    {
		if (!dataout.Output(outbuffer, zs.total_out - outoffset))
		    return ETE_SUBFILE_OUTPUT_ERROR;

		crc32run = update_crc32(crc32run, (uint8_t*)outbuffer, zs.total_out - outoffset);

		outoffset = zs.total_out;
	    }

	    progress.Update(inoffset);
	}

	if (ret != Z_STREAM_END) // an error occurred that was not EOF
	    return ErrorFromZLibError(ret);

	inflateEnd(&zs);
   }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	bz_stream bz;
	memset(&bz, 0, sizeof(bz));

	int ret = BZ2_bzDecompressInit(&bz, 0, 0);
	if (ret != BZ_OK)
	    return ErrorFromBZLibError(ret);

	size_t inoffset = 0;
	char inbuffer[65536];

	size_t outoffset = 0;
	char outbuffer[65536];

	while(ret == BZ_OK)
	{
	    if (bz.avail_in == 0)
	    {
		if (inoffset >= subfile.data.size())
		{
		    BZ2_bzDecompressEnd(&bz);
		    return ETE_SUBFILE_UNEXPECTED_EOF;
		}

		size_t currlen = std::min<size_t>(sizeof(inbuffer), subfile.data.size() - inoffset);

		memcpy(inbuffer, subfile.data.data() + inoffset, currlen);

		if (subfile.encryption == ENCRYPTION_NONE)
		{
		}
		else if (subfile.encryption == ENCRYPTION_SERPENT256)
		{
		    if (iskeyset)
		    {
			serpentctx.decrypt(inbuffer, currlen);
		    }
		}

		bz.next_in = inbuffer;
		bz.avail_in = currlen;

		inoffset += currlen;
	    }

	    bz.next_out = outbuffer;
	    bz.avail_out = sizeof(outbuffer);

	    ret = BZ2_bzDecompress(&bz);

	    if (outoffset < bz.total_out_lo32)
	    {
		if (!dataout.Output(outbuffer, bz.total_out_lo32 - outoffset))
		    return ETE_SUBFILE_OUTPUT_ERROR;

		crc32run = update_crc32(crc32run, (uint8_t*)outbuffer, bz.total_out_lo32 - outoffset);

		outoffset = bz.total_out_lo32;
	    }

	    progress.Update(inoffset);
	}

	BZ2_bzDecompressEnd(&bz);

	if (ret != BZ_STREAM_END) // an error occurred that was not EOF
	    return ErrorFromBZLibError(ret);
    }
    else
    {
	return ETE_SUBFILE_COMPRESSION_INVALID;
    }

    if (crc32run != subfile.crc32)
	return ETE_SUBFILE_CRC32;

    return ETE_SUCCESS;
}

} // namespace internal

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

/* static */ const char* Container::GetErrorString(error_t e)
{
    return internal::ContainerImpl::GetErrorString(e);
}

error_t Container::Save(DataOutput& dataout)
{
    return pimpl->Save(dataout);
}

error_t Container::Load(DataInput& datain, const std::string& filekey)
{
    return pimpl->Load(datain, filekey);
}

void Container::Clear()
{
    return pimpl->Clear();
}

void Container::SetKey(const std::string& keystr)
{
    return pimpl->SetKey(keystr);
}

bool Container::IsKeySet() const
{
    return pimpl->IsKeySet();
}

size_t Container::GetLastWritten() const
{
    return pimpl->GetLastWritten();
}

void Container::SetProgressIndicator(ProgressIndicator* pi)
{
    return pimpl->SetProgressIndicator(pi);
}

void Container::SetGlobalUnencryptedProperty(const std::string& key, const std::string& value)
{
    return pimpl->SetGlobalUnencryptedProperty(key, value);
}

const std::string& Container::GetGlobalUnencryptedProperty(const std::string& key) const
{
    return pimpl->GetGlobalUnencryptedProperty(key);
}

bool Container::EraseGlobalUnencryptedProperty(const std::string& key)
{
    return pimpl->EraseGlobalUnencryptedProperty(key);
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

bool Container::EraseGlobalEncryptedProperty(const std::string& key)
{
    return pimpl->EraseGlobalEncryptedProperty(key);
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

bool Container::EraseSubFileProperty(unsigned int subfileindex, const std::string& key)
{
    return pimpl->EraseSubFileProperty(subfileindex, key);
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

error_t Container::SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen)
{
    return pimpl->SetSubFileData(subfileindex, data, datalen);
}

error_t Container::GetSubFileData(unsigned int subfileindex, std::string& outstr) const
{
    return pimpl->GetSubFileData(subfileindex, outstr);
}

error_t Container::GetSubFileData(unsigned int subfileindex, class DataOutput& dataout) const
{
    return pimpl->GetSubFileData(subfileindex, dataout);
}

} // namespace Enctain
