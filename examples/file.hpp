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

#define HASH_LENGTH 32U
#define FRAGMENT_SIZE 512U

#include <string>
#include <stdint.h>

struct fileID {
    unsigned char hash[HASH_LENGTH];
    uint32_t type;
    uint32_t original_length;
    uint32_t hashed_length;
};

class File {
private:
  std::string m_fname;
  uint32_t m_original_size;
  uint32_t m_padded_size;
  uint32_t m_hashed_size;
  uint32_t m_fragment_count;
  unsigned char* m_data;

public:
  File (const char*, uint32_t);
  ~File ();

  fileID m_fileid;

  unsigned char* get_data_ptr () {
      return m_data;
  }
};

#endif
