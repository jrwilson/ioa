#include "mftp.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

void
mftp_Announcement_init (mftp_Message_t* m, const mftp_FileID_t* fileid)
{
  assert (m != NULL);
  assert (fileid != NULL);

  m->header.type = ANNOUNCEMENT;
  memcpy (&m->announcement.fileid, fileid, sizeof (mftp_FileID_t));
}

void
mftp_Match_init (mftp_Message_t* m, const mftp_FileID_t* fileid1, const mftp_FileID_t* fileid2)
{
  assert (m != NULL);
  assert (fileid1 != NULL);
  assert (fileid2 != NULL);
  assert (fileid1->type <= fileid2->type);

  m->header.type = MATCH;
  memcpy (&m->match.fileid1, fileid1, sizeof (mftp_FileID_t));
  memcpy (&m->match.fileid2, fileid2, sizeof (mftp_FileID_t));
}

void
mftp_Request_init (mftp_Message_t* m, const mftp_FileID_t* fileid, uint32_t offset, uint32_t size)
{
  assert (m != NULL);
  assert (fileid != NULL);
  assert (ALIGNED (offset));
  assert (size > 0);
  assert (offset + size <= fileid->size);
  assert (ALIGNED (size) || (offset + size == fileid->size));

  m->header.type = REQUEST;
  memcpy (&m->request.fileid, fileid, sizeof (mftp_FileID_t));
  m->request.offset = offset;
  m->request.size = size;
}

void
mftp_Fragment_init (mftp_Message_t* m, const mftp_FileID_t* fileid, uint32_t offset, uint32_t size, uint8_t data[FRAGMENT_SIZE])
{
  assert (m != NULL);
  assert (fileid != NULL);
  assert (ALIGNED (offset));
  assert (size > 0 && size <= FRAGMENT_SIZE);
  assert (offset + size <= fileid->size);
  assert (size == FRAGMENT_SIZE || offset + size == fileid->size);
  assert (data != NULL);

  m->header.type = FRAGMENT;
  memcpy (&m->fragment.fileid, fileid, sizeof (mftp_FileID_t));
  m->fragment.offset = offset;
  m->fragment.size = size;
  memcpy (m->fragment.data, data, size);
}

static void
Header_hostToNet (mftp_Header_t* dst, const mftp_Header_t* src)
{
  dst->type = htonl (src->type);
}

static void
Announcement_hostToNet (mftp_Announcement_t* dst, const mftp_Announcement_t* src)
{
  mftp_FileID_hostToNet (&dst->fileid, &src->fileid);
}

static void
Match_hostToNet (mftp_Match_t* dst, const mftp_Match_t* src)
{
  mftp_FileID_hostToNet (&dst->fileid1, &src->fileid1);
  mftp_FileID_hostToNet (&dst->fileid2, &src->fileid2);
}

static void
Request_hostToNet (mftp_Request_t* dst, const mftp_Request_t* src)
{
  mftp_FileID_hostToNet (&dst->fileid, &src->fileid);
  dst->offset = htonl (src->offset);
  dst->size = htonl (src->size);
}

static void
Fragment_hostToNet (mftp_Fragment_t* dst, const mftp_Fragment_t* src)
{
  mftp_FileID_hostToNet (&dst->fileid, &src->fileid);
  dst->offset = htonl (src->offset);
  dst->size = htonl (src->size);
  memcpy (dst->data, src->data, src->size);
}

void
mftp_Message_hostToNet (mftp_Message_t* dst, const mftp_Message_t* src)
{
  assert (dst != NULL);
  assert (src != NULL);

  switch (src->header.type) {
  case ANNOUNCEMENT:
    Announcement_hostToNet (&dst->announcement, &src->announcement);
    break;
  case MATCH:
    Match_hostToNet (&dst->match, &src->match);
    break;
  case REQUEST:
    Request_hostToNet (&dst->request, &src->request);
    break;
  case FRAGMENT:
    Fragment_hostToNet (&dst->fragment, &src->fragment);
    break;
  }

  Header_hostToNet (&dst->header, &src->header);
}

