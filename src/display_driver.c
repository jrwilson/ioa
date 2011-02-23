#include <stdlib.h>

#include "display.h"

int
main (int argc, char** argv)
{
  ueioa_run (&display_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
