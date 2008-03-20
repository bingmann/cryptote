// $Id$

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <zlib.h>

void fixfilename(std::string& str)
{
    // remove path
    std::string::size_type pos = str.find_last_of('/');
    if (pos != std::string::npos) {
	str.erase(0, pos+1);
    }

    // replace characters illegal in C identifiers
    for (std::string::iterator ci = str.begin(); ci != str.end(); ++ci)
    {
	if (!isalnum(*ci)) *ci = '_';
    }
}

/**
 * Compress a string using zlib with given compression level and return the
 * binary data.
 *
 * @param str          (binary) string to compress
 * @param compressionlevel     ranging 0-9
 * @return             (binary) compressed image
 */
std::string compress_string(const std::string& str,
                            int compressionlevel = Z_BEST_COMPRESSION)
{
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK) {
	std::cerr << "deflateInit failed while compressing.\n";
	exit(0);
    }

    zs.next_in = (Bytef*)str.data();
    zs.avail_in = str.size();           // set the z_stream's input

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // retreive the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) { // append the block to the output string
            outstring.append(outbuffer,
                             zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
	std::cerr << "Exception during zlib compression: (" << ret << ") " << zs.msg << "\n";
	exit(0);
    }

    return outstring;
}

int main(int argc, char* argv[])
{
    std::cout << "/* $" << "Id$ */\n";
    std::cout << "/* Automatically generated file dump */\n";
    std::cout << "\n";

    std::string prefix;
    bool compress = false;

    for(int ai = 1; ai < argc; ++ai)
    {
	if (strcmp(argv[ai], "-p") == 0 && ai+1 < argc)
	{
	    prefix = argv[++ai];
	    continue;
	}
	if (strcmp(argv[ai], "-c") == 0)
	{
	    compress = true;
	    continue;
	}

	std::cerr << "Reading " << argv[ai] << "\n";

	// open file path
	std::ifstream filestream(argv[ai], std::ios::in | std::ios::binary);
	if (!filestream.good()) {
	    std::cerr << "Could not open " << argv[ai] << "\n";
	    return 0;
	}

	// read complete data file
	std::string filedata;
	char fileindata[4096];
	do {
	    filestream.read(fileindata, sizeof(fileindata));
	    filedata.append(fileindata, filestream.gcount());
	} while(filestream.good());

	unsigned int filesize = filedata.size();

	// compress if required
	if (compress) {
	    filedata = compress_string(filedata);
	    std::cout << "/* compressed data dump of " << argv[ai] << " */\n\n";
	}
	else {
	    std::cout << "/* data dump of " << argv[ai] << " */\n\n";
	}

	// output data char array
	std::string filename = prefix + argv[ai];
	fixfilename(filename);
	
	std::cout << "static char " << filename
		  << "[" << std::dec << filedata.size() << "] = {\n";

	static const int charsperline = 16;

	for(unsigned int ci = 0; ci < filedata.size(); ++ci)
	{
	    if (ci % charsperline == 0) {
		//std::cout << "    ";
	    }

	    unsigned char c = filedata[ci];

	    switch(c)
	    {
	    default:
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)c;
		break;
	    }

	    if (ci+1 < filedata.size()) {
		std::cout << ",";

		if (ci % charsperline == charsperline-1) {
		    std::cout << "\n";
		}
	    }
	}

	std::cout << "\n};\n";

	if (compress) {
	    std::cout << "static const unsigned int " << filename
		      << "_uncompressed = " << std::dec << filesize << ";\n";
	}

	std::cout << "\n";
    }
}
