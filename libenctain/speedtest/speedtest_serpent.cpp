// $Id$

#include <assert.h>
#include <stdint.h>
#include <sys/time.h>

#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <cmath>

#include <gcrypt.h>

#include "../serpent.h"

// *** Speedtest Parameters ***

// speed test different buffer sizes in this range
const unsigned int buffermin = 16;
const unsigned int buffermax = 16 * 65536;
const unsigned int repeatsize = 65536;
const unsigned int minrepeats = 2;
const unsigned int measureruns = 64;

/// Time is measured using gettimeofday()
inline double timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 0.000001;
}

// *** Global Buffers and Settings for the Speedtest Functions ***

char	enckey[32];	/// 256 bit encryption key
char	enciv[16];	/// 16 byte initialization vector if needed.

char	buffer[buffermax];	/// encryption buffer
unsigned int bufferlen;		/// currently tested buffer length

// *** Test Functions for libgcrypt ***

void test_libgcrypt_serpent_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_libgcrypt_serpent_cbc()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_CBC, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
    gcry_cipher_setiv(encctx, (uint8_t*)enciv, 16);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_CBC, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
    gcry_cipher_setiv(decctx, (uint8_t*)enciv, 16);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_custom_serpent_ecb()
{
    {
	Enctain::SerpentECB encctx;

	encctx.set_key((uint8_t*)enckey, 256);

	encctx.encrypt(buffer, buffer, bufferlen);
    }

    {
	Enctain::SerpentECB decctx;

	decctx.set_key((uint8_t*)enckey, 256);

	decctx.decrypt(buffer, buffer, bufferlen);
    }
}

void test_custom_serpent_cbc()
{
    {
	Enctain::SerpentCBC encctx;

	encctx.set_key((uint8_t*)enckey, 256);
	encctx.set_cbciv((uint8_t*)enciv);

	encctx.encrypt(buffer, buffer, bufferlen);
    }

    {
	Enctain::SerpentCBC decctx;

	decctx.set_key((uint8_t*)enckey, 256);
	decctx.set_cbciv((uint8_t*)enciv);

	decctx.decrypt(buffer, buffer, bufferlen);
    }
}

// *** run_test() ***

/**
 * This function will run a test routine multiple times with different buffer
 * sizes configured. It measures the time required to encrypt a number of
 * bytes. The average time and standard deviation are calculated and written to
 * a log file for gnuplot.
 */

template <void (*testfunc)()>
void run_test(const char* logfile)
{
    std::cout << "Speed testing for " << logfile << "\n";

    // Save the time required for each run.
    std::map<unsigned int, std::vector<double> > timelog;

    for(unsigned int fullrun = 0; fullrun < measureruns; ++fullrun)
    {
	for(unsigned int bufflen = buffermin; bufflen <= buffermax; bufflen *= 2)
	{
	    // because small time measurements are inaccurate, repeat very fast
	    // tests until the same amount of data is encrypted as in the large
	    // tests.
	    unsigned int repeat = repeatsize / bufflen;
	    if (repeat < minrepeats) repeat = minrepeats;

	    // std::cout << "Test: bufflen " << bufflen << " repeat " << repeat << "\n";

	    bufferlen = bufflen;

	    // fill buffer
	    for(unsigned int i = 0; i < bufferlen; ++i)
		buffer[i] = (char)i;

	    double ts1 = timestamp();

	    for(unsigned int testrun = 0; testrun < repeat; ++testrun)
	    {
		testfunc();
	    }

	    double ts2 = timestamp();

	    // check buffer status after repeated en/decryption
	    for(unsigned int i = 0; i < bufferlen; ++i)
		assert(buffer[i] == (char)i);

	    timelog[bufferlen].push_back( (ts2 - ts1) / (double)repeat );
	}
    }

    // Calculate and output statistics.
    std::ofstream of (logfile);

    for(std::map<unsigned int, std::vector<double> >::const_iterator ti = timelog.begin();
	ti != timelog.end(); ++ti)
    {
	const std::vector<double>& timelist = ti->second;

	double average = std::accumulate(timelist.begin(), timelist.end(), 0.0) / timelist.size();

	double variance = 0.0;
	for(unsigned int i = 0; i < timelist.size(); ++i)
	    variance += (timelist[i] - average) * (timelist[i] - average);
	variance = variance / (timelist.size() - 1);

	double stddev = std::sqrt(variance);

	if (timelist.size() == 1) { // only one run -> no variance or stddev
	    variance = stddev = 0.0;
	}

	double vmin = *std::min_element(timelist.begin(), timelist.end());
	double vmax = *std::max_element(timelist.begin(), timelist.end());

	of << std::setprecision(16);
	of << ti->first << " " << average << " " << stddev << " " << vmin << " " << vmax << "\n";
    }
}

// *** main() ***

int main()
{
    // Initialize cryptographic library

    gcry_check_version(GCRYPT_VERSION);

    // Create (somewhat) random encryption key and initialization vector

    srand(time(NULL));

    for(unsigned int i = 0; i < sizeof(enckey); ++i)
	enckey[i] = rand();

    for(unsigned int i = 0; i < sizeof(enciv); ++i)
	enciv[i] = rand();

    // Run speed tests

    run_test<test_libgcrypt_serpent_ecb>("gcrypt-serpent-ecb.txt");
    run_test<test_libgcrypt_serpent_cbc>("gcrypt-serpent-cbc.txt");

    run_test<test_custom_serpent_ecb>("custom-serpent-ecb.txt");
    run_test<test_custom_serpent_cbc>("custom-serpent-cbc.txt");
}
