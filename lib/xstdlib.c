#include "xstdlib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void*
xmalloc (size_t size)
{
  void* ptr = malloc (size);
  if (ptr == NULL) {
    perror ("malloc");
    abort ();
  }
  memset (ptr, 0, size);
  return ptr;
}

void*
xrealloc (void* ptr, size_t size)
{
  void* newptr = realloc (ptr, size);
  if (newptr == NULL) {
    perror ("realloc");
    abort ();
  }
  return newptr;
}

void
xfree (void* ptr)
{
  free (ptr);
}
