/*************************************************
* Default Engine Algorithms Source File          *
* (C) 1999-2007 The Botan Project                *
*************************************************/

#include "botan-1.6/include/eng_def.h"
#include "botan-1.6/include/lookup.h"
#include "botan-1.6/include/parsing.h"

#include "botan-1.6/include/aes.h"
// #include "botan-1.6/include/blowfish.h"
// #include "botan-1.6/include/cast128.h"
// #include "botan-1.6/include/cast256.h"
#include "botan-1.6/include/des.h"
// #include "botan-1.6/include/gost.h"
// #include "botan-1.6/include/idea.h"
// #include "botan-1.6/include/kasumi.h"
// #include "botan-1.6/include/lion.h"
// #include "botan-1.6/include/lubyrack.h"
// #include "botan-1.6/include/mars.h"
// #include "botan-1.6/include/misty1.h"
// #include "botan-1.6/include/rc2.h"
// #include "botan-1.6/include/rc5.h"
// #include "botan-1.6/include/rc6.h"
// #include "botan-1.6/include/safer_sk.h"
// #include "botan-1.6/include/seed.h"
#include "botan-1.6/include/serpent.h"
// #include "botan-1.6/include/skipjack.h"
// #include "botan-1.6/include/square.h"
// #include "botan-1.6/include/tea.h"
#include "botan-1.6/include/twofish.h"
// #include "botan-1.6/include/xtea.h"

// #include "botan-1.6/include/arc4.h"
// #include "botan-1.6/include/turing.h"
// #include "botan-1.6/include/wid_wake.h"

// #include "botan-1.6/include/adler32.h"
// #include "botan-1.6/include/crc24.h"
#include "botan-1.6/include/crc32.h"
// #include "botan-1.6/include/fork256.h"
// #include "botan-1.6/include/has160.h"
// #include "botan-1.6/include/md2.h"
// #include "botan-1.6/include/md4.h"
#include "botan-1.6/include/md5.h"
// #include "botan-1.6/include/rmd128.h"
// #include "botan-1.6/include/rmd160.h"
#include "botan-1.6/include/sha160.h"
#include "botan-1.6/include/sha256.h"
#include "botan-1.6/include/sha_64.h"
// #include "botan-1.6/include/tiger.h"
// #include "botan-1.6/include/whrlpool.h"
// #include "botan-1.6/include/par_hash.h"

// #include "botan-1.6/include/cmac.h"
#include "botan-1.6/include/hmac.h"
// #include "botan-1.6/include/x919_mac.h"

#include "botan-1.6/include/mode_pad.h"
// #include "botan-1.6/include/pgp_s2k.h"
#include "botan-1.6/include/pkcs5.h"

