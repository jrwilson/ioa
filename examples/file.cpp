#include "file.hpp"
#include "sha2_256.hpp"

#include <math.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <stdio.h>

File::File (const char* name,
	    const uint32_t type)
{
  int fd = open (name, O_RDONLY);
  assert (fd != -1);

  uint32_t original_length = 0;
  struct stat stats;

  if (fstat (fd, &stats) >= 0) {
    original_length = stats.st_size;
  }
  else {
    perror ("fstat");
    exit (EXIT_FAILURE);
  }

  m_fileid.length = original_length;
  m_fileid.type = type;

  const uint32_t final_length = m_fileid.final_length ();
  const uint32_t padding = m_fileid.padding ();

  /*
    Layout of the file will be
    +-------------------------+-------------+---------------------------------+
    |  DATA (original_length) | 0 (padding) | DIGEST (final_length - padding) |
    +-------------------------+-------------+---------------------------------+
  */

  // Allocate storage for the data and read it.
  m_data = new unsigned char[final_length];
  if (read (fd, m_data, original_length) != static_cast<ssize_t> (original_length)) {
    perror ("read");
    exit (EXIT_FAILURE);
  }
  close (fd);

  // Clear the padding.
  for (uint32_t idx = 0; idx < padding; ++idx) {
    m_data[original_length + idx] = 0;
  }

  sha2_256 digester;

  uint32_t read_idx;
  uint32_t write_idx;

  for (read_idx = 0, write_idx = original_length + padding;
       read_idx < final_length;
       read_idx += FRAGMENT_SIZE, write_idx += HASH_SIZE) {
    // Update the digest.
    digester.update (m_data + read_idx, FRAGMENT_SIZE);

    if (write_idx < final_length) {
      // Sample it without finalizing.
      unsigned char samp[HASH_SIZE];
      digester.get (samp);
      memcpy (m_data + write_idx, samp, HASH_SIZE);
    }
    else {
      // Finalize it then sample.
      digester.finalize ();
      unsigned char samp[HASH_SIZE];
      digester.get (samp);
      memcpy (m_fileid.hash, samp, HASH_SIZE);
    }
  }

  for (unsigned int idx = 0; idx < HASH_SIZE; ++idx) {
    printf ("%02x", m_fileid.hash[idx]);
  }

  printf ("\n");
}

File::File (const fileID& f) :
  m_fileid (f),
  m_data (new unsigned char[m_fileid.final_length ()])
{ }

File::File (const File& other) :
  m_fileid (other.m_fileid)
{
  const uint32_t final_length = m_fileid.final_length ();
  m_data = new unsigned char[final_length];
  memcpy (m_data, other.m_data, final_length);
}

File::~File () {
  delete [] m_data;
}
