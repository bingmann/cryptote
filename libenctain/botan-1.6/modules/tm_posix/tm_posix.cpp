/*************************************************
* POSIX Timer Source File                        *
* (C) 1999-2007 The Botan Project                *
*************************************************/

#include "botan-1.6/include/tm_posix.h"
#include "botan-1.6/include/util.h"

#ifdef BOTAN_EXT_TIMER_POSIX

#ifndef _POSIX_C_SOURCE
  #define _POSIX_C_SOURCE 199309
#endif

#include <time.h>

#ifndef CLOCK_REALTIME
  #define CLOCK_REALTIME 0
#endif

namespace Enctain {
namespace Botan {

/*************************************************
* Get the timestamp                              *
*************************************************/
u64bit POSIX_Timer::clock() const
   {
   struct timespec tv;
   clock_gettime(CLOCK_REALTIME, &tv);
   return combine_timers(tv.tv_sec, tv.tv_nsec, 1000000000);
   }

}
}

#endif // BOTAN_EXT_TIMER_POSIX
