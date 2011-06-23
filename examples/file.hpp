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
private:
  fileID m_fileid;
  unsigned char* m_data;
public:
  File (const char*,
	const uint32_t);
  File (const fileID& f);
  File (const File& other);
  ~File ();

  const fileID& get_fileid () const {
    return m_fileid;
  }

  unsigned char* get_data_ptr () {
      return m_data;
  }
};

#endif
