#include "file.hpp"

#include <string.h>
#include <math.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <cassert>
#include <unistd.h>
#include <iostream>
//#include <openssl/md5.h> // Developed by Eric Young.  Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
#include <openssl/evp.h>  // Developed by Eric Young.  Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)

//think I need -lcrypto -lssl somewhere in a makefile, perhaps also with a necessary .a file.
File::File (const char* name) :
  m_fname (name)
{
  assert(m_fname.size () > 0);
  m_fd = open(m_fname.c_str (), O_RDONLY);
  assert(m_fd >= 0);

  struct stat m_stat;

  if (fstat (m_fd, &m_stat) >= 0) {
    m_fsize = m_stat.st_size;
  }
  else {
    exit(-1);
  }

  calc_file ();
  m_data = new char[m_hashed_size];
  int r = static_cast<int>(read(m_fd, m_data, static_cast<size_t>(m_original_size)));
  assert(r > 0);
  std::cout << std::string (m_data, m_original_size) << std::endl;
  close(m_fd);

  for(int i = m_original_size; i < m_padded_size; i++) {
    m_data[i] = 0;
  }
  hash();
  std::cout << "1" << std::endl;
  //std::cout << std::string (m_data, m_hashed_size) << std::endl;
}

File::~File () {
  delete [] m_data;
}
/*
EVP_DigestUpdate() hashes cnt bytes of data at d into the digest
context ctx. This function can be called several times on the same ctx
to hash additional data.
  int EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *d, size_t cnt);

EVP_DigestFinal_ex() retrieves the digest value from ctx and places it
in md. If the s parameter is not NULL then the number of bytes of data
written (i.e. the length of the digest) will be written to the integer
at s, at most EVP_MAX_MD_SIZE bytes will be written.  After calling
EVP_DigestFinal_ex() no additional calls to EVP_DigestUpdate() can be
made, but EVP_DigestInit_ex() can be called to initialize a new digest
operation.
  int EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);
*/

void File::hash() {
  std::cout << "Frag index: " << m_frag_index << std::endl;
  std::cout << "Fragment size: " << FRAGMENT_SIZE << std::endl;
  std::cout << "Original size: " << m_original_size << std::endl;
  std::cout << "Padded size: " << m_padded_size << std::endl;
  std::cout << "Hashed size: " << m_hashed_size << std::endl;
  std::cout << "2" << std::endl;
  //creates an evp digest context for md5
  EVP_MD_CTX mdctx;
  //EVP_DigestInit_ex (&mdctx, EVP_md5(), NULL);
  EVP_DigestInit (&mdctx, EVP_md5());
  unsigned char md_result[MD5_HASH_LENGTH];
  unsigned int md_len;
  int start;
  //EVP_DigestInit_ex (&mdctx, EVP_md5(), NULL);
  std::cout << "3" << std::endl;

  for(int i = 0; i < m_frag_index; i++) {
    std::cout << "in first for: " << i << std::endl;
    std::cout << "DigestUpdate to hash " << FRAGMENT_SIZE << " at " <<  static_cast<int> ((m_data*)) + FRAGMENT_SIZE * i << " of end data." << std::endl << std::endl;
    EVP_DigestUpdate(&mdctx, static_cast<void*> (m_data + FRAGMENT_SIZE * i), static_cast<size_t> (FRAGMENT_SIZE));

    start = m_padded_size + MD5_HASH_LENGTH * i;
    for(int j = start; j < start + MD5_HASH_LENGTH; j++) {
      std::cout << "in second for: " << j << std::endl;
      m_data[j] = md_result[j - start];
      std::cerr << "Data in position " << j << " of m_data is being taken from position " << j - start << " of the hash array\n";
    }

    std::cout << std::endl << "Hash for fragment " << i << " is ";
    for(int j = 0; j < MD5_HASH_LENGTH; j++) {
      printf("%02x",md_result[i]);
    }
    std::cout << std::endl << std::endl;
  }

  std::cout << "4" << std::endl;


  EVP_DigestUpdate(&mdctx, static_cast<void*> (m_data), static_cast<size_t> (m_original_size));
  std::cout << "5" << std::endl;
  //EVP_DigestFinal_ex(&mdctx, md_result, &md_len);

  start = m_padded_size + MD5_HASH_LENGTH * m_frag_index;
  for(int i = start; i < m_hashed_size; i++) {
    m_data[i] = md_result[i - start];
    std::cerr << "Data in position " << i << " of m_data is being taken from position " << i - start << " of the hash array\n";
  }

  EVP_MD_CTX_cleanup(&mdctx);
}

void File::calc_file() {
  long temp = 0;

  m_original_size = get_size();
  std::cout << "The original length of the file is " << m_original_size << " bytes."<< std::endl;
  m_padded_size = m_original_size + MD5_HASH_LENGTH - (m_original_size % MD5_HASH_LENGTH);
  std::cout << "The padded length of the file is " << m_padded_size << " bytes."<< std::endl;

  /*
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

  m_frag_index = static_cast<int> (ceil (static_cast<double> (m_padded_size) / static_cast<double> (FRAGMENT_SIZE - MD5_HASH_LENGTH)));
  m_hashed_size = m_padded_size + MD5_HASH_LENGTH * (m_frag_index - 1);

  std::cout << "The final length of the file is " << m_hashed_size << std::endl;
}

