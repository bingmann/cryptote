// $Id$

#include "enctain.h"

#include <wx/log.h>
#include <wx/intl.h>
#include <wx/utils.h>

#include "crc32.h"
#include "bytebuff.h"
#include "sha256.h"
#include "serpent.h"

#include <stdlib.h>
#include <time.h>
#include <zlib.h>
#include <bzlib.h>

namespace Enctain {

// *** Constructor and Destructor ***

Container::Container()
    : opened(false), modified(false),
      iskeyset(false), written(0),
      progressindicator(NULL)
{
}

Container::~Container()
{
    serpentctx.wipe();
}

// *** Progress Indicator Wrappers ***

void Container::ProgressStart(const char* text, size_t value, size_t limit) const
{
    if (progressindicator)
	progressindicator->ProgressStart(text, value, limit);
}

void Container::ProgressUpdate(size_t value) const
{
    if (progressindicator)
	progressindicator->ProgressUpdate(value);
}

void Container::ProgressStop() const
{
    if (progressindicator)
	progressindicator->ProgressStop();
}

// *** Settings ***

char Container::fsignature[8] =
{ 'C', 'r', 'y', 'p', 't', 'o', 'T', 'E' };

void Container::SetSignature(const char* sign)
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

// *** Load/Save Operations ***

bool Container::Save(wxOutputStream& outstream)
{
    written = 0;
    off_t streamoff = outstream.TellO();

    if (!iskeyset) {
	wxLogError( _("Error loading container: no encryption password set!") );
	return false;
    }

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

    ProgressStart("Saving Container", 0, esttotal);

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

	outstream.Write(&header1, sizeof(header1));
	outstream.Write(unc_metadata.data(), unc_metadata.size());

	ProgressUpdate(outstream.TellO() - streamoff);
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
	if (ret != Z_OK) {
	    wxLogError( wxString::Format(_("Exception during zlib initialization: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    ProgressStop();
	    return false;
	}

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

	if (ret != Z_STREAM_END) {
	    wxLogError( wxString::Format(_("Exception during compression: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    ProgressStop();
	    return false;
	}

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

    outstream.Write(&header2, sizeof(header2));
    outstream.Write(metadata_compressed.data(), metadata_compressed.size());

    // Refine file target size because it is now exactly known.
    esttotal = (outstream.TellO() - streamoff) + subfiletotal;
    ProgressStart("Saving Container", (outstream.TellO() - streamoff), esttotal);

    // Output data of all subfiles simply concatenated

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	assert(subfiles[si].storagesize == subfiles[si].data.GetDataLen());

	outstream.Write(subfiles[si].data.GetData(), subfiles[si].storagesize);

	ProgressUpdate(outstream.TellO() - streamoff);
    }

    written = (outstream.TellO() - streamoff);
    opened = true;

    ProgressStop();
    return true;
}

bool Container::Load(wxInputStream& instream, const std::string& filekey)
{
    ProgressStart("Loading Container", 0, 1000);

    opened = false;

    // Read unencrypted fixed Header1
    struct Header1 header1;

    instream.Read(&header1, sizeof(header1));

    if (instream.LastRead() != sizeof(header1)) {
	wxLogError(_("Error loading container: could not read header."));
	ProgressStop();
	return false;
    }

    if (memcmp(header1.signature, fsignature, 8) != 0) {
	wxLogError(_("Error loading container: invalid signature."));
	ProgressStop();
	return false;
    }

    if (header1.version == 0x00010000) {
	bool b = Loadv00010000(instream, filekey, header1);
	ProgressStop();
	return b;
    }
    else {
	wxLogError(_("Error loading container: invalid file version."));
	ProgressStop();
	return false;
    }
}

bool Container::Loadv00010000(wxInputStream& instream, const std::string& filekey, const Header1& header1)
{
    size_t streamoff = instream.TellI() - sizeof(Header1);

    // Read unencrypted metadata length
    {
	ByteBuffer unc_metadata;
	unc_metadata.alloc(header1.unc_metalen);

	instream.Read(unc_metadata.data(), header1.unc_metalen);
	if (instream.LastRead() != header1.unc_metalen) {
	    wxLogError(_("Error loading container: could not unencrypted metadata."));
	    return false;
	}

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
	    wxLogError(_("Error loading container: could not parse unencrypted metadata buffer."));
	    return false;
	}
    }

    // Read encrypted fixed Header2
    struct Header2 header2;

    instream.Read(&header2, sizeof(header2));

    if (instream.LastRead() != sizeof(header2)) {
	wxLogError(_("Error loading container: could not read secondary header."));
	return false;
    }

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

    if (header2.test123 != 0x12345678) {
	wxLogError(_("Error loading container: could not decrypt header. Check the encryption key."));
	return false;
    }

    // Read compressed variable length metadata

    ByteBuffer metadata_compressed;
    metadata_compressed.alloc(header2.metacomplen);

    instream.Read(metadata_compressed.data(), header2.metacomplen);
    if (instream.LastRead() != header2.metacomplen) {
	wxLogError(_("Error loading container: could not decrypt metadata. Check the encryption key."));
	return false;
    }

    metadata_compressed.set_size(header2.metacomplen);

    decctx.decrypt(metadata_compressed.data(), metadata_compressed.size());

    // Decompress variable encrypted metadata

    ByteBuffer metadata;

    {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = inflateInit(&zs);
	if (ret != Z_OK) {
	    wxLogError( wxString::Format(_("Exception during zlib initialization: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    return false;
	}

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

	if (ret != Z_STREAM_END) {
	    wxLogError( wxString::Format(_("Exception during decompression: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    return false;
	}

	inflateEnd(&zs);

	uint32_t metacrc32 = crc32((unsigned char*)metadata.data(), metadata.size());

	if (metacrc32 != header2.metacrc32) {
	    wxLogError( _("Error loading container: metadata crc32 does not match.") );
	    return false;
	}
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
	wxLogError(_("Error loading container: could not parse metadata buffer."));
	return false;
    }

    // Now we can say how large the file actually is.

    size_t subfiletotal = 0;

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	subfiletotal += subfiles[si].storagesize;
    }

    ProgressStart("Loading Container",
		  (instream.TellI() - streamoff),
		  (instream.TellI() - streamoff) + subfiletotal);

    // load data of all subfiles which are simply concatenated

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	SubFile& subfile = subfiles[si];

	subfile.data.SetBufSize( subfile.storagesize );

	instream.Read(subfile.data.GetData(), subfile.storagesize);
	subfile.data.SetDataLen(instream.LastRead());

	if (instream.LastRead() != subfile.storagesize) {
	    wxLogError(_("Error loading container: could not read encrypted subfile data."));
	    return false;
	}

	ProgressUpdate(instream.TellI() - streamoff);
    }

    opened = true;
    iskeyset = true;
    serpentctx = decctx;

    decctx.wipe();

    return true;
}

// *** Container Info Operations ***

bool Container::IsOpen() const
{
    return opened;
}

void Container::SetKey(const std::string& keystr)
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

    ProgressStart("Reencrypting", 0, totalsize);

    // reencrypt all subfile data

    totalsize = 0;
    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	SubFile& subfile = subfiles[si];

	assert(subfile.storagesize == subfile.data.GetDataLen());

	if (subfile.encryption == ENCRYPTION_NONE)
	{
	    totalsize += subfile.storagesize;
	    ProgressUpdate(totalsize);
	}
	else if (subfile.encryption == ENCRYPTION_SERPENT256)
	{
	    if (iskeyset)
		serpentctx.set_cbciv(subfile.cbciv);

	    newctx.set_cbciv(subfile.cbciv);

	    const size_t batch = 65536;

	    for (size_t offset = 0; offset < subfile.data.GetDataLen(); offset += batch)
	    {
		size_t len = wxMin(batch, subfile.data.GetDataLen() - offset);

		if (iskeyset)
		    serpentctx.decrypt((char*)subfile.data.GetData() + offset, len);
	
		newctx.encrypt((char*)subfile.data.GetData() + offset, len);

		totalsize += len;
		ProgressUpdate(totalsize);
	    }
	}
    }

    ProgressStop();

    serpentctx = newctx;
    iskeyset = true;

    newctx.wipe();
}

bool Container::IsKeySet() const
{
    return iskeyset;
}

size_t Container::GetLastWritten() const
{
    return written;
}

void Container::SetProgressIndicator(ProgressIndicator* pi)
{
    progressindicator = pi;
}

// *** Container Global Unencrypted Properties ***

void Container::SetGlobalUnencryptedProperty(const std::string& key, const std::string& value)
{
    unc_properties[key] = value;
}

const std::string& Container::GetGlobalUnencryptedProperty(const std::string& key) const
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

bool Container::EraseGlobalUnencryptedProperty(const std::string& key)
{
    return (unc_properties.erase(key) > 0);
}

bool Container::GetGlobalUnencryptedPropertyIndex(unsigned int propindex, std::string& key, std::string& value) const
{
    if (propindex >= unc_properties.size()) return false;

    propertymap_type::const_iterator pi = unc_properties.begin();

    for(unsigned int i = 0; i < propindex; ++i)	++pi;

    key = pi->first;
    value = pi->second;

    return true;
}

// *** Container Global Encrypted Properties ***

void Container::SetGlobalEncryptedProperty(const std::string& key, const std::string& value)
{
    enc_properties[key] = value;
}

const std::string& Container::GetGlobalEncryptedProperty(const std::string& key) const
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

bool Container::EraseGlobalEncryptedProperty(const std::string& key)
{
    return (enc_properties.erase(key) > 0);
}

bool Container::GetGlobalEncryptedPropertyIndex(unsigned int propindex, std::string& key, std::string& value) const
{
    if (propindex >= enc_properties.size()) return false;

    propertymap_type::const_iterator pi = enc_properties.begin();

    for(unsigned int i = 0; i < propindex; ++i)	++pi;

    key = pi->first;
    value = pi->second;

    return true;
}

// *** Container SubFiles - Subfile array management ***

unsigned int Container::CountSubFile() const
{
    return subfiles.size();
}

unsigned int Container::AppendSubFile()
{
    unsigned int si = subfiles.size();
    subfiles.push_back( SubFile() );
    return si;
}

unsigned int Container::InsertSubFile(unsigned int subfileindex)
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

bool Container::DeleteSubFile(unsigned int subfileindex)
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

void Container::SetSubFileProperty(unsigned int subfileindex, const std::string& key, const std::string& value)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    SubFile& subfile = subfiles[subfileindex];
    subfile.properties[ key ] = value;
}

const std::string& Container::GetSubFileProperty(unsigned int subfileindex, const std::string& key) const
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

bool Container::EraseSubFileProperty(unsigned int subfileindex, const std::string& key)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    SubFile& subfile = subfiles[subfileindex];
    return (subfile.properties.erase(key) > 0);
}

bool Container::GetSubFilePropertyIndex(unsigned int subfileindex, unsigned int propindex, std::string& key, std::string& value) const
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

uint32_t Container::GetSubFileStorageSize(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    return subfiles[subfileindex].storagesize;
}

uint32_t Container::GetSubFileSize(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    return subfiles[subfileindex].realsize;
}

encryption_t Container::GetSubFileEncryption(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    return (encryption_t)subfiles[subfileindex].encryption;
}

compression_t Container::GetSubFileCompression(unsigned int subfileindex) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    return (compression_t)subfiles[subfileindex].compression;
}

// *** Container SubFiles - Set operations of subfile header fields ***

void Container::SetSubFileEncryption(unsigned int subfileindex, encryption_t c)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    if (c < 0 || c > ENCRYPTION_SERPENT256)
	throw(std::runtime_error("Invalid encryption cipher index"));

    // reencrypt if necessary
    if (subfiles[subfileindex].encryption != c)
    {
	wxMemoryBuffer data;

	GetSubFileData(subfileindex, data);

	subfiles[subfileindex].encryption = c;

	SetSubFileData(subfileindex, data.GetData(), data.GetDataLen());
    }
}

void Container::SetSubFileCompression(unsigned int subfileindex, compression_t c)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    if (c < 0 || c > COMPRESSION_BZIP2)
	throw(std::runtime_error("Invalid compression algorithm index"));

    // recompress if necessary
    if (subfiles[subfileindex].compression != c)
    {
	wxMemoryBuffer data;

	GetSubFileData(subfileindex, data);

	subfiles[subfileindex].compression = c;

	SetSubFileData(subfileindex, data.GetData(), data.GetDataLen());
    }
}

void Container::SetSubFileCompressionEncryption(unsigned int subfileindex, compression_t comp, encryption_t enc)
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
	wxMemoryBuffer data;

	GetSubFileData(subfileindex, data);

	subfiles[subfileindex].encryption = enc;
	subfiles[subfileindex].compression = comp;

	SetSubFileData(subfileindex, data.GetData(), data.GetDataLen());
    }
}

