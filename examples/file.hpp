/*
  open the file (constructor with file name or id)
  figures out how big the file is
  how big the file is with hashing, which is reliant on hash length and fragment size
  query the various sizes that we need
  read/write chunks of data

  Data array file

  init(), update(), and final()
*/

#ifndef __file_hpp__
#define __file_hpp__

#include "mftp.hpp"

class File {
public:
  fileID m_fileid;
  uint32_t m_fragment_count;
private:
  uint32_t m_original_size;
  uint32_t m_padded_size;
  uint32_t m_hashed_size;
  unsigned char* m_data;
public:
  File (const char*, uint32_t);
  File (const fileID& f);
  File (const File& other);
  ~File ();

  unsigned char* get_data_ptr () {
      return m_data;
  }
};

#endif
