#include "mftp.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

mftp_File_t*
mftp_File_create_buffer (const void* buffer, uint32_t size, uint32_t type)
{
  uint8_t hash[HASH_SIZE];
  MD5 ((const unsigned char*)buffer, size, hash);

  mftp_File_t* file = (mftp_File_t*)malloc (sizeof (mftp_File_t) + size);
  mftp_FileID_init (&file->fileid, hash, size, type);
  memcpy (file->data, buffer, size);
  return file;
}

mftp_File_t*
mftp_File_create_empty (const mftp_FileID_t* fileid)
{
  assert (fileid != NULL);

  mftp_File_t* file = (mftp_File_t*)malloc (sizeof (mftp_File_t) + fileid->size);
  memcpy (&file->fileid, fileid, sizeof (mftp_FileID_t));
  memset (file->data, 0, fileid->size);

  return file;
}

void
mftp_File_destroy (mftp_File_t* file)
{
  assert (file != NULL);
  free (file);
}