static int
Header_netToHost (mftp_Header_t* dst, const mftp_Header_t* src, int bytesRemaining)
{
  if (bytesRemaining < sizeof (mftp_Header_t)) {
    return -1;
  }

  dst->type = ntohl (src->type);
  return bytesRemaining - sizeof (mftp_Header_t);
}

static int
Announcement_netToHost (mftp_Announcement_t* dst, const mftp_Announcement_t* src, int bytesRemaining)
{
  if (bytesRemaining < sizeof (mftp_Announcement_t)) {
    return -1;
  }

  mftp_FileID_netToHost (&dst->fileid, &src->fileid);
  return bytesRemaining - sizeof (mftp_Announcement_t);
}

static int
Match_netToHost (mftp_Match_t* dst, const mftp_Match_t* src, int bytesRemaining)
{
  if (bytesRemaining < sizeof (mftp_Match_t)) {
    return -1;
  }

  mftp_FileID_netToHost (&dst->fileid1, &src->fileid1);
  mftp_FileID_netToHost (&dst->fileid2, &src->fileid2);

  if (dst->fileid1.type > src->fileid2.type) {
    return -1;
  }
  return bytesRemaining - sizeof (mftp_Match_t);
}

static int
Request_netToHost (mftp_Request_t* dst, const mftp_Request_t* src, int bytesRemaining)
{
  if (bytesRemaining < sizeof (mftp_Request_t)) {
    return -1;
  }

  mftp_FileID_netToHost (&dst->fileid, &src->fileid);
  dst->offset = ntohl (src->offset);
  dst->size = ntohl (src->size);

  if (!(ALIGNED (dst->offset) &&
	(dst->size > 0) &&
	(dst->offset + dst->size <= dst->fileid.size) &&
	(ALIGNED (dst->size) || (dst->offset + dst->size == dst->fileid.size)))) {
    return -1;
  }

  return bytesRemaining - sizeof (mftp_Request_t);
}

static int
Fragment_netToHost (mftp_Fragment_t* dst, const mftp_Fragment_t* src, int bytesRemaining)
{
  if (bytesRemaining < sizeof (mftp_Fragment_t) - FRAGMENT_SIZE) {
    return -1;
  }

  mftp_FileID_netToHost (&dst->fileid, &src->fileid);
  dst->offset = ntohl (src->offset);
  dst->size = ntohl (src->size);

  if (!(ALIGNED (dst->offset)  &&
	(dst->size > 0 && dst->size <= FRAGMENT_SIZE) &&
	(dst->offset + dst->size <= dst->fileid.size) &&
	(dst->size == FRAGMENT_SIZE || dst->offset + dst->size == dst->fileid.size))) {
    return -1;
  }

  memcpy (dst->data, src->data, dst->size);

  return bytesRemaining - sizeof (mftp_Fragment_t) + FRAGMENT_SIZE - dst->size;
}

int
mftp_Message_netToHost (mftp_Message_t* dst, const mftp_Message_t* src, int bytesRemaining)
{
  assert (dst != NULL);
  assert (src != NULL);

  if ((bytesRemaining = Header_netToHost (&dst->header, &src->header, bytesRemaining)) < 0) {
    return -1;
  }

  switch (dst->header.type) {
  case ANNOUNCEMENT:
    return Announcement_netToHost (&dst->announcement, &src->announcement, bytesRemaining);
    break;
  case MATCH:
    return Match_netToHost (&dst->match, &src->match, bytesRemaining);
    break;
  case REQUEST:
    return Request_netToHost (&dst->request, &src->request, bytesRemaining);
    break;
  case FRAGMENT:
    return Fragment_netToHost (&dst->fragment, &src->fragment, bytesRemaining);
    break;
  }

  return -1;
}
