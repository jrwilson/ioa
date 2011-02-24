#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>             /* getopt_long() */
#include <assert.h>

#include "camera.h"
#include "display.h"

typedef struct {
  manager_t* manager;

  camera_create_arg_t camera_arg;
  aid_t camera;
  aid_t display;
} composer_t;

typedef struct {
  char* dev_name;
} composer_create_arg_t;

static void*
composer_create (void* a)
{
  composer_create_arg_t* arg = a;
  assert (arg != NULL);

  composer_t* composer = malloc (sizeof (composer_t));
  composer->camera_arg.dev_name = arg->dev_name;

  composer->manager = manager_create ();

  manager_child_add (composer->manager, &composer->camera, &camera_descriptor, &composer->camera_arg);
  manager_child_add (composer->manager, &composer->display, &display_descriptor, NULL);
  manager_composition_add (composer->manager,
			   &composer->camera, camera_frame_out, NULL,
			   &composer->display, display_frame_in, NULL);

  return composer;
}

static bid_t composer_system_output (void* state, void* param);

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (composer->manager, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  assert (state != NULL);
  composer_t* composer = state;

  return manager_action (composer->manager);
}

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
};

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
  composer_create_arg_t arg = {
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

  ueioa_run (&composer_descriptor, &arg, 1);
  exit (EXIT_SUCCESS);
}