// *** Container SubFiles - Subfile data operations ***

bool Container::SetSubFileData(unsigned int subfileindex, const void* data, unsigned int datalen)
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    SubFile& subfile = subfiles[subfileindex];

    subfile.realsize = datalen;

    ProgressStart("Saving SubFile", 0, datalen);

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
	wxLogError( _("Exception during encryption: unknown encryption cipher.") );
	ProgressStop();
	return false;
    }

    // Copy or compress data into the wxMemoryBuffer

    uint32_t crc32run = 0;

    if (subfile.compression == COMPRESSION_NONE)
    {
	subfile.data.SetBufSize(datalen);
	subfile.data.SetDataLen(0);

	const size_t batch = 65536;
	size_t offset = 0;

	while(offset < datalen)
	{
	    size_t currlen = wxMin(batch, datalen - offset);

	    subfile.data.AppendData((char*)data + offset, currlen);

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
			subfile.data.AppendByte(0);
			++enclen;
		    }
		}

		if (iskeyset)
		{
		    serpentctx.encrypt((char*)subfile.data.GetData() + offset, enclen);
		}
	    }

	    offset += currlen;
	    ProgressUpdate(offset);
	}
    }
    else if (subfile.compression == COMPRESSION_ZLIB)
    {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = deflateInit(&zs, Z_BEST_COMPRESSION);
	if (ret != Z_OK) {
	    wxLogError( wxString::Format(_("Exception during zlib initialization: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    ProgressStop();
	    return false;
	}

	zs.next_in = (Bytef*)(data);
	zs.avail_in = datalen;

	crc32run = update_crc32(crc32run, zs.next_in, zs.avail_in);

	const size_t batch = 65536;
	size_t offset = 0;
	size_t encoffset = 0;

	while (ret == Z_OK)
	{
	    subfile.data.SetBufSize(offset + batch); // extend output buffer

	    zs.next_out = (Bytef*)(subfile.data.GetData()) + offset;
	    zs.avail_out = subfile.data.GetBufSize() - offset;

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

		    serpentctx.encrypt((char*)subfile.data.GetData() + encoffset, encsize);

		    encoffset += encsize;
		}

		offset = zs.total_out;
		ProgressUpdate(datalen - zs.avail_in);
	    }
	}

	if (ret != Z_STREAM_END) { // an error occurred that was not EOF
	    wxLogError( wxString::Format(_("Exception during zlib compression: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    ProgressStop();
	    return false;
	}

	deflateEnd(&zs);

	subfile.data.SetDataLen(offset);

	if (subfile.encryption == ENCRYPTION_SERPENT256)
	{
	    if (encoffset < subfile.data.GetDataLen())
	    {
		while(subfile.data.GetDataLen() % 16 != 0) {
		    subfile.data.AppendByte(0);
		}

		if (iskeyset)
		{
		    size_t encsize = subfile.data.GetDataLen() - encoffset;
		    serpentctx.encrypt((char*)subfile.data.GetData() + encoffset, encsize);
		}
	    }
	}
    }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	bz_stream bz;
	memset(&bz, 0, sizeof(bz));

	int ret = BZ2_bzCompressInit(&bz, 9, 0, 0);
	if (ret != BZ_OK) {
	    wxLogError( _("Exception during bzip2 initialization.") );
	    ProgressStop();
	    return false;
	}

	bz.next_in = (char*)data;
	bz.avail_in = datalen;

	crc32run = update_crc32(crc32run, (uint8_t*)bz.next_in, bz.avail_in);

	const size_t batch = 65536;
	size_t offset = 0;
	size_t encoffset = 0;

	while (ret == BZ_OK || ret == BZ_FINISH_OK)
	{
	    subfile.data.SetBufSize(offset + batch); // extend output buffer

	    bz.next_out = (char*)(subfile.data.GetData()) + offset;
	    bz.avail_out = subfile.data.GetBufSize() - offset;

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

		    serpentctx.encrypt((char*)subfile.data.GetData() + encoffset, encsize);

		    encoffset += encsize;
		}

		offset = bz.total_out_lo32;
		ProgressUpdate(datalen - bz.avail_in);
	    }
	}

	if (ret != BZ_STREAM_END) {
	    wxLogError( wxString::Format(_("Exception during bzip2 compression: %d"), ret) );
	    ProgressStop();
	    return false;
	}

	BZ2_bzCompressEnd(&bz);

	subfile.data.SetDataLen(offset);

	if (subfile.encryption == ENCRYPTION_SERPENT256)
	{
	    if (encoffset < subfile.data.GetDataLen())
	    {
		while(subfile.data.GetDataLen() % 16 != 0) {
		    subfile.data.AppendByte(0);
		}

		if (iskeyset)
		{
		    size_t encsize = subfile.data.GetDataLen() - encoffset;
		    serpentctx.encrypt((char*)subfile.data.GetData() + encoffset, encsize);
		}
	    }
	}
    }
    else
    {
	wxLogError( _("Exception during compression: unknown compression algorithm.") );
	ProgressStop();
	return false;
    }

    subfile.crc32 = crc32run;
    subfile.storagesize = subfile.data.GetDataLen();

    ProgressStop();
    return true;
}

