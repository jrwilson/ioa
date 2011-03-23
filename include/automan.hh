#ifndef __automan_hh__
#define __automan_hh__

#include <ueioa.hh>
#include <list>
#include <cassert>

class automan {
  
 public:
  
  typedef enum {
    PROXY_REQUEST_INPUT_DNE,
    PROXY_CREATED,
    PROXY_NOT_CREATED,
    PROXY_DESTROYED,
  } proxy_receipt_type_t;
  
  typedef void (*automan_handler_t) (void* state, void* param, receipt_type_t receipt);
  typedef void (*proxy_handler_t) (void* state, void* param, proxy_receipt_type_t receipt, bid_t bid);
  
  automan (void* state,
	   aid_t* self_ptr);
    
  void
  apply (const receipt_t& receipt);
  
  bid_t
  action () __attribute__ ((warn_unused_result));
  
  int
  create (aid_t* aid_ptr,
	  const descriptor_t* descriptor,
	  const void* ctor_arg,
	  automan_handler_t handler,
	  void* hparam) __attribute__ ((warn_unused_result));
  
  int
  declare (bool* flag_ptr,
	   void* param,
	   automan_handler_t handler,
	   void* hparam) __attribute__ ((warn_unused_result));
  
  int
  compose (bool* flag_ptr,
	   aid_t* out_aid_ptr,
	   output_t output,
	   void* out_param,
	   aid_t* in_aid_ptr,
	   input_t input,
	   void* in_param,
	   automan_handler_t handler,
	   void* hparam) __attribute__ ((warn_unused_result));
  
  int
  decompose (bool* flag_ptr) __attribute__ ((warn_unused_result));
  
  int
  rescind (bool* flag_ptr) __attribute__ ((warn_unused_result));
  
  int
  destroy (aid_t* aid_ptr) __attribute__ ((warn_unused_result));
  
  void
  self_destruct ();
  
  int
  input_add (bool* flag_ptr,
	     input_t input,
	     void* in_param,
	     automan_handler_t handler,
	     void* hparam) __attribute__ ((warn_unused_result));
  
