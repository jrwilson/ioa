#ifndef __buffers_h__
#define __buffers_h__

#include <ueioa.h>

typedef struct buffers_struct buffers_t;

buffers_t* buffers_create (void);
void buffers_destroy (buffers_t*);

bid_t buffers_alloc (buffers_t*, aid_t, size_t);

void* buffers_write_ptr (buffers_t*, aid_t, bid_t);
const void* buffers_read_ptr (buffers_t*, aid_t, bid_t);
size_t buffers_size (buffers_t*, aid_t, bid_t);
/* size_t buffers_ref_count (buffers_t*, aid_t, bid_t); /\* For testing. *\/ */
bool buffers_exists (buffers_t*, aid_t, bid_t);

void buffers_change_owner (buffers_t*, aid_t, bid_t);
void buffers_incref (buffers_t*, aid_t, bid_t);
void buffers_decref (buffers_t*, aid_t, bid_t);

void buffers_add_child (buffers_t*, aid_t, bid_t, bid_t);
void buffers_remove_child (buffers_t*, aid_t, bid_t, bid_t);

void buffers_purge_aid (buffers_t*, aid_t);

bid_t buffers_dup (buffers_t*, aid_t, bid_t, size_t);

#endif /* __buffers_h__ */
