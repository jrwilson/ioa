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

namespace mftp {

  file::file (const char* name,
	      const uint32_t type) :
    m_start_idx (0)
  {
    int fd = open (name, O_RDONLY);
    assert (fd != -1);

    struct stat stats;

    if (fstat (fd, &stats) >= 0) {
      m_mfileid.set_length (stats.st_size);
    }
    else {
      perror ("fstat");
      exit (EXIT_FAILURE);
    }

    m_mfileid.set_type (type);

    // We have all of the file.
    m_have_count = m_mfileid.get_fragment_count ();
    m_have.resize (m_mfileid.get_fragment_count ());
    for (uint32_t idx = 0; idx < m_mfileid.get_fragment_count (); ++idx) {
      m_have[idx] = true;
    }

    /*
      Layout of the file will be
      +-------------------------+-------------+---------------------------------+
      |  DATA (original_length) | 0 (padding) | DIGEST (final_length - padding) |
      +-------------------------+-------------+---------------------------------+
    */

    // Allocate storage for the data and read it.
    m_data = new unsigned char[m_mfileid.get_final_length ()];
    if (read (fd, m_data, m_mfileid.get_original_length ()) != static_cast<ssize_t> (m_mfileid.get_original_length ())) {
      perror ("read");
      exit (EXIT_FAILURE);
    }
    close (fd);

    // Clear the padding.
    for (uint32_t idx = 0; idx < m_mfileid.get_padding (); ++idx) {
      m_data[m_mfileid.get_original_length () + idx] = 0;
    }

    sha2_256 digester;

    uint32_t read_idx;
    uint32_t write_idx;

    for (read_idx = 0, write_idx = m_mfileid.get_original_length () + m_mfileid.get_padding ();
	 read_idx < m_mfileid.get_final_length ();
	 read_idx += FRAGMENT_SIZE, write_idx += HASH_SIZE) {
      // Update the digest.
      digester.update (m_data + read_idx, FRAGMENT_SIZE);

      if (write_idx < m_mfileid.get_final_length ()) {
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
	m_mfileid.set_hash (samp);
      }
    }
  }

  file::file (const fileid& f) :
    m_mfileid (f),
    m_data (new unsigned char[m_mfileid.get_final_length ()]),
    m_have (m_mfileid.get_fragment_count ()),
    m_have_count (0),
    m_start_idx (0)
  { }

  file::file (const file& other) :
    m_mfileid (other.m_mfileid),
    m_have (other.m_have),
    m_have_count (other.m_have_count),
    m_start_idx (0)
  {
    const uint32_t final_length = m_mfileid.get_final_length ();
    m_data = new unsigned char[final_length];
    memcpy (m_data, other.m_data, final_length);
  }

  file::~file () {
    delete [] m_data;
  }

  const mfileid& file::get_mfileid () const {
    return m_mfileid;
  }
  
  unsigned char* file::get_data_ptr () {
    return m_data;
  }

  bool file::complete () const {
    return m_have_count == m_mfileid.get_fragment_count ();
  }

  bool file::empty () const {
    return m_have_count == 0;
  }

  bool file::have (const uint32_t offset) const {
    assert (offset % FRAGMENT_SIZE == 0);
    assert (offset < m_mfileid.get_final_length ());
    return m_have[offset / FRAGMENT_SIZE];
  }

  void file::write_chunk (const uint32_t offset,
			  const uint8_t* data) {
    assert (offset % FRAGMENT_SIZE == 0);
    assert (offset < m_mfileid.get_final_length ());

    const uint32_t idx = offset / FRAGMENT_SIZE;
    if (!m_have[idx]) {
      // Copy it.
      memcpy (m_data + offset, data, FRAGMENT_SIZE);
      // Now we have it.
      m_have[idx] = true;
      ++m_have_count;
	
      // Printed out the file to make sure it was a valid transfer.
      if (m_have_count == m_mfileid.get_fragment_count ()) {
        std::string st (reinterpret_cast<char*> (m_data), m_mfileid.get_original_length ());
        std::cout << st << std::endl;
      }
      // TODO:  Validate it.
    }
  }

  std::pair<uint32_t, uint32_t> file::get_next_range () {
    // We shouldn't have all the fragments.
    assert (m_have_count != m_mfileid.get_fragment_count ());
      
    // Move start until we don't have the fragment.
    for (;
	 m_have[m_start_idx] == true;
	 m_start_idx = (m_start_idx + 1) % m_mfileid.get_fragment_count ())
      ;;
      
    // Move end until we do have a fragment or reach the end.
    uint32_t end_idx;
    for (end_idx = m_start_idx + 1;
	 end_idx < m_mfileid.get_fragment_count () && m_have[end_idx] == false;
	 ++end_idx)
      ;;
      
    // Request range [m_start_idx, end_idx).
    std::pair<uint32_t, uint32_t> retval (m_start_idx * FRAGMENT_SIZE, end_idx * FRAGMENT_SIZE);
    m_start_idx = end_idx;

    return retval;
  }

  uint32_t file::get_random_index () const {
    assert (m_have_count != 0);
    uint32_t rf = rand () % m_mfileid.get_fragment_count ();
    for (; !m_have[rf]; rf = (rf + 1) % m_mfileid.get_fragment_count ()) { }
    return rf;  
  }

}
