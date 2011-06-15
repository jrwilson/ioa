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

#include <string>

#define MD5_HASH_LENGTH 16
#define FRAGMENT_SIZE 512

class File {
private:
  char* m_data;
  int m_frag_index;
  int m_fd;
  long m_fsize;
  std::string m_fname;

  long m_original_size;
  long m_padded_size;
  long m_hashed_size;

public:
  File (const char*);
  ~File ();

  long get_size () { return m_fsize; }
  int get_fd () { return m_fd; }

  void calc_file ();
  void hash();
};

#endif
