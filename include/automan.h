#ifndef __automan_h__
#define __automan_h__

#include <ueioa.h>

typedef struct automan_struct automan_t;

automan_t* automan_create (aid_t* self_ptr);

void automan_apply (automan_t* automan,
		    const receipt_t* receipt);

bid_t automan_action (automan_t* automan);

int automan_child_create (automan_t* automan,
			  aid_t* child_aid_ptr,
			  const descriptor_t* descriptor,
			  const void* ctor_arg,
			  internal_t internal,
			  void* param) __attribute__ ((warn_unused_result));

int automan_param_declare (automan_t* automan,
			   void* param,
			   internal_t internal);

/* void automan_child_remove (automan_t* automan, */
/* 			   aid_t* child_aid_ptr); */

/* void automan_input_add (automan_t* automan, */
/* 			bool* flag_ptr, */
/* 			input_t input, */
/* 			void* in_param, */
/* 			internal_t internal); */

/* void automan_output_add (automan_t* automan, */
/* 			 bool* flag_ptr, */
/* 			 output_t output, */
/* 			 void* out_param); */


/* void automan_param_remove (automan_t* automan, */
/* 			   void* param); */

/* void automan_composition_add (automan_t*, aid_t*, output_t, void*, aid_t*, input_t, void*); */

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
