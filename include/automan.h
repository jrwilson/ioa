#ifndef __automan_h__
#define __automan_h__

#include <ueioa.h>

typedef struct automan_struct automan_t;
typedef void (*automan_handler_t) (void* state, void* param, receipt_type_t receipt);

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

/* void automan_output_add (automan_t* automan, */
/* 			 bool* flag_ptr, */
/* 			 output_t output, */
/* 			 void* out_param); */

/* void automan_dependency_add (automan_t* automan, */
/* 			     aid_t* child, */
/* 			     aid_t* dependent, */
/* 			     input_t free_input, */
/* 			     bid_t bid); */
/* void automan_dependency_remove (automan_t* automan, */
/* 				aid_t* child, */
/* 				aid_t* dependent); */

/* void automan_proxy_add (automan_t*, aid_t* proxy_aid_ptr, aid_t* source_aid_ptr, input_t source_free_input, input_t callback, bid_t); */
/* typedef struct proxy_request_struct { */
/*   bid_t bid; */
/*   aid_t callback_aid; */
/*   input_t callback_free_input; */
/* } proxy_request_t; */
/* typedef struct proxy_receipt_struct { */
/*   aid_t proxy_aid; */
/*   bid_t bid; */
/* } proxy_receipt_t; */
/* bid_t proxy_receipt_create (aid_t, bid_t); */
/* void automan_proxy_receive (automan_t*, const proxy_receipt_t*); */

#endif /* __automan_h__ */
