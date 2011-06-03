#ifndef __environment_hpp__
#define __environment_hpp__

// Determines whether 32 or 64 bit CPU

#if __x86_64__ || __ppc64__
   #define ENVIRONMENT64
#else
   #define ENVIRONMENT32
#endif

#endif

