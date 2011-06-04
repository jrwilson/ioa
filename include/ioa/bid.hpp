#ifndef __bid_hpp__
#define __bid_hpp__

#include <ioa/environment.hpp>

#include <stdint.h>

#include <ioa/environment.hpp>

#include <stdint.h>

namespace ioa {

  /*
    Binding ID.

    I found that associating a unique ID with each successful bind is good because the user doesn't have to re-supply the bind parameters when unbinding and the only possible error is that the binding doesn't exist.
   */
#ifdef ENVIRONMENT64
  typedef int64_t bid_t;
#endif
#ifdef ENVIRONMENT32
  typedef int32_t bid_t;
#endif

}

#endif
