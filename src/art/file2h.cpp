// $Id$

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

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

int main(int argc, char* argv[])
{
    std::cout << "/* $" << "Id$ */\n";
    std::cout << "/* Automatically generated file dump */\n";
    std::cout << "\n";

    for(int ai = 1; ai < argc; ++ai)
    {
	std::cerr << "Reading " << argv[ai] << "\n";

	// open file path
	std::ifstream filestream(argv[ai]);
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

	// output data char array
	std::cout << "/* data dump of " << argv[ai] << " */\n\n";

	std::string filename = argv[ai];
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

	std::cout << "\n};\n\n";
    }
}
