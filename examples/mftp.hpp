#ifndef __mftp_hpp__
#define __mftp_hpp__

#include <stdint.h>
#include <cstring>
#include <cassert>
#include <math.h>
#include <utility>

#define HASH_SIZE 32U
#define FRAGMENT_SIZE 512U

#define REQUEST 0u
#define FRAGMENT 1u

struct fileID {
  uint8_t hash[HASH_SIZE];
  uint32_t type;
  uint32_t length;

  bool operator== (const fileID& other) const {
    return memcmp (hash, other.hash, HASH_SIZE) == 0 &&
      type == other.type &&
      length == other.length;
  }

private:

  std::pair<uint32_t, uint32_t> calculate_lengths () const {
    // LET'S DO MATH!!

    // Calculate the size after we pad with 0s to fall on a HASH_SIZE boundary.
    uint32_t padded_length = length + HASH_SIZE - (length % HASH_SIZE);
    assert ((padded_length % HASH_SIZE) == 0);

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

    uint32_t fragment_count = static_cast<uint32_t> (ceil (static_cast<double> (padded_length) / static_cast<double> (FRAGMENT_SIZE - HASH_SIZE)));
    uint32_t hashed_length = padded_length + HASH_SIZE * (fragment_count - 1);
    assert ((hashed_length % HASH_SIZE) == 0);

    uint32_t final_length = hashed_length + FRAGMENT_SIZE - (hashed_length % FRAGMENT_SIZE);
    assert ((final_length % FRAGMENT_SIZE) == 0);

    uint32_t padding = (final_length - hashed_length) + (padded_length - length);
    assert (padding < FRAGMENT_SIZE);

    return std::make_pair (padding, final_length);
  }

public:

  uint32_t padding () const {
    return calculate_lengths ().first;
  }

  uint32_t final_length () const {
    return calculate_lengths ().second;
  }

  uint32_t fragment_count () const {
    return final_length () / FRAGMENT_SIZE;
  }

};

struct fragment {
  fileID fid;
  uint32_t offset;
  uint8_t data[FRAGMENT_SIZE];
};

struct request {
  fileID fid;
  uint32_t start;
  uint32_t stop;
};

struct message_header {
  uint32_t message_type;
};

struct fragment_type { };
struct request_type { };

struct message {
  message_header header;
  union {
    fragment frag;
    request req;
  };

  message (fragment_type /* */,
	   const fileID& fileid,
	   uint32_t offset,
	   const uint8_t* data)
  {
    header.message_type = FRAGMENT;
    frag.fid = fileid;
    frag.offset = offset;
    memcpy (frag.data, data, FRAGMENT_SIZE);
  }

  message (request_type /* */,
	   const fileID& fileid,
	   uint32_t start,
	   uint32_t stop)
  {
    header.message_type = REQUEST;
    req.fid = fileid;
    req.start = start;
    req.stop = stop;
  }

};

#endif