namespace Enctain {
namespace Botan {

/*************************************************
* Some macros to simplify control flow           *
*************************************************/
#define HANDLE_TYPE_NO_ARGS(NAME, TYPE)        \
   if(algo_name == NAME)                       \
      {                                        \
      if(name.size() == 1)                     \
         return new TYPE;                      \
      throw Invalid_Algorithm_Name(algo_spec); \
      }

#define HANDLE_TYPE_ONE_U32BIT(NAME, TYPE, DEFAULT) \
   if(algo_name == NAME)                            \
      {                                             \
      if(name.size() == 1)                          \
         return new TYPE(DEFAULT);                  \
      if(name.size() == 2)                          \
         return new TYPE(to_u32bit(name[1]));       \
      throw Invalid_Algorithm_Name(algo_spec);      \
      }

#define HANDLE_TYPE_TWO_U32BIT(NAME, TYPE, DEFAULT)               \
   if(algo_name == NAME)                                          \
      {                                                           \
      if(name.size() == 1)                                        \
         return new TYPE(DEFAULT);                                \
      if(name.size() == 2)                                        \
         return new TYPE(to_u32bit(name[1]));                     \
      if(name.size() == 3)                                        \
         return new TYPE(to_u32bit(name[1]), to_u32bit(name[2])); \
      throw Invalid_Algorithm_Name(algo_spec);                    \
      }

#define HANDLE_TYPE_ONE_STRING(NAME, TYPE)     \
   if(algo_name == NAME)                       \
      {                                        \
      if(name.size() == 2)                     \
         return new TYPE(name[1]);             \
      throw Invalid_Algorithm_Name(algo_spec); \
      }

/*************************************************
* Look for an algorithm with this name           *
*************************************************/
BlockCipher*
Default_Engine::find_block_cipher(const std::string& algo_spec) const
   {
   std::vector<std::string> name = parse_algorithm_name(algo_spec);
   if(name.empty())
      return 0;
   const std::string algo_name = deref_alias(name[0]);

   HANDLE_TYPE_NO_ARGS("AES", AES);
   HANDLE_TYPE_NO_ARGS("AES-128", AES_128);
   HANDLE_TYPE_NO_ARGS("AES-192", AES_192);
   HANDLE_TYPE_NO_ARGS("AES-256", AES_256);
   // HANDLE_TYPE_NO_ARGS("Blowfish", Blowfish);
   // HANDLE_TYPE_NO_ARGS("CAST-128", CAST_128);
   // HANDLE_TYPE_NO_ARGS("CAST-256", CAST_256);
   HANDLE_TYPE_NO_ARGS("DES", DES);
   HANDLE_TYPE_NO_ARGS("DESX", DESX);
   HANDLE_TYPE_NO_ARGS("TripleDES", TripleDES);
   // HANDLE_TYPE_NO_ARGS("GOST", GOST);
   // HANDLE_TYPE_NO_ARGS("IDEA", IDEA);
   // HANDLE_TYPE_NO_ARGS("KASUMI", KASUMI);
   // HANDLE_TYPE_ONE_STRING("Luby-Rackoff", LubyRackoff);
   // HANDLE_TYPE_NO_ARGS("MARS", MARS);
   // HANDLE_TYPE_ONE_U32BIT("MISTY1", MISTY1, 8);
   // HANDLE_TYPE_NO_ARGS("RC2", RC2);
   // HANDLE_TYPE_ONE_U32BIT("RC5", RC5, 12);
   // HANDLE_TYPE_NO_ARGS("RC6", RC6);
   // HANDLE_TYPE_ONE_U32BIT("SAFER-SK", SAFER_SK, 10);
   // HANDLE_TYPE_NO_ARGS("SEED", SEED);
   HANDLE_TYPE_NO_ARGS("Serpent", Serpent);
   // HANDLE_TYPE_NO_ARGS("Skipjack", Skipjack);
   // HANDLE_TYPE_NO_ARGS("Square", Square);
   // HANDLE_TYPE_NO_ARGS("TEA", TEA);
   HANDLE_TYPE_NO_ARGS("Twofish", Twofish);
   // HANDLE_TYPE_NO_ARGS("XTEA", XTEA);

   // if(algo_name == "Lion")
   //    {
   //    if(name.size() != 4)
   //       throw Invalid_Algorithm_Name(algo_spec);
   //    return new Lion(name[1], name[2], to_u32bit(name[3]));
   //    }
   return 0;
   }

/*************************************************
* Look for an algorithm with this name           *
*************************************************/
StreamCipher*
Default_Engine::find_stream_cipher(const std::string& algo_spec) const
   {
   std::vector<std::string> name = parse_algorithm_name(algo_spec);
   if(name.empty())
      return 0;
   const std::string algo_name = deref_alias(name[0]);

   // HANDLE_TYPE_ONE_U32BIT("ARC4", ARC4, 0);
   // HANDLE_TYPE_ONE_U32BIT("RC4_drop", ARC4, 768);
   // HANDLE_TYPE_NO_ARGS("Turing", Turing);
   // HANDLE_TYPE_NO_ARGS("WiderWake4+1-BE", WiderWake_41_BE);

   return 0;
   }

/*************************************************
* Look for an algorithm with this name           *
*************************************************/
HashFunction*
Default_Engine::find_hash(const std::string& algo_spec) const
   {
   std::vector<std::string> name = parse_algorithm_name(algo_spec);
   if(name.empty())
      return 0;
   const std::string algo_name = deref_alias(name[0]);

   // HANDLE_TYPE_NO_ARGS("Adler32", Adler32);
   // HANDLE_TYPE_NO_ARGS("CRC24", CRC24);
   HANDLE_TYPE_NO_ARGS("CRC32", CRC32);
   // HANDLE_TYPE_NO_ARGS("FORK-256", FORK_256);
   // HANDLE_TYPE_NO_ARGS("HAS-160", HAS_160);
   // HANDLE_TYPE_NO_ARGS("MD2", MD2);
   // HANDLE_TYPE_NO_ARGS("MD4", MD4);
   HANDLE_TYPE_NO_ARGS("MD5", MD5);
   // HANDLE_TYPE_NO_ARGS("RIPEMD-128", RIPEMD_128);
   // HANDLE_TYPE_NO_ARGS("RIPEMD-160", RIPEMD_160);
   HANDLE_TYPE_NO_ARGS("SHA-160", SHA_160);
   HANDLE_TYPE_NO_ARGS("SHA-256", SHA_256);
   HANDLE_TYPE_NO_ARGS("SHA-384", SHA_384);
   HANDLE_TYPE_NO_ARGS("SHA-512", SHA_512);
   // HANDLE_TYPE_TWO_U32BIT("Tiger", Tiger, 24);
   // HANDLE_TYPE_NO_ARGS("Whirlpool", Whirlpool);

   // if(algo_name == "Parallel")
   //    {
   //    if(name.size() < 2)
   //       throw Invalid_Algorithm_Name(algo_spec);
   //    name.erase(name.begin());
   //    return new Parallel(name);
   //    }
   return 0;
   }

/*************************************************
* Look for an algorithm with this name           *
*************************************************/
MessageAuthenticationCode*
Default_Engine::find_mac(const std::string& algo_spec) const
   {
   std::vector<std::string> name = parse_algorithm_name(algo_spec);
   if(name.empty())
      return 0;
   const std::string algo_name = deref_alias(name[0]);

   // HANDLE_TYPE_ONE_STRING("CMAC", CMAC);
   HANDLE_TYPE_ONE_STRING("HMAC", HMAC);
   // HANDLE_TYPE_NO_ARGS("X9.19-MAC", ANSI_X919_MAC);

   return 0;
   }

/*************************************************
* Look for an algorithm with this name           *
*************************************************/
S2K* Default_Engine::find_s2k(const std::string& algo_spec) const
   {
   std::vector<std::string> name = parse_algorithm_name(algo_spec);
   if(name.empty())
      return 0;

   const std::string algo_name = deref_alias(name[0]);

   HANDLE_TYPE_ONE_STRING("PBKDF1", PKCS5_PBKDF1);
   HANDLE_TYPE_ONE_STRING("PBKDF2", PKCS5_PBKDF2);
   // HANDLE_TYPE_ONE_STRING("OpenPGP-S2K", OpenPGP_S2K);

   return 0;
   }

/*************************************************
* Look for an algorithm with this name           *
*************************************************/
BlockCipherModePaddingMethod*
Default_Engine::find_bc_pad(const std::string& algo_spec) const
   {
   std::vector<std::string> name = parse_algorithm_name(algo_spec);
   if(name.empty())
      return 0;

   const std::string algo_name = deref_alias(name[0]);

   HANDLE_TYPE_NO_ARGS("PKCS7",       PKCS7_Padding);
   HANDLE_TYPE_NO_ARGS("OneAndZeros", OneAndZeros_Padding);
   HANDLE_TYPE_NO_ARGS("X9.23",       ANSI_X923_Padding);
   HANDLE_TYPE_NO_ARGS("NoPadding",   Null_Padding);

   return 0;
   }

}
}
