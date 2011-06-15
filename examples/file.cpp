#include "file.hpp"

#include <arpa/inet.h>
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



class sha2_256 {
private:
  static const unsigned int h_init[8];
  static const unsigned int k[64];
  unsigned int h[8];
  unsigned int total_length;
  unsigned int buf_length;
  unsigned char buf[64];

  static unsigned int right_rotate (unsigned int x, unsigned int s) {
    return (x >> s) | (x << (32 - s));
  }

  void process () {
    assert (buf_length == 64);

    // Break chunk into 16 4-byte variables.
    unsigned int w[64];
    for (unsigned int idx = 0; idx < 16; ++idx) {
      memcpy (&w[idx], &buf[idx * 4], 4);
      w[idx] = ntohl (w[idx]);
    }

    // Extend to 64 variables.
    for (unsigned int idx = 16;
	 idx < 64;
	 ++idx) {
      unsigned int s0 = right_rotate (w[idx - 15], 7) ^ right_rotate (w[idx - 15], 18) ^ (w[idx - 15] >> 3);
      unsigned int s1 = right_rotate (w[idx - 2], 17) ^ right_rotate (w[idx - 2], 19) ^ (w[idx - 2] >> 10);
      w[idx] = w[idx - 16] + s0 + w[idx - 7] + s1;
    }

    unsigned int a0 = h[0];
    unsigned int b0 = h[1];
    unsigned int c0 = h[2];
    unsigned int d0 = h[3];
    unsigned int e0 = h[4];
    unsigned int f0 = h[5];
    unsigned int g0 = h[6];
    unsigned int h0 = h[7];

    for (unsigned int idx = 0;
	 idx < 64; 
	 ++idx) {
      unsigned int s0 = right_rotate (a0, 2) ^ right_rotate (a0, 13) ^ right_rotate (a0, 22);
      unsigned int maj = (a0 & b0) ^ (a0 & c0) ^ (b0 & c0);
      unsigned int t2 = s0 + maj;
      unsigned int s1 = right_rotate (e0, 6) ^ right_rotate (e0, 11) ^ right_rotate (e0, 25);
      unsigned int ch = (e0 & f0) ^ ((~e0) & g0);
      unsigned int t1 = h0 + s1 + ch + k[idx] + w[idx];

      h0 = g0;
      g0 = f0;
      f0 = e0;
      e0 = d0 + t1;
      d0 = c0;
      c0 = b0;
      b0 = a0;
      a0 = t1 + t2;
    }

    h[0] += a0;
    h[1] += b0;
    h[2] += c0;
    h[3] += d0;
    h[4] += e0;
    h[5] += f0;
    h[6] += g0;
    h[7] += h0;

    buf_length = 0;
  }

public:
  static const unsigned int hash_size = 32;
  sha2_256 () :
    total_length (0),
    buf_length (0)
  {  
    memcpy (h, h_init, sizeof (h));
    memset (buf, 0, sizeof (buf));
  }

  void finalize () {
    if (buf_length == 64) {
      process ();
    }

    // THIS IS IN BITS!!!
    unsigned int length_before_finalize = total_length * 8;

    // Append 1.
    buf[buf_length] = (1 << 7);
    ++buf_length;

    if (buf_length == 64) {
      process ();
    }

    // Append 0 to pad until length.
    for (;
    	 buf_length < (64 - 8);
    	 ++buf_length) {
      buf[buf_length] = 0;
    }

    buf[buf_length++] = 0;
    buf[buf_length++] = 0;
    buf[buf_length++] = 0;
    buf[buf_length++] = 0;

    // Append the length before finalizing.
    unsigned int be_len = htonl (length_before_finalize);
    memcpy (buf + buf_length, &be_len, sizeof (be_len));
    buf_length += sizeof (be_len);

    assert (buf_length == 64);

    for (unsigned int idx = 0; idx < 64; ++idx) {
      printf ("%02x", buf[idx]);
    }
    printf ("\n");

    process ();
  }

  void get (unsigned char* hash) const {
    for (unsigned int idx = 0; idx < 8; ++idx) {
      unsigned int x = htonl (h[idx]);
      memcpy (hash + idx * 4, &x, 4);
    }
  }

  void update (const unsigned char* data, size_t length) {
    size_t read_idx = 0;
    while (read_idx < length) {
      size_t bytes_to_process = std::min (static_cast<size_t> (64 - buf_length), length - read_idx);
      memcpy (&buf[buf_length], data + read_idx, bytes_to_process);
      buf_length += bytes_to_process;

      if (buf_length == 64) {
	process ();
      }

      read_idx += bytes_to_process;
    }

    total_length += length;
  }

};


const unsigned int sha2_256::h_init[8] = {
  0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 
};


