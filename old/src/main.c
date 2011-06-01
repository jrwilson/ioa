#include <stdlib.h>

#include <ueioa.h>
#include "composer.h"

int
main (int argc, char** argv)
{
  ueioa_run (&composer_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
