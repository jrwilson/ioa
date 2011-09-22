#ifndef __profile_hpp__
#define __profile_hpp__

#ifdef PROFILE
#include <ioa/scheduler_interface.hpp>
namespace ioa {
  extern scheduler_interface* scheduler;
}
#define BEGIN_SYS_CALL do { if (scheduler != 0) { scheduler->begin_sys_call (); } } while (0);
#define END_SYS_CALL do { if (scheduler != 0) { scheduler->end_sys_call (); } } while (0);
#else
#define BEGIN_SYS_CALL do { if (scheduler != 0) { } while (0);
#define END_SYS_CALL do { if (scheduler != 0) { } while (0);
#endif

#endif
