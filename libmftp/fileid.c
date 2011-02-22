#include "mftp.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void
mftp_FileID_init (mftp_FileID_t* f, uint8_t hash[HASH_SIZE], uint32_t size, uint32_t type)
{
  assert (f != NULL);
  assert (hash != NULL);
  assert (size > 0);
  
  memcpy (f->hash, hash, HASH_SIZE);
  f->size = size;
  f->type = type;
}

void
mftp_FileID_hostToNet (mftp_FileID_t* dst, const mftp_FileID_t* src)
{
  assert (dst != NULL);
  assert (src != NULL);

  memcpy (dst->hash, src->hash, HASH_SIZE);
  dst->size = htonl (src->size);
  dst->type = htonl (src->type);
}

void
mftp_FileID_netToHost (mftp_FileID_t* dst, const mftp_FileID_t* src)
{
  assert (dst != NULL);
  assert (src != NULL);

  memcpy (dst->hash, src->hash, HASH_SIZE);
  dst->size = ntohl (src->size);
  dst->type = ntohl (src->type);
}

int
mftp_FileID_cmp (const mftp_FileID_t* id1, const mftp_FileID_t* id2)
{
  return memcmp (id1, id2, sizeof (mftp_FileID_t));
}
