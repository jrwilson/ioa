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

File::File (const char* name, uint32_t type)
{
  int fd = open (name, O_RDONLY);
  assert (fd != -1);

  struct stat stats;

  if (fstat (fd, &stats) >= 0) {
    m_original_size = stats.st_size;
  }
  else {
    perror ("fstat");
    exit (EXIT_FAILURE);
  }

  // Calculate the size after we pad with 0s to fall on a HASH_LENGTH boundary.
  m_padded_size = m_original_size + HASH_LENGTH - (m_original_size % HASH_LENGTH);
  assert ((m_padded_size % HASH_LENGTH) == 0);

  /*
    Calculate the size after hashing.

    Read pointer is named r.
    Write pointer is named w.
    The fragment index is i.
    The MD5 hash length is M.
    The fragment size is F.
    The padded length is L.

    r = 0 + F * i
    w = L + M * i

    Looking for first i such that r >= w
    F * i >= L + M * i
    F * i - M * i >= L
    i ( F - M) >= L
    i >= L / (F - M)
    i = ceil (L / (F - M) )
  */

  m_fragment_count = static_cast<uint32_t> (ceil (static_cast<double> (m_padded_size) / static_cast<double> (FRAGMENT_SIZE - HASH_LENGTH)));
  m_hashed_size = m_padded_size + HASH_LENGTH * (m_fragment_count - 1);
  assert ((m_hashed_size % HASH_LENGTH) == 0);

  m_fileid.original_length = m_original_size;
  m_fileid.hashed_length = m_hashed_size;
  m_fileid.type = type;

  std::cout << "The original length of the file is " << m_original_size << " bytes."<< std::endl;
  std::cout << "The padded length of the file is " << m_padded_size << " bytes."<< std::endl;
  std::cout << "The hashed length of the file is " << m_hashed_size << std::endl;
  std::cout << "The number of fragments is " << m_fragment_count << std::endl;

  // Allocate storage for the data and read it.
  m_data = new unsigned char[m_hashed_size];
  if (read (fd, m_data, m_original_size) != static_cast<ssize_t> (m_original_size)) {
    perror ("read");
    exit (EXIT_FAILURE);
  }
  close (fd);

  // Clear the padding.
  for (uint32_t idx = m_original_size; idx < m_padded_size; ++idx) {
    m_data[idx] = 0;
  }

  sha2_256 digester;

  uint32_t read_idx;
  uint32_t write_idx;

  for (read_idx = 0, write_idx = m_padded_size; read_idx < m_hashed_size; read_idx += FRAGMENT_SIZE, write_idx += HASH_LENGTH) {
    // How many bytes should we read?
    uint32_t read_size = std::min (FRAGMENT_SIZE, m_hashed_size - read_idx);
    // Update the digest.
    digester.update (m_data + read_idx, read_size);

    if (write_idx < m_hashed_size) {
      // Sample it without finalizing.
      unsigned char samp[HASH_LENGTH];
      digester.get (samp);
      memcpy (m_data + write_idx, samp, HASH_LENGTH);
    }
    else {
      // Finalize it then sample.
      digester.finalize ();
      unsigned char samp[HASH_LENGTH];
      digester.get (samp);
      memcpy (m_fileid.hash, samp, HASH_LENGTH);
    }
  }

  for (unsigned int idx = 0; idx < HASH_LENGTH; ++idx) {
    printf ("%02x", m_fileid.hash[idx]);
  }

  printf ("\n");
}

File::File (const fileID& f) :
  m_fileid (f),
  m_fragment_count ((f.hashed_length + FRAGMENT_SIZE - 1) / FRAGMENT_SIZE),
  m_original_size (f.original_length),
  m_padded_size (m_original_size + (HASH_LENGTH - (m_original_size % HASH_LENGTH))),
  m_hashed_size (f.hashed_length),
  m_data (new unsigned char[m_hashed_size])
{ }

File::File (const File& other) :
  m_fileid (other.m_fileid),
  m_fragment_count (other.m_fragment_count),
  m_original_size (other.m_original_size),
  m_padded_size (other.m_padded_size),
  m_hashed_size (other.m_hashed_size)
{
  m_data = new unsigned char[m_hashed_size];
  memcpy (m_data, other.m_data, m_hashed_size);
}

File::~File () {
  delete [] m_data;
}
