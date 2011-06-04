#ifndef __minunit__h__
#define __minunit__h__

/* MinUnit
   http://www.jera.com/techinfo/jtns/jtn002.html

   License

   You may use the code in this tech note for any purpose, with the understanding that it comes with NO WARRANTY. 
 */

#define STRX(x) #x
#define STR(x) STRX(x)

#define mu_assert(test) do { if (!(test)) return __FILE__ ":" STR(__LINE__) ": assertion failed: " STR(test); } while (0)
#define mu_run_test(test) do { const char *message = test (); if (message) return message; } while (0)

#endif
