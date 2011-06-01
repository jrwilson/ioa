#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>             /* getopt_long() */

#include "camera.h"

static void
usage                           (FILE *                 fp,
                                 int                    argc,
                                 char **                argv)
{
  fprintf (fp,
	   "Usage: %s [options]\n\n"
	   "Options:\n"
	   "-d | --device name   Video device name [/dev/video]\n"
	   "-h | --help          Print this message\n"
	   "",
	   argv[0]);
}

static const char short_options [] = "d:hmru";

static const struct option
long_options [] = {
  { "device",     required_argument,      NULL,           'd' },
  { "help",       no_argument,            NULL,           'h' },
  { 0, 0, 0, 0 }
};

int
main (int argc, char** argv)
{
  camera_create_arg_t arg = {
    .dev_name = "/dev/video",
  };

  for (;;) {
    int index;
    int c;
                
    c = getopt_long (argc, argv,
		     short_options, long_options,
		     &index);

    if (-1 == c)
      break;

    switch (c) {
    case 0: /* getopt_long() flag */
      break;

    case 'd':
      arg.dev_name = optarg;
      break;

    case 'h':
      usage (stdout, argc, argv);
      exit (EXIT_SUCCESS);

    default:
      usage (stderr, argc, argv);
      exit (EXIT_FAILURE);
    }
  }

  ueioa_run (&camera_descriptor, &arg, 1);
  exit (EXIT_SUCCESS);
}
