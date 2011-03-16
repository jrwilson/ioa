#ifndef __automan_h__
#define __automan_h__

#include <ueioa.h>

typedef enum {
  PROXY_REQUEST_INPUT_DNE,
  PROXY_CREATED,
  PROXY_NOT_CREATED,
  PROXY_DESTROYED,
} proxy_receipt_type_t;

typedef struct automan_struct automan_t;
typedef void (*automan_handler_t) (void* state, void* param, receipt_type_t receipt);
typedef void (*proxy_handler_t) (void* state, void* param, proxy_receipt_type_t receipt, bid_t bid);

automan_t*
automan_creat (void* state,
	       aid_t* self_ptr);

void
automan_apply (automan_t* automan,
	       const receipt_t* receipt);

bid_t
automan_action (automan_t* automan) __attribute__ ((warn_unused_result));

int
automan_create (automan_t* automan,
		aid_t* aid_ptr,
		const descriptor_t* descriptor,
		const void* ctor_arg,
		automan_handler_t handler,
		void* hparam) __attribute__ ((warn_unused_result));

int
automan_declare (automan_t* automan,
		 bool* flag_ptr,
		 void* param,
		 automan_handler_t handler,
		 void* hparam) __attribute__ ((warn_unused_result));

int
automan_compose (automan_t* automan,
		 bool* flag_ptr,
		 aid_t* out_aid_ptr,
		 output_t output,
		 void* out_param,
		 aid_t* in_aid_ptr,
		 input_t input,
		 void* in_param,
		 automan_handler_t handler,
		 void* hparam) __attribute__ ((warn_unused_result));

int
automan_decompose (automan_t* automan,
		   bool* flag_ptr) __attribute__ ((warn_unused_result));

int
automan_rescind (automan_t* automan,
		 bool* flag_ptr) __attribute__ ((warn_unused_result));

int
automan_destroy (automan_t* automan,
		 aid_t* aid_ptr) __attribute__ ((warn_unused_result));

void
automan_self_destruct (automan_t* automan);

int
automan_input_add (automan_t* automan,
		   bool* flag_ptr,
		   input_t input,
		   void* in_param,
		   automan_handler_t handler,
		   void* hparam) __attribute__ ((warn_unused_result));

int
automan_output_add (automan_t* automan,
		    bool* flag_ptr,
		    output_t output,
		    void* out_param,
		    automan_handler_t handler,
		    void* hparam) __attribute__ ((warn_unused_result));

typedef struct proxy_request_struct {
  bid_t bid;
  aid_t callback_aid;
  input_t callback_free_input;
} proxy_request_t;

typedef struct proxy_receipt_struct {
  proxy_receipt_type_t type;
  aid_t proxy_aid;
  bid_t bid;
} proxy_receipt_t;

int
automan_proxy_add (automan_t* automan,
		   aid_t* aid_ptr,
		   aid_t source_aid,
		   input_t source_free_input,
		   bid_t bid,
		   input_t callback,
		   proxy_handler_t handler,
		   void* pparam) __attribute__ ((warn_unused_result));

int
automan_proxy_send_created (aid_t proxy_aid,
			    bid_t bid,
			    const proxy_request_t* proxy_request)  __attribute__ ((warn_unused_result));

int
automan_proxy_send_not_created (bid_t bid,
				const proxy_request_t* proxy_request)  __attribute__ ((warn_unused_result));

int
automan_proxy_send_destroyed (aid_t proxy_aid,
			      const proxy_request_t* proxy_request)  __attribute__ ((warn_unused_result));

void
automan_proxy_receive (automan_t* automan,
		       bid_t bid);

#endif /* __automan_h__ */
