#ifndef __xstdlib_h__
#define __xstdlib_h__

#include <stddef.h>

void* xmalloc (size_t);
void* xrealloc (void*, size_t);
void xfree (void*);

#endif /* __xstdlib_h__ */