struct GetSubFileDataAcceptor : public DataAcceptor
{
    wxMemoryBuffer&	buffer;

    GetSubFileDataAcceptor(wxMemoryBuffer& mb)
	: buffer(mb)
    {
    }

    ~GetSubFileDataAcceptor()
    {
    }

    virtual void Append(const void* data, size_t datalen)
    {
	buffer.AppendData(data, datalen);
    }
};

bool Container::GetSubFileData(unsigned int subfileindex, wxMemoryBuffer& outdata) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    const SubFile& subfile = subfiles[subfileindex];

    outdata.SetBufSize(subfile.realsize);
    outdata.SetDataLen(0);

    GetSubFileDataAcceptor da(outdata);

    return GetSubFileData(subfileindex, da);
}

bool Container::GetSubFileData(unsigned int subfileindex, class DataAcceptor& acceptor) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    const SubFile& subfile = subfiles[subfileindex];

    if (subfile.data.GetDataLen() == 0) return true;

    assert(subfile.data.GetDataLen() == subfile.storagesize);

    ProgressStart("Loading SubFile", 0, subfile.storagesize);

    // Setup decryption context if requested

    if (subfile.encryption == ENCRYPTION_NONE)
    {
    }
    else if (subfile.encryption == ENCRYPTION_SERPENT256)
    {
	if (iskeyset)
	{
	    // Ensure that the data buffer has a length multiple of 16
	    if (subfile.data.GetDataLen() % 16 != 0) {
		wxLogError( _("Exception during subfile decryption: invalid data length.") );
		ProgressStop();
		return false;
	    }

	    // Prepare data block decryption
	    serpentctx.set_cbciv(subfile.cbciv);
	}
    }
    else
    {
	wxLogError( _("Exception during decryption: unknown encryption cipher.") );
	ProgressStop();
	return false;
    }

    // Copy or decompress subfile data and send output to DataAcceptor

    uint32_t crc32run = 0;

    if (subfile.compression == COMPRESSION_NONE)
    {
	size_t offset = 0;
	char buffer[65536];

	assert(subfile.data.GetDataLen() >= subfile.realsize);

	while(offset < subfile.data.GetDataLen())
	{
	    size_t currlen = wxMin(sizeof(buffer), subfile.data.GetDataLen() - offset);

	    memcpy(buffer, (char*)subfile.data.GetData() + offset, currlen);

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

	    acceptor.Append(buffer, reallen);
	    crc32run = update_crc32(crc32run, (uint8_t*)buffer, reallen);

	    offset += currlen;
	    ProgressUpdate(offset);
	}
    }
    else if (subfile.compression == COMPRESSION_ZLIB)
    {
	// z_stream is zlib's control structure
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = inflateInit(&zs);
	if (ret != Z_OK) {
	    wxLogError( wxString::Format(_("Exception during zlib initialization: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    ProgressStop();
	    return false;
	}

	size_t inoffset = 0;
	char inbuffer[65536];

	size_t outoffset = 0;
	char outbuffer[65536];

	while(ret == Z_OK)
	{
	    if (zs.avail_in == 0)
	    {
		if (inoffset >= subfile.data.GetDataLen())
		{
		    wxLogError( _("Exception during decompression: reading beyond end of stream") );
		    inflateEnd(&zs);
		    ProgressStop();
		    return false;
		}

		size_t currlen = wxMin(sizeof(inbuffer), subfile.data.GetDataLen() - inoffset);

		memcpy(inbuffer, (char*)subfile.data.GetData() + inoffset, currlen);

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
		acceptor.Append(outbuffer, zs.total_out - outoffset);
		crc32run = update_crc32(crc32run, (uint8_t*)outbuffer, zs.total_out - outoffset);

		outoffset = zs.total_out;
	    }

	    ProgressUpdate(inoffset);
	}

	if (ret != Z_STREAM_END) { // an error occurred that was not EOF
	    wxLogError( wxString::Format(_("Exception during decompression: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    ProgressStop();
	    return false;
	}

	inflateEnd(&zs);
   }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	bz_stream bz;
	memset(&bz, 0, sizeof(bz));

	int ret = BZ2_bzDecompressInit(&bz, 0, 0);
	if (ret != BZ_OK) {
	    wxLogError( _("Exception during bzip2 initialization.") );
	    ProgressStop();
	    return false;
	}

	size_t inoffset = 0;
	char inbuffer[65536];

	size_t outoffset = 0;
	char outbuffer[65536];

	while(ret == BZ_OK)
	{
	    if (bz.avail_in == 0)
	    {
		if (inoffset >= subfile.data.GetDataLen())
		{
		    wxLogError( _("Exception during decompression: reading beyond end of stream") );
		    BZ2_bzDecompressEnd(&bz);
		    ProgressStop();
		    return false;
		}

		size_t currlen = wxMin(sizeof(inbuffer), subfile.data.GetDataLen() - inoffset);

		memcpy(inbuffer, (char*)subfile.data.GetData() + inoffset, currlen);

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
		acceptor.Append(outbuffer, bz.total_out_lo32 - outoffset);
		crc32run = update_crc32(crc32run, (uint8_t*)outbuffer, bz.total_out_lo32 - outoffset);

		outoffset = bz.total_out_lo32;
	    }

	    ProgressUpdate(inoffset);
	}

	BZ2_bzDecompressEnd(&bz);

	if (ret != BZ_STREAM_END) { // an error occurred that was not EOF
	    wxLogError( wxString::Format(_("Exception during bzip2 decompression: %d"), ret) );
	    ProgressStop();
	    return false;
	}
    }
    else
    {
	wxLogError( _("Exception during decompression: unknown decompression algorithm.") );
	ProgressStop();
	return false;
    }

    if (crc32run != subfile.crc32)
    {
	wxLogError( _("Exception during subfile loading: crc32 mismatch - data possibly corrupt.") );
    }

    ProgressStop();
    return true;
}

DataAcceptor::~DataAcceptor()
{
}

ProgressIndicator::~ProgressIndicator()
{
}

} // namespace Enctain
