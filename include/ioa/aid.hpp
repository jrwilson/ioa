#ifndef __aid_hpp__
#define __aid_hpp__

#include <ioa/environment.hpp>

#include <stdint.h>

namespace ioa {
  
  // Automaton ID.
  #ifdef ENVIRONMENT64
    typedef int64_t aid_t;
  #elif ENVIRONMENT32
    typedef int32_t aid_t;
  #endif

}

#endif

