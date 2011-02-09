#ifndef __buffers_h__
#define __buffers_h__

#include <ueioa.h>

typedef struct buffers_struct buffers_t;

buffers_t* buffers_create (void);
void buffers_destroy (buffers_t*);

bid_t buffer_alloc (buffers_t*, aid_t, size_t);

void* buffer_write_ptr (buffers_t*, aid_t, bid_t);
const void* buffer_read_ptr (buffers_t*, aid_t, bid_t);
size_t buffer_size (buffers_t*, aid_t, bid_t);
size_t buffer_ref_count (buffers_t*, aid_t, bid_t); /* For testing. */
bool buffer_exists (buffers_t*, bid_t); /* For testing. */

void buffer_change_owner (buffers_t*, aid_t, bid_t);
void buffer_incref (buffers_t*, aid_t, bid_t);
void buffer_decref (buffers_t*, aid_t, bid_t);

void buffer_add_child (buffers_t*, aid_t, bid_t, bid_t);
void buffer_remove_child (buffers_t*, aid_t, bid_t, bid_t);

void buffer_purge_aid (buffers_t*, aid_t);

bid_t buffer_dup (buffers_t*, aid_t, bid_t);

#endif /* __buffers_h__ */
