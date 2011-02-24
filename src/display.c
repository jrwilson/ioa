#include "display.h"

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
  Display* display;
  int fd;
  int screen;
  Window window;
  Atom del_window;
} display_t;

#define X_OFFSET 10
#define Y_OFFSET 10
#define WIDTH 320
#define HEIGHT 240
#define BORDER_WIDTH 1

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
    assert (schedule_alarm_input (1, 0) == 0);
  }
  else {
    assert (0);
  }
}

static void
display_alarm_input (void* state, void* param, bid_t bid)
{
  display_t* display = state;
  assert (display != NULL);

  XDrawPoint (display->display,
  	      display->window,
  	      DefaultGC (display->display, display->screen),
  	      rand () % WIDTH, rand () % HEIGHT);
  XFlush (display->display);

  assert (schedule_alarm_input (0, 1000) == 0);
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
      XFillRectangle (display->display, display->window, DefaultGC (display->display, display->screen), 20, 20, 10, 10);
    }
    /* exit on key press */
    if (event.type == KeyPress)
      break;
    
    // Handle Windows Close Event
    if (event.type == ClientMessage)
      break;
  }
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

descriptor_t display_descriptor = {
  .constructor = display_create,
  .system_input = display_system_input,
  .alarm_input = display_alarm_input,
  .read_input = display_read_input,
};
