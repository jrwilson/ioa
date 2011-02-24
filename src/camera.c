/*
  Derived from
  http://v4l2spec.bytesex.org/spec/a16706.htm
*/

/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 */

#include "camera.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <stdlib.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#define BUFFER_COUNT 4

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
  bid_t bid;
  void *                  start;
};

static void
errno_exit                      (const char *           s)
{
  fprintf (stderr, "%s error %d, %s\n",
	   s, errno, strerror (errno));

  exit (EXIT_FAILURE);
}

static int
xioctl                          (int                    fd,
                                 int                    request,
                                 void *                 arg)
{
  int r;

  do r = ioctl (fd, request, arg);
  while (-1 == r && EINTR == errno);

  return r;
}

/* static void */
/* stop_capturing                  (void) */
/* { */
/*   enum v4l2_buf_type type; */

/*   switch (io) { */
/*   case IO_METHOD_READ: */
/*     /\* Nothing to do. *\/ */
/*     break; */

/*   case IO_METHOD_MMAP: */
/*   case IO_METHOD_USERPTR: */
/*     type = V4L2_BUF_TYPE_VIDEO_CAPTURE; */

/*     if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type)) */
/*       errno_exit ("VIDIOC_STREAMOFF"); */

/*     break; */
/*   } */
/* } */

/* static void */
/* uninit_device                   (void) */
/* { */
/*   unsigned int i; */

/*   switch (io) { */
/*   case IO_METHOD_READ: */
/*     free (buffers[0].start); */
/*     break; */

/*   case IO_METHOD_MMAP: */
/*     for (i = 0; i < n_buffers; ++i) */
/*       if (-1 == munmap (buffers[i].start, buffers[i].length)) */
/* 	errno_exit ("munmap"); */
/*     break; */

/*   case IO_METHOD_USERPTR: */
/*     for (i = 0; i < n_buffers; ++i) */
/*       free (buffers[i].start); */
/*     break; */
/*   } */

/*   free (buffers); */
/* } */

/* static void */
/* close_device                    (void) */
/* { */
/*   if (-1 == close (fd)) */
/*     errno_exit ("close"); */

/*   fd = -1; */
/* } */

/* int */
/* main                            (int                    argc, */
/*                                  char **                argv) */
/* { */
/*   stop_capturing (); */

/*   uninit_device (); */

/*   close_device (); */

/*   exit (EXIT_SUCCESS); */

/*   return 0; */
/* } */

typedef struct {
  int fd;
  size_t page_size;
  size_t buffer_size;
  struct buffer* buffers;
  bidq_t* bidq;
} camera_t;

