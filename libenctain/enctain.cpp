// $Id$

#include "enctain.h"

#include <wx/log.h>
#include <wx/intl.h>

#include "crc32.h"
#include "bytebuff.h"

#include <zlib.h>
#include <bzlib.h>

namespace Enctain {

// *** Constructor ***

Container::Container()
    : written(0)
{
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
    std::string metadata_compressed;
    {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = deflateInit(&zs, 9);
	if (ret != Z_OK) {
	    wxLogError( wxString::Format(_("Exception during zlib initialization: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    return false;
	}

	zs.next_in = (Bytef*)metadata.data();
	zs.avail_in = metadata.size();

	char buffer[65536];
	do {
	    zs.next_out = (Bytef*)buffer;
	    zs.avail_out = sizeof(buffer);

	    ret = deflate(&zs, Z_FINISH);

	    if (metadata_compressed.size() < zs.total_out)
	    {
		metadata_compressed.append(buffer,
					   zs.total_out - metadata_compressed.size());
	    }
	} while (ret == Z_OK);

	deflateEnd(&zs);

	if (ret != Z_STREAM_END) {
	    wxLogError( wxString::Format(_("Exception during compression: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    return false;
	}
    }

    // Prepare encrypted Header2 by writing all subfile metadata into a buffer
    struct Header2 header2;
    header2.test123 = 0x12345678;
    header2.metacomplen = metadata_compressed.size();
    header2.metacrc32 = crc32((unsigned char*)metadata.data(), metadata.size());
    header2.subfilenum = subfiles.size();

    outstream.Write(&header2, sizeof(header2));

    outstream.Write(metadata_compressed.data(), metadata_compressed.size());

    // Output data of all subfiles simply concatenated

    for (unsigned int si = 0; si < subfiles.size(); ++si)
    {
	assert(subfiles[si].storagesize == subfiles[si].data.GetDataLen());

	outstream.Write(subfiles[si].data.GetData(), subfiles[si].storagesize);
    }

    written = outstream.TellO() - streamoff;
    return true;
}

bool Container::Load(wxInputStream& instream, const std::string& filekey)
{
    // Read unencrypted fixed Header1
    struct Header1 header1;

    instream.Read(&header1, sizeof(header1));

    if (instream.LastRead() != sizeof(header1)) {
	wxLogError(_("Error loading container: could not read header."));
	return false;
    }

    if (memcmp(header1.signature, fsignature, 8) != 0) {
	wxLogError(_("Error loading container: invalid signature."));
	return false;
    }

    if (header1.version == 0x00010000) {
	return Loadv00010000(instream, filekey, header1);
    }
    else {
	wxLogError(_("Error loading container: invalid file version."));
	return false;
    }
}

bool Container::Loadv00010000(wxInputStream& instream, const std::string& filekey, const Header1& header1)
{
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

    if (header2.test123 != 0x12345678) {
	wxLogError(_("Error loading container: could not decrypt header."));
	return false;
    }

    // Read compressed variable length metadata

    ByteBuffer metadata_compressed;
    metadata_compressed.alloc(header2.metacomplen);

    instream.Read(metadata_compressed.data(), header2.metacomplen);
    if (instream.LastRead() != header2.metacomplen) {
	wxLogError(_("Error loading container: could not decrypt metadata."));
	return false;
    }

    metadata_compressed.set_size(header2.metacomplen);

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

	do
	{
	    zs.next_out = (Bytef*)buffer;
	    zs.avail_out = sizeof(buffer);

	    ret = inflate(&zs, 0);

	    if (metadata.size() < zs.total_out) {
		metadata.append(buffer, zs.total_out - metadata.size());
	    }
	}
	while (ret == Z_OK);

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
    }

    return true;
}

// *** Container Info Operations ***

size_t Container::GetLastWritten() const
{
    return written;
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
    subfile.crc32 = crc32((const unsigned char*)data, datalen);

    // Copy or compress data into the wxMemoryBuffer
    if (subfile.compression == COMPRESSION_NONE)
    {
	subfile.data.SetBufSize(datalen);
	subfile.data.SetDataLen(0);
	subfile.data.AppendData(data, datalen);
    }
    else if (subfile.compression == COMPRESSION_ZLIB)
    {
	// z_stream is zlib's control structure
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	int ret = deflateInit(&zs, Z_BEST_COMPRESSION);
	if (ret != Z_OK) {
	    wxLogError( wxString::Format(_("Exception during zlib initialization: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    return false;
	}

	zs.next_in = (Bytef*)(data);
	zs.avail_in = datalen;

	const size_t batch = 65536;
	size_t offset = 0;

	do
	{
	    subfile.data.SetBufSize(offset + batch);

	    zs.next_out = (Bytef*)(subfile.data.GetData()) + offset;
	    zs.avail_out = subfile.data.GetBufSize() - offset;

	    ret = deflate(&zs, Z_FINISH);

	    offset = zs.total_out;
	}
	while (ret == Z_OK);

	if (ret != Z_STREAM_END) { // an error occurred that was not EOF
	    wxLogError( wxString::Format(_("Exception during zlib compression: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    return false;
	}

	deflateEnd(&zs);

	subfile.data.SetDataLen(offset);
    }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	bz_stream bz;
	memset(&bz, 0, sizeof(bz));

	int ret = BZ2_bzCompressInit(&bz, 9, 0, 0);
	if (ret != BZ_OK) {
	    wxLogError( _("Exception during bzip2 initialization.") );
	    return false;
	}

	bz.next_in = (char*)data;
	bz.avail_in = datalen;

	const size_t batch = 65536;
	size_t offset = 0;

	do
	{
	    subfile.data.SetBufSize(offset + batch);

	    bz.next_out = (char*)(subfile.data.GetData()) + offset;
	    bz.avail_out = subfile.data.GetBufSize() - offset;

	    ret = BZ2_bzCompress(&bz, BZ_FINISH);

	    offset = bz.total_out_lo32;
	}
	while (ret == BZ_OK);

	if (ret != BZ_STREAM_END) {
	    wxLogError( wxString::Format(_("Exception during bzip2 compression: %d"), ret) );
	    return false;
	}

	BZ2_bzCompressEnd(&bz);

	subfile.data.SetDataLen(offset);
    }
    else
    {
	wxLogError( _("Exception during compression: unknown compression algorithm.") );
	return false;
    }

    subfile.storagesize = subfile.data.GetDataLen();

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

bool Container::GetSubFileData(unsigned int subfileindex, class DataAcceptor& da) const
{
    if (subfileindex >= subfiles.size())
	throw(std::runtime_error("Invalid subfile index"));

    const SubFile& subfile = subfiles[subfileindex];

    if (subfile.data.GetDataLen() == 0) return true;

    // Copy or decompress data into the wxMemoryBuffer
    if (subfile.compression == COMPRESSION_NONE)
    {
	da.Append( subfile.data.GetData(), subfile.data.GetDataLen() );

	return true;
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
	    return false;
	}

	zs.next_in = (Bytef*)(subfile.data.GetData());
	zs.avail_in = subfile.data.GetDataLen();

	size_t output = 0;
	char buffer[65536];

	do
	{
	    zs.next_out = (Bytef*)buffer;
	    zs.avail_out = sizeof(buffer);

	    ret = inflate(&zs, 0);

	    if (output < zs.total_out)
	    {
		da.Append(buffer, zs.total_out - output);
		output = zs.total_out;
	    }
	}
	while (ret == Z_OK);

	if (ret != Z_STREAM_END) { // an error occurred that was not EOF
	    wxLogError( wxString::Format(_("Exception during decompression: (%d) %s"),
					 ret, wxString(zs.msg, wxConvISO8859_1).c_str()) );
	    return false;
	}

	inflateEnd(&zs);

	return true;
    }
    else if (subfile.compression == COMPRESSION_BZIP2)
    {
	bz_stream bz;
	memset(&bz, 0, sizeof(bz));

	int ret = BZ2_bzDecompressInit(&bz, 0, 0);
	if (ret != BZ_OK) {
	    wxLogError( _("Exception during bzip2 initialization.") );
	    return false;
	}

	bz.next_in = (char*)(subfile.data.GetData());
	bz.avail_in = subfile.data.GetDataLen();

	size_t output = 0;
	char buffer[65536];

	do
	{
	    bz.next_out = (char*)buffer;
	    bz.avail_out = sizeof(buffer);

	    ret = BZ2_bzDecompress(&bz);

	    if (output < bz.total_out_lo32)
	    {
		da.Append(buffer, bz.total_out_lo32 - output);
		output = bz.total_out_lo32;
	    }
	}
	while (ret == BZ_OK);

	BZ2_bzDecompressEnd(&bz);

	if (ret != BZ_STREAM_END) { // an error occurred that was not EOF
	    wxLogError( wxString::Format(_("Exception during bzip2 decompression: %d"), ret) );
	    return false;
	}

	return true;
    }
    else
    {
	wxLogError( _("Exception during decompression: unknown decompression algorithm.") );
	return false;
    }
}

DataAcceptor::~DataAcceptor()
{
}

} // namespace Enctain
