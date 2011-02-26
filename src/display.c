#include "display.h"

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <jpeglib.h>

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t unused;
} rgb_t;

typedef struct {
  Display* display;
  int fd;
  int screen;
  Window window;
  Atom del_window;
  rgb_t* data;
  XImage* image;
} display_t;

#define X_OFFSET 0
#define Y_OFFSET 0
#define WIDTH 352
#define HEIGHT 288
#define BORDER_WIDTH 0

static void*
display_create (void* arg)
{
  display_t* display = malloc (sizeof (display_t));

  /* open connection with the server */
  display->display = XOpenDisplay (NULL);
  
  if (display->display == NULL) {
    fprintf (stderr, "Cannot open display\n");
    exit (EXIT_FAILURE);
  }
  
  display->fd = ConnectionNumber (display->display);
  display->screen = DefaultScreen (display->display);
  
  /* create window */
  display->window = XCreateSimpleWindow (display->display,
					 RootWindow (display->display, display->screen),
					 X_OFFSET, Y_OFFSET, WIDTH, HEIGHT, BORDER_WIDTH,
					 BlackPixel (display->display, display->screen),
					 WhitePixel (display->display, display->screen));
  
  // Prosses Window Close Event through event handler so XNextEvent does Not fail
  display->del_window = XInternAtom (display->display, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols (display->display, display->window, &display->del_window, 1);
  
  /* select kind of events we are interested in */
  XSelectInput (display->display, display->window, StructureNotifyMask | ExposureMask | KeyPressMask);
  
  /* map (show) the window */
  XMapWindow (display->display, display->window);
  XFlush (display->display);

  display->data = malloc (WIDTH * HEIGHT * sizeof (rgb_t));

  int depth = DefaultDepth (display->display, display->screen);
  Visual* visual = DefaultVisual (display->display, display->screen);

  /* I assume these.
   * Generlize later.
   */
  assert (depth == 24);
  assert (visual->red_mask == 0x00FF0000);
  assert (visual->green_mask == 0x0000FF00);
  assert (visual->blue_mask == 0x000000FF);

  display->image = XCreateImage (display->display,
				 CopyFromParent,
				 depth,
				 ZPixmap,
				 0,
				 (char *)display->data,
				 WIDTH,
				 HEIGHT,
				 32,
				 0);

  assert (1 == XInitImage (display->image));
  
  return display;
}

static void
display_system_input (void* state, void* param, bid_t bid)
{
  display_t* display = state;
  assert (display != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    assert (schedule_read_input (display->fd) == 0);
  }
}

static void
display_read_input (void* state, void* param, bid_t bid)
{
  display_t* display = state;
  assert (display != NULL);

  XEvent event;

  // Handle XEvents and flush the input 
  while (XPending (display->display)) {
    XNextEvent (display->display, &event);

    /* draw or redraw the window */
    if (event.type == Expose) {
      /* XFillRectangle (display->display, display->window, DefaultGC (display->display, display->screen), 20, 20, 10, 10); */
      XPutImage(display->display, 
		display->window,
		DefaultGC (display->display, display->screen),
		display->image,
		0, 0,
		0, 0,
		WIDTH, HEIGHT);
    }
    /* exit on key press */
    if (event.type == KeyPress)
      break;
    
    // Handle Windows Close Event
    if (event.type == ClientMessage)
      break;
  }
}

static void
init_source (struct jpeg_decompress_struct* cinfo)
{
  /* Do nothing. */
}

static boolean
fill_input_buffer (struct jpeg_decompress_struct* cinfo)
{
  assert (0);
}

static void
skip_input_data (struct jpeg_decompress_struct* cinfo, long numbytes)
{
  assert (0);
}

static void
term_source (struct jpeg_decompress_struct* cinfo)
{
  /* Do nothing. */
}

void
display_frame_in (void* state, void* param, bid_t bid)
{
  display_t* display = state;
  assert (display != NULL);

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  struct jpeg_source_mgr source = {
    .next_input_byte = buffer_read_ptr (bid),
    .bytes_in_buffer = buffer_size (bid),
    .init_source = init_source,
    .fill_input_buffer = fill_input_buffer,
    .skip_input_data = skip_input_data,
    .resync_to_restart = jpeg_resync_to_restart,
    .term_source = term_source,
  };

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  cinfo.src = &source;
  jpeg_read_header(&cinfo, TRUE);

  jpeg_start_decompress (&cinfo);

  assert (cinfo.out_color_components == 3);

  int row_stride = sizeof (JSAMPLE) * 3 * cinfo.output_width;
  JSAMPROW row_pointer[1];
  row_pointer[0] = malloc (row_stride * cinfo.rec_outbuf_height);
  while (cinfo.output_scanline < cinfo.output_height) {
    int num_lines = jpeg_read_scanlines (&cinfo, row_pointer, cinfo.rec_outbuf_height);
    int y = cinfo.output_scanline;
    int s;
    for (s = 0; s < num_lines; ++s, ++y) {
      int x;
      for (x = 0; x < cinfo.output_width; ++x) {
	display->data[y * WIDTH + x].red = row_pointer[0][s * row_stride + 3 * x];
	display->data[y * WIDTH + x].green = row_pointer[0][s * row_stride + 3 * x + 1];
	display->data[y * WIDTH + x].blue = row_pointer[0][s * row_stride + 3 * x + 2];
      }
    }
  }
  free (row_pointer[0]);

  jpeg_finish_decompress (&cinfo);

  jpeg_destroy_decompress (&cinfo);

  XPutImage(display->display,
  	    display->window,
  	    DefaultGC (display->display, display->screen),
  	    display->image,
  	    0, 0,
  	    0, 0,
  	    WIDTH, HEIGHT);
  XFlush (display->display);

}

/* /\* */
/*   Simple Xlib application drawing a box in a window. */
/*   To Compile: gcc -O2 -Wall -o test test.c -L /usr/X11R6/lib -lX11 -lm */
/* *\/ */

/* int */
/* func (void) { */
/*   Display* display; */
/*   int fd; */
/*   int screen; */
/*   Window window; */
/*   XEvent event; */
 
/*   /\* open connection with the server *\/ */
/*   display = XOpenDisplay (NULL); */

/*   if (display == NULL) { */
/*     printf ("Cannot open display\n"); */
/*     exit (1); */
/*   } */

/*   fd = ConnectionNumber (display); */
/*   printf ("fd = %d\n", fd); */

/*   screen = DefaultScreen (display); */
 
/*   /\* create window *\/ */
/*   window = XCreateSimpleWindow (display, */
/* 				RootWindow(display, screen), */
/* 				10, 10, 100, 100, 1, */
/* 				BlackPixel(display, screen), */
/* 				WhitePixel(display, screen)); */
  
/*   // Prosses Window Close Event through event handler so XNextEvent does Not fail */
/*   Atom delWindow = XInternAtom (display, "WM_DELETE_WINDOW", 0); */
/*   XSetWMProtocols(display, window, &delWindow, 1); */
 
/*   /\* select kind of events we are interested in *\/ */
/*   XSelectInput (display, window, ExposureMask | KeyPressMask); */
 
/*   /\* map (show) the window *\/ */
/*   XMapWindow (display, window); */
 
/*   /\* event loop *\/ */
/*   while (1) { */
/*     XNextEvent (display, &event); */
/*     /\* draw or redraw the window *\/ */
/*     if (event.type == Expose) { */
/*       XFillRectangle (display, window, DefaultGC (display, screen), 20, 20, 10, 10); */
/*     } */
/*     /\* exit on key press *\/ */
/*     if (event.type == KeyPress) */
/*       break; */
    
/*     // Handle Windows Close Event */
/*     if (event.type == ClientMessage) */
/*       break; */
/*   } */
 
/*   /\* destroy our window *\/ */
/*   XDestroyWindow (display, window); */
 
/*   /\* close connection to server *\/ */
/*   XCloseDisplay (display); */
 
/*   return 0; */
/* } */

static input_t display_inputs[] = {
  display_frame_in,
  NULL
};

descriptor_t display_descriptor = {
  .constructor = display_create,
  .system_input = display_system_input,
  .read_input = display_read_input,
  .inputs = display_inputs,
};