static void*
camera_create (void* a)
{
  camera_create_arg_t* arg = a;
  assert (arg != NULL);

  camera_t* camera = malloc (sizeof (camera_t));

  /* Open device. */
  struct stat st;
  
  if (-1 == stat (arg->dev_name, &st)) {
    fprintf (stderr, "Cannot identify '%s': %d, %s\n",
	     arg->dev_name, errno, strerror (errno));
    exit (EXIT_FAILURE);
  }
  
  if (!S_ISCHR (st.st_mode)) {
    fprintf (stderr, "%s is no device\n", arg->dev_name);
    exit (EXIT_FAILURE);
  }

  camera->fd = open (arg->dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

  if (-1 == camera->fd) {
    fprintf (stderr, "Cannot open '%s': %d, %s\n",
	     arg->dev_name, errno, strerror (errno));
    exit (EXIT_FAILURE);
  }

  /* Init device. */
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if (-1 == xioctl (camera->fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fprintf (stderr, "%s is no V4L2 device\n",
	       arg->dev_name);
      exit (EXIT_FAILURE);
    } else {
      errno_exit ("VIDIOC_QUERYCAP");
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf (stderr, "%s is no video capture device\n",
	     arg->dev_name);
    exit (EXIT_FAILURE);
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    fprintf (stderr, "%s does not support streaming i/o\n",
	     arg->dev_name);
    exit (EXIT_FAILURE);
  }
  
  /* Select video input, video standard and tune here. */

  CLEAR (cropcap);
  
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if (0 == xioctl (camera->fd, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */
    
    if (-1 == xioctl (camera->fd, VIDIOC_S_CROP, &crop)) {
      switch (errno) {
      case EINVAL:
	/* Cropping not supported. */
	break;
      default:
	/* Errors ignored. */
	break;
      }
    }
  } else {
    /* Errors ignored. */
  }

  CLEAR (fmt);

  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = 640;
  fmt.fmt.pix.height      = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

  if (-1 == xioctl (camera->fd, VIDIOC_S_FMT, &fmt))
    errno_exit ("VIDIOC_S_FMT");
  
  /* Note VIDIOC_S_FMT may change width and height. */
  
  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  struct v4l2_requestbuffers req;

  camera->page_size = getpagesize ();
  camera->buffer_size = (fmt.fmt.pix.sizeimage + camera->page_size - 1) & ~(camera->page_size - 1);
  
  CLEAR (req);
  
  req.count               = BUFFER_COUNT;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_USERPTR;

  if (-1 == xioctl (camera->fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      fprintf (stderr, "%s does not support "
	       "user pointer i/o\n", arg->dev_name);
      exit (EXIT_FAILURE);
    } else {
      errno_exit ("VIDIOC_REQBUFS");
    }
  }

  camera->buffers = calloc (BUFFER_COUNT, sizeof (*camera->buffers));
  
  if (!camera->buffers) {
    fprintf (stderr, "Out of memory\n");
    exit (EXIT_FAILURE);
  }

  unsigned int i;
  for (i = 0; i < BUFFER_COUNT; ++i) {
    camera->buffers[i].bid = buffer_alloc_aligned (camera->buffer_size, camera->page_size);
    camera->buffers[i].start = buffer_write_ptr (camera->buffers[i].bid);
    
    if (!camera->buffers[i].start) {
      fprintf (stderr, "Out of memory\n");
      exit (EXIT_FAILURE);
    }
  }

  camera->bidq = bidq_create ();

  return camera;
}

static void
camera_system_input (void* state, void* param, bid_t bid)
{
  camera_t* camera = state;
  assert (camera != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    /* Start capturing. */

    unsigned int i;
    enum v4l2_buf_type type;
    
    for (i = 0; i < BUFFER_COUNT; ++i) {
      struct v4l2_buffer buf;
      
      CLEAR (buf);
      
      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_USERPTR;
      buf.index       = i;
      buf.m.userptr   = (unsigned long) camera->buffers[i].start;
      buf.length      = camera->buffer_size;
      
      if (-1 == xioctl (camera->fd, VIDIOC_QBUF, &buf))
	errno_exit ("VIDIOC_QBUF");
    }
    
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (-1 == xioctl (camera->fd, VIDIOC_STREAMON, &type))
      errno_exit ("VIDIOC_STREAMON");
    
    assert (schedule_read_input (camera->fd) == 0);
  }
}

static void
camera_read_input (void* state, void* param, bid_t bid)
{
  camera_t* camera = state;
  assert (camera != NULL);

  struct v4l2_buffer buf;
  unsigned int i;

  CLEAR (buf);
  
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_USERPTR;
  
  if (-1 == xioctl (camera->fd, VIDIOC_DQBUF, &buf)) {
    switch (errno) {
    case EAGAIN:
      assert (schedule_read_input (camera->fd) == 0);
      return;
      
    case EIO:
      /* Could ignore EIO, see spec. */
      
      /* fall through */
      
    default:
      errno_exit ("VIDIOC_DQBUF");
    }
  }

  for (i = 0; i < BUFFER_COUNT; ++i) {
    if (buf.m.userptr == (unsigned long) camera->buffers[i].start
	&& buf.length == camera->buffer_size) {
      break;
    }
  }
  
  assert (i < BUFFER_COUNT);

  /* Put the frame on the outgoing queue. */
  bidq_push_back (camera->bidq, camera->buffers[i].bid);
  assert (schedule_output (camera_frame_out, NULL) == 0);

  camera->buffers[i].bid = buffer_alloc_aligned (camera->buffer_size, camera->page_size);
  camera->buffers[i].start = buffer_write_ptr (camera->buffers[i].bid);
  buf.m.userptr = (unsigned long)camera->buffers[i].start;

  if (-1 == xioctl (camera->fd, VIDIOC_QBUF, &buf))
    errno_exit ("VIDIOC_QBUF");

  assert (schedule_read_input (camera->fd) == 0);
}

bid_t
camera_frame_out (void* state, void* param)
{
  camera_t* camera = state;
  assert (camera != NULL);

  if (!bidq_empty (camera->bidq)) {
    /* Get the bid. */
    bid_t bid = bidq_front (camera->bidq);
    bidq_pop_front (camera->bidq);

    /* Go again. */
    assert (schedule_output (camera_frame_out, NULL) == 0);

    return bid;
  }
  else {
    return -1;
  }
}

static output_t camera_outputs[] = {
  camera_frame_out,
  NULL
};

descriptor_t camera_descriptor = {
  .constructor = camera_create,
  .system_input = camera_system_input,
  .read_input = camera_read_input,
  .outputs = camera_outputs,
};
