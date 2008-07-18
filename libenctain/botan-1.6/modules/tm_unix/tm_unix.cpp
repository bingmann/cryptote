/*************************************************
* Unix Timer Source File                         *
* (C) 1999-2007 The Botan Project                *
*************************************************/

#include "botan-1.6/include/tm_unix.h"
#include "botan-1.6/include/util.h"
#include <sys/time.h>

namespace Enctain {
namespace Botan {

/*************************************************
* Get the timestamp                              *
*************************************************/
u64bit Unix_Timer::clock() const
   {
   struct timeval tv;
   gettimeofday(&tv, 0);
   return combine_timers(tv.tv_sec, tv.tv_usec, 1000000);
   }

}
}
