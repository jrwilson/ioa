#ifndef __decls_h__
#define __decls_h__

#include <automan.h>
#include <table.h>

typedef enum {
  NORMAL,
  OUTSTANDING
} status_t;

typedef struct sequence_item_struct {
  order_type_t order_type;
  automan_handler_t handler;
  void* hparam;
  union {
    aid_t* aid_ptr;
    bool* flag_ptr;
  };
  union {
    struct {
      const descriptor_t* descriptor;
      const void* ctor_arg;
    } create;
    struct {
      void* param;
    } declare;
    struct {
      aid_t* out_aid_ptr;
      output_t output;
      void* out_param;
      aid_t* in_aid_ptr;
      input_t input;
      void* in_param;
    } compose;
  };
} sequence_item_t;

typedef struct input_item_struct {
  automan_handler_t handler;
  void* hparam;
  bool* flag_ptr;
  input_t input;
  void* in_param;
} input_item_t;

typedef struct output_item_struct {
  automan_handler_t handler;
  void* hparam;
  bool* flag_ptr;
  output_t output;
  void* out_param;
} output_item_t;

typedef struct proxy_item_struct {
  aid_t* aid_ptr;
  aid_t source_aid;
  input_t source_free_input;
  bid_t bid;
  input_t callback;
  proxy_handler_t handler;
  void* pparam;
} proxy_item_t;

struct automan_struct {
  void* state;
  aid_t* self_ptr;

  status_t sequence_status;
  order_t last_order;
  sequence_item_t last_sequence;
  table_t* si_table;
  index_t* si_index;

  table_t* ii_table;
  index_t* ii_index;

  table_t* oi_table;
  index_t* oi_index;

  status_t proxy_status;
  table_t* pi_table;
  index_t* pi_index;
};

bool
si_decomposed_input_equal (const void* x0, const void* y0);


void
si_balance_compose (automan_t* automan,
		    bool* flag_ptr);

iterator_t
si_decompose_flag_ptr (automan_t* automan,
		       bool* flag_ptr,
		       receipt_type_t receipt);

bool
si_child_destroyed_aid_equal (const void* x0, const void* y0);

void
si_balance_create (automan_t* automan,
		   aid_t* aid_ptr);

void
si_destroy_aid_ptr (automan_t* automan,
		    aid_t* aid_ptr);

input_item_t*
ii_find_input (automan_t* automan,
	       input_t input,
	       void* in_param);

output_item_t*
oi_find_output (automan_t* automan,
		output_t output,
		void* out_param);

void
pi_process (automan_t* automan);

#endif
