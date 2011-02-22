#ifndef __consumer_h__
#define __consumer_h__

#include <ueioa.h>

extern descriptor_t consumer_descriptor;

void consumer_announcement_in (void*, void*, bid_t);
void consumer_match_in (void*, void*, bid_t);
void consumer_request_in (void*, void*, bid_t);
void consumer_fragment_in (void*, void*, bid_t);

#endif /* __consumer_h__ */