const unsigned int sha2_256::k[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

File::File (const char* name) :
  m_fname (name)
{
  int fd = open (m_fname.c_str (), O_RDONLY);
  assert (fd != -1);

  struct stat stats;

  if (fstat (fd, &stats) >= 0) {
    m_original_size = stats.st_size;
  }
  else {
    perror ("fstat");
    exit (EXIT_FAILURE);
  }

  // Calculate the size after we pad with 0s to fall on a MD5_HASH_LENGTH boundary.
  m_padded_size = m_original_size + MD5_HASH_LENGTH - (m_original_size % MD5_HASH_LENGTH);
  assert ((m_padded_size % MD5_HASH_LENGTH) == 0);

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

  m_fragment_count = static_cast<uint32_t> (ceil (static_cast<double> (m_padded_size) / static_cast<double> (FRAGMENT_SIZE - MD5_HASH_LENGTH)));
  m_hashed_size = m_padded_size + MD5_HASH_LENGTH * (m_fragment_count - 1);
  assert ((m_hashed_size % MD5_HASH_LENGTH) == 0);

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
  digester.update (m_data, m_original_size);
  digester.finalize ();


  unsigned char d[sha2_256::hash_size];
  digester.get (d);

  for (unsigned int idx = 0; idx < sha2_256::hash_size; ++idx) {
    printf ("%02x", d[idx]);
  }

  printf ("\n");


  // EVP_MD_CTX running_context;
  // assert (EVP_DigestInit (&running_context, EVP_md5 ()) == 1);

  // uint32_t read_idx;
  // uint32_t write_idx;

  // for (read_idx = 0, write_idx = m_padded_size; read_idx < m_hashed_size; read_idx += FRAGMENT_SIZE, write_idx += MD5_HASH_LENGTH) {
  //   // How many bytes should we read?
  //   uint32_t read_size = std::min (FRAGMENT_SIZE, m_hashed_size - read_idx);
  //   // Update the digest.
  //   assert (EVP_DigestUpdate (&running_context, m_data + read_idx, read_size) == 1);

  //   unsigned int len = MD5_HASH_LENGTH;
  //   unsigned char samp[MD5_HASH_LENGTH];

  //   EVP_MD_CTX sample_context;
  //   assert (EVP_MD_CTX_copy (&sample_context, &running_context) == 1);
  //   assert (EVP_DigestFinal (&sample_context, samp, &len) == 1);

  //   for (unsigned int idx = 0; idx < MD5_HASH_LENGTH; ++idx) {
  //     printf ("%02x", samp[idx]);
  //   }
  //   printf ("\n");

  //   if (write_idx < m_hashed_size) {
  //     std::cout << "[" << read_idx << "," << read_idx + read_size << ") => [" << write_idx << "," << write_idx + MD5_HASH_LENGTH << ")" << std::endl;
  //     memcpy (m_data + write_idx, samp, MD5_HASH_LENGTH);
  //   }
  //   else {
  //     std::cout << "[" << read_idx << "," << read_idx + read_size << ") => fileid" << std::endl;
  //   }
  // }

  // assert (EVP_MD_CTX_cleanup (&running_context) == 1);
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

// void File::hash() {
//   unsigned char md_result[MD5_HASH_LENGTH];
//   unsigned int md_len;
//   int start;

//   for(int i = 0; i < m_frag_index; i++) {
//     std::cout << "in first for: " << i << std::endl;
//     std::cout << "DigestUpdate to hash " << FRAGMENT_SIZE << " at " <<  static_cast<void*> (m_data + FRAGMENT_SIZE * i) << " of end data." << std::endl << std::endl;
//     EVP_DigestUpdate(&mdctx, static_cast<void*> (m_data + FRAGMENT_SIZE * i), static_cast<size_t> (FRAGMENT_SIZE));

//     start = m_padded_size + MD5_HASH_LENGTH * i;
//     for(int j = start; j < start + MD5_HASH_LENGTH; j++) {
//       std::cout << "in second for: " << j << std::endl;
//       m_data[j] = md_result[j - start];
//       std::cerr << "Data in position " << j << " of m_data is being taken from position " << j - start << " of the hash array\n";
//     }

//     std::cout << std::endl << "Hash for fragment " << i << " is ";
//     for(int j = 0; j < MD5_HASH_LENGTH; j++) {
//       printf("%02x",md_result[i]);
//     }
//     std::cout << std::endl << std::endl;
//   }

//   std::cout << "4" << std::endl;


//   EVP_DigestUpdate(&mdctx, static_cast<void*> (m_data), static_cast<size_t> (m_original_size));
//   std::cout << "5" << std::endl;
//   //EVP_DigestFinal_ex(&mdctx, md_result, &md_len);

//   start = m_padded_size + MD5_HASH_LENGTH * m_frag_index;
//   for(int i = start; i < m_hashed_size; i++) {
//     m_data[i] = md_result[i - start];
//     std::cerr << "Data in position " << i << " of m_data is being taken from position " << i - start << " of the hash array\n";
//   }

//   EVP_MD_CTX_cleanup(&mdctx);
// }

