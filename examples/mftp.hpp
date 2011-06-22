#ifndef __mftp_hpp__
#define __mftp_hpp__

#include <stdint.h>
#include <string.h>

#define HASH_LENGTH 32U
#define FRAGMENT_SIZE 512U

#define REQUEST 0u
#define FRAGMENT 1u

struct fileID {
  uint8_t hash[HASH_LENGTH];
  uint32_t type;
  uint32_t original_length;
  uint32_t hashed_length;

  bool operator== (const fileID& other) const {
    return memcmp (hash, other.hash, HASH_LENGTH) == 0 &&
      type == other.type &&
      original_length == other.original_length &&
      hashed_length == other.hashed_length;
  }
};

struct fragment {
  fileID fid;
  uint32_t offset;
  uint32_t length;
  uint8_t data[FRAGMENT_SIZE];
};

struct request {
  fileID fid;
  uint32_t offset;
  uint32_t length;
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
	   uint32_t length,
	   const uint8_t* data)
  {
    header.message_type = FRAGMENT;
    frag.fid = fileid;
    frag.offset = offset;
    frag.length = length;
    memcpy (frag.data, data, length);
  }

  message (request_type /* */,
	   const fileID& fileid,
	   uint32_t offset,
	   uint32_t length)
  {
    header.message_type = REQUEST;
    req.fid = fileid;
    req.offset = offset;
    req.length = length;
  }

};

#endif