  int
  output_add (bool* flag_ptr,
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
  proxy_add (aid_t* aid_ptr,
	     aid_t source_aid,
	     input_t source_free_input,
	     bid_t bid,
	     input_t callback,
	     proxy_handler_t handler,
	     void* pparam) __attribute__ ((warn_unused_result));
  
  static int
  proxy_send_created (aid_t proxy_aid,
		      bid_t bid,
		      const proxy_request_t& proxy_request)  __attribute__ ((warn_unused_result));
  
  static int
  proxy_send_not_created (bid_t bid,
			  const proxy_request_t& proxy_request)  __attribute__ ((warn_unused_result));
  
  static int
  proxy_send_destroyed (aid_t proxy_aid,
			const proxy_request_t& proxy_request)  __attribute__ ((warn_unused_result));
  
  void
  proxy_receive (bid_t bid);

private:

  typedef enum {
    NORMAL,
    OUTSTANDING
  } status_t;
  
  typedef struct sequence_item_struct {
    order_type_t order_type;
    automan::automan_handler_t handler;
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

  class si_aid_ptr_equal {
  private:
    aid_t* m_aid_ptr;
  public:
    si_aid_ptr_equal (aid_t* aid_ptr) :
      m_aid_ptr (aid_ptr) { }
    bool operator() (const sequence_item_t& x) const {
      return m_aid_ptr == x.aid_ptr;
    }
  };

  class si_flag_ptr_equal {
  private:
    bool* m_flag_ptr;
  public:
    si_flag_ptr_equal (bool* flag_ptr) :
      m_flag_ptr (flag_ptr) { }
    bool operator() (const sequence_item_t& x) const {
      return m_flag_ptr == x.flag_ptr;
    }
  };

  class si_param_equal {
  private:
    void* m_param;
  public:
    si_param_equal (void* param) :
      m_param (param) { }
    bool operator() (const sequence_item_t& x) const {
      switch (x.order_type) {
      case CREATE:
      case COMPOSE:
      case DECOMPOSE:
      case DESTROY:
	return false;
      case DECLARE:
      case RESCIND:
	return m_param == x.declare.param;
      }
      /* Not reached. */
      assert (0);
      return false;
    }
  };

  class si_composition_aid_ptr_equal {
  private:
    aid_t* m_aid_ptr;
  public:
    si_composition_aid_ptr_equal (aid_t* aid_ptr) :
      m_aid_ptr (aid_ptr) { }
    bool operator() (const sequence_item_t& x) const {
      switch (x.order_type) {
      case CREATE:
      case DECLARE:
      case DECOMPOSE:
      case RESCIND:
      case DESTROY:
	return false;
      case COMPOSE:
	return x.compose.out_aid_ptr == m_aid_ptr || x.compose.in_aid_ptr == m_aid_ptr;
      }
      /* Not reached. */
      assert (0);
      return false;
    }
  };


  class si_decomposed_input_equal {
  private:
    const receipt_t& m_receipt;
  public:
    si_decomposed_input_equal (const receipt_t& receipt) :
      m_receipt (receipt) { }
    bool operator() (const sequence_item_t& x) const {
      switch (x.order_type) {
      case CREATE:
      case DESTROY:
      case DECLARE:
      case RESCIND:
	return false;
      case COMPOSE:
      case DECOMPOSE:
	return
	  *x.compose.in_aid_ptr == m_receipt.decomposed.in_aid &&
	  x.compose.input == m_receipt.decomposed.input &&
	  x.compose.in_param == m_receipt.decomposed.in_param;
      }
      /* Not reached. */
      assert (0);
      return false;
    }
  };

  class si_child_destroyed_aid_equal {
  private:
    const receipt_t& m_receipt;
  public:
    si_child_destroyed_aid_equal (const receipt_t& receipt) :
      m_receipt (receipt) { }
    bool operator() (const sequence_item_t& x) const {
      switch (x.order_type) {
      case COMPOSE:
      case DECOMPOSE:
      case DECLARE:
      case RESCIND:
	return false;
      case CREATE:
      case DESTROY:
	return *x.aid_ptr == m_receipt.child_destroyed.child;
      }
      /* Not reached. */
      assert (0);
      return false;
    }
  };
  
  struct input_item_t {
    automan::automan_handler_t handler;
    void* hparam;
    bool* flag_ptr;
    input_t input;
    void* in_param;
    input_item_t (automan::automan_handler_t _handler, void* _hparam, bool* _flag_ptr, input_t _input, void* _in_param) :
      handler (_handler),
      hparam (_hparam),
      flag_ptr (_flag_ptr),
      input (_input),
      in_param (_in_param) { }
  };

  class ii_flag_ptr_equal {
  private:
    bool* m_flag_ptr;
  public:
    ii_flag_ptr_equal (bool* flag_ptr) :
      m_flag_ptr (flag_ptr) { }
    bool operator() (const input_item_t& x) const {
      return m_flag_ptr == x.flag_ptr;
    }
  };

  class ii_input_equal {
  private:
    input_t m_input;
    void* m_in_param;
  public:
    ii_input_equal (input_t input, void* in_param) :
      m_input (input),
      m_in_param (in_param) { }
    bool operator() (const input_item_t& x) const {
      return
	m_input == x.input &&
	m_in_param == x.in_param;
    }
  };

  struct output_item_t {
    automan::automan_handler_t handler;
    void* hparam;
    bool* flag_ptr;
    output_t output;
    void* out_param;
    output_item_t (automan::automan_handler_t _handler, void* _hparam, bool* _flag_ptr, output_t _output, void* _out_param) :
      handler (_handler),
      hparam (_hparam),
      flag_ptr (_flag_ptr),
      output (_output),
      out_param (_out_param) { }
  };

  class oi_flag_ptr_equal {
  private:
    bool* m_flag_ptr;
  public:
    oi_flag_ptr_equal (bool* flag_ptr) :
      m_flag_ptr (flag_ptr) { }
    bool operator() (const output_item_t& x) const {
      return m_flag_ptr == x.flag_ptr;
    }
  };

  class oi_output_equal {
  private:
    output_t m_output;
    void* m_out_param;
  public:
    oi_output_equal (output_t output, void* out_param) :
      m_output (output),
      m_out_param (out_param) { }
    bool operator() (const output_item_t& x) const {
      return
	m_output == x.output &&
	m_out_param == x.out_param;
    }
  };

  struct proxy_item_t {
    aid_t* aid_ptr;
    aid_t source_aid;
    input_t source_free_input;
    bid_t bid;
    input_t callback;
    automan::proxy_handler_t handler;
    void* pparam;
    proxy_item_t () { }
    proxy_item_t (aid_t* _aid_ptr, aid_t _source_aid, input_t _source_free_input, bid_t _bid, input_t _callback, automan::proxy_handler_t _handler, void* _pparam) :
      aid_ptr (_aid_ptr),
      source_aid (_source_aid),
      source_free_input (_source_free_input),
      bid (_bid),
      callback (_callback),
      handler (_handler),
      pparam (_pparam) { }
  };

  class pi_aid_ptr_equal {
  private:
    aid_t* m_aid_ptr;
  public:
    pi_aid_ptr_equal (aid_t* aid_ptr) :
      m_aid_ptr (aid_ptr) { }
    bool operator() (const proxy_item_t& x) const {
      return m_aid_ptr == x.aid_ptr;
    }
  };

  class pi_receipt_aid_equal {
  private:
    const proxy_receipt_t& m_receipt;
  public:
    pi_receipt_aid_equal (const proxy_receipt_t& receipt) :
      m_receipt (receipt) { }
    bool operator() (const proxy_item_t& x) const {
      return m_receipt.proxy_aid == *x.aid_ptr;
    }
  };

  void* m_state;
  aid_t* m_self_ptr;
  
  status_t m_sequence_status;
  order_t m_last_order;
  sequence_item_t m_last_sequence;
  typedef std::list<sequence_item_t> si_list;
  si_list m_si_index;
  typedef std::list<input_item_t> ii_list;
  ii_list m_ii_index;
  typedef std::list<output_item_t> oi_list;
  oi_list m_oi_index;
  
  status_t m_proxy_status;
  proxy_item_t m_last_proxy;
  typedef std::list<proxy_item_t> pi_list;
  pi_list m_pi_index;

  void
  si_balance_compose (bool* flag_ptr);

  si_list::iterator
  si_decompose_flag_ptr (bool* flag_ptr,
  			 receipt_type_t receipt);
  
  si_list::iterator
  si_rescind_flag_ptr (bool* flag_ptr,
  		       receipt_type_t receipt);

  void
  si_balance_create (aid_t* aid_ptr);

  void
  si_destroy_aid_ptr (aid_t* aid_ptr);

  void
  pi_process ();

  bool
  pi_closed_aid_ptr (aid_t* aid_ptr) const;

  static bid_t
  proxy_request_create (bid_t bid,
			aid_t callback_aid,
			input_t callback_free_input);

  bool
  si_process ();
  
  bool
  si_param_declared (void* param);

  bool
  si_open_aid_ptr_create (aid_t* aid_ptr);
  
  void
  si_append_decompose (bool* flag_ptr,
		       aid_t* out_aid_ptr,
		       output_t output,
		       void* out_param,
		       aid_t* in_aid_ptr,
		       input_t input,
		       void* in_param,
		       automan_handler_t handler,
		       void* hparam);

  void
  si_append_destroy (aid_t* aid_ptr,
		     automan_handler_t handler,
		     void* hparam);

  bool
  si_open_flag_ptr_compose (bool* flag_ptr);

  bool
  si_closed_aid_ptr (aid_t* aid_ptr);

  void
  si_append_create (aid_t* aid_ptr,
		    const descriptor_t* descriptor,
		    const void* ctor_arg,
		    automan_handler_t handler,
		    void* hparam);

    
};

#endif /* __automan_hh__ */
