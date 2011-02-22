#ifndef __mftp_h__
#define __mftp_h__

#include <stdint.h>
#include <openssl/md5.h>
#include <ueioa.h>

#define HASH_SIZE MD5_DIGEST_LENGTH
#define FRAGMENT_SIZE 512
#define FRAGMENT_SIZE_LOG2 9

#define ALIGNED(x) (((x) & (FRAGMENT_SIZE - 1)) == 0)

typedef struct {
  uint8_t hash[HASH_SIZE];
  uint32_t size;
  uint32_t type;
} mftp_FileID_t;

void mftp_FileID_init (mftp_FileID_t*, uint8_t hash[HASH_SIZE], uint32_t size, uint32_t type);
void mftp_FileID_hostToNet (mftp_FileID_t*, const mftp_FileID_t*);
void mftp_FileID_netToHost (mftp_FileID_t*, const mftp_FileID_t*);
int mftp_FileID_cmp (const mftp_FileID_t*, const mftp_FileID_t*);

typedef enum {
  ANNOUNCEMENT,
  MATCH,
  REQUEST,
  FRAGMENT
} mftp_MessageType_t;

typedef struct {
  uint32_t type;
} mftp_Header_t;

typedef struct {
  mftp_FileID_t fileid;
} mftp_Announcement_t;

typedef struct {
  mftp_FileID_t fileid1;
  mftp_FileID_t fileid2;
} mftp_Match_t;

typedef struct {
  mftp_FileID_t fileid;
  uint32_t offset;
  uint32_t size;
} mftp_Request_t;

typedef struct {
  mftp_FileID_t fileid;
  uint32_t offset;
  uint32_t size;
  uint8_t data[FRAGMENT_SIZE];
} mftp_Fragment_t;

typedef struct {
  mftp_Header_t header;
  union {
    mftp_Announcement_t announcement;
    mftp_Match_t match;
    mftp_Request_t request;
    mftp_Fragment_t fragment;
  };
} mftp_Message_t;

void mftp_Announcement_init (mftp_Message_t*, const mftp_FileID_t*);
void mftp_Match_init (mftp_Message_t*, const mftp_FileID_t*, const mftp_FileID_t*);
void mftp_Request_init (mftp_Message_t*, const mftp_FileID_t*, uint32_t offset, uint32_t size);
void mftp_Fragment_init (mftp_Message_t*, const mftp_FileID_t*, uint32_t offset, uint32_t size, uint8_t data[FRAGMENT_SIZE]);
void mftp_Message_hostToNet (mftp_Message_t*, const mftp_Message_t*);
int mftp_Message_netToHost (mftp_Message_t*, const mftp_Message_t*, int);

typedef struct {
  mftp_FileID_t fileid;
  uint8_t data[];
} mftp_File_t;

mftp_File_t* mftp_File_create_buffer (const void* buffer, uint32_t size, uint32_t type);
mftp_File_t* mftp_File_create_empty (const mftp_FileID_t*);
void mftp_File_destroy (mftp_File_t*);

extern descriptor_t msg_sender_descriptor;
void msg_sender_request_proxy (void*, void*, bid_t);
void msg_sender_proxy_message_in (void*, void*, bid_t);

extern descriptor_t msg_receiver_descriptor;
bid_t msg_receiver_announcement_out (void*, void*);
bid_t msg_receiver_match_out (void*, void*);
bid_t msg_receiver_request_out (void*, void*);
bid_t msg_receiver_fragment_out (void*, void*);

extern descriptor_t file_server_descriptor;
typedef struct {
  mftp_File_t* file;
  bool download;
} file_server_create_arg_t;
void file_server_announcement_in (void*, void*, bid_t);
void file_server_request_in (void*, void*, bid_t);
void file_server_fragment_in (void*, void*, bid_t);
bid_t file_server_message_out (void*, void*);
bid_t file_server_download_complete_out (void*, void*);

#endif /* __mftp_h__ */
