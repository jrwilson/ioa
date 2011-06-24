#ifndef __sha2_256_hpp__
#define __sha2_256_hpp__

/*
  Implementation of the SHA2-256 digest algorithm.
  Based on pseuedo-code found at http://en.wikipedia.org/wiki/SHA-2
 */

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>

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

  sha2_256 (const unsigned int length,
	    const unsigned char* hash) :
    total_length (length),
    buf_length (0)
  {
    if (length != 0) {
      for (unsigned int idx = 0; idx < 8; ++idx) {
	memcpy (h + idx, hash + idx * 4, 4);
	h[idx] = htonl (h[idx]);
      }
    }
    else {
      memcpy (h, h_init, sizeof (h));
    }
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
    ++total_length;

    if (buf_length == 64) {
      process ();
    }

    // Append 0 to pad until length.
    for (;
    	 buf_length < (64 - 8);
    	 ++buf_length, ++total_length) {
      buf[buf_length] = 0;
    }

    buf[buf_length++] = 0;
    buf[buf_length++] = 0;
    buf[buf_length++] = 0;
    buf[buf_length++] = 0;
    total_length += 4;

    // Append the length before finalizing.
    unsigned int be_len = htonl (length_before_finalize);
    memcpy (buf + buf_length, &be_len, sizeof (be_len));
    buf_length += sizeof (be_len);
    total_length += sizeof (be_len);

    assert (buf_length == 64);

    process ();
  }

  void get (unsigned char* hash) const {
    assert (buf_length == 0);
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

#endif
