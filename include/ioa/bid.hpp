#ifndef __bid_hpp__
#define __bid_hpp__

namespace ioa {

  /*
    Binding ID.

    I found that associating a unique ID with each successful bind is good because the user doesn't have to re-supply the bind parameters when unbinding and the only possible error is that the binding doesn't exist.
   */
  typedef int bid_t;

}

#endif
