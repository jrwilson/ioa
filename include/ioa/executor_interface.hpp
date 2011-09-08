#ifndef __executor_interface_hpp__
#define __executor_interface_hpp__

#include <ioa/aid.hpp>
#include <cstdlib>
#include <memory>

namespace ioa {

  class model_interface;
  class system_scheduler_interface;

  class action_executor_interface
  {
  public:
    virtual ~action_executor_interface () { }
    virtual bool fetch_instance (model_interface&) = 0;
    virtual aid_t get_aid () const = 0;
    virtual void* get_member_ptr () const = 0;
    virtual void* get_pid () const = 0;

    virtual bool operator== (const action_executor_interface& x) const {
      return
  	get_aid () == x.get_aid () &&
  	get_member_ptr () == x.get_member_ptr () &&
  	get_pid () == x.get_pid ();
    }

    bool operator!= (const action_executor_interface& x) const {
      return !(*this == x);
    }

    bool operator< (const action_executor_interface& x) const {
      if (get_aid () != x.get_aid ()) {
	return get_aid () < x.get_aid ();
      }
      if (get_member_ptr () != x.get_member_ptr ()) {
	return get_member_ptr () < x.get_member_ptr ();
      }
      return get_pid () < x.get_pid ();
    }

  };

  class input_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~input_executor_interface () { }
    virtual std::auto_ptr<input_executor_interface> clone () const = 0;
    virtual void set_parameter (const aid_t) = 0;
    virtual void bound (model_interface&, system_scheduler_interface&) const = 0;
    virtual void unbound (model_interface&, system_scheduler_interface&) const = 0;
  };

  class unvalued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~unvalued_input_executor_interface () { }
    virtual void operator() (system_scheduler_interface&) const = 0;
  };

  template <typename T>
  class valued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~valued_input_executor_interface () { }
    virtual void operator() (system_scheduler_interface&, const T& t) const = 0;
  };

  class local_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~local_executor_interface () { }
    virtual void operator() (model_interface&, system_scheduler_interface&) const = 0;
  };
  
  class output_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~output_executor_interface () { }
    virtual std::auto_ptr<output_executor_interface> clone () const = 0;
    virtual bool involves_output (const action_executor_interface&) const = 0;
    virtual bool involves_input (const action_executor_interface&) const = 0;
    virtual bool involves_input_automaton (const aid_t) const = 0;
    virtual bool involves_binding (const action_executor_interface&,
				   const action_executor_interface&,
				   const aid_t) const = 0;
    virtual bool involves_aid_key (const aid_t,
				   void* const) const = 0;
    virtual bool empty () const = 0;
    virtual size_t size () const = 0;
    virtual void bind (system_scheduler_interface&,
		       model_interface&,
		       const input_executor_interface&,
		       const aid_t,
		       void* const) = 0;
    virtual void unbind (const aid_t,
			 void* const) = 0;
    virtual void unbind_automaton (const aid_t) = 0;
    virtual void set_parameter (const aid_t) = 0;
    virtual void bound (model_interface&,
			system_scheduler_interface&) const = 0;
    virtual void unbound (model_interface&,
			  system_scheduler_interface&) const = 0;
  };

  class unvalued_output_executor_interface :
    public output_executor_interface
  {
  public:
    virtual ~unvalued_output_executor_interface () { }
  };

  template <typename T>
  class valued_output_executor_interface :
    public output_executor_interface
  {
  public:
    virtual ~valued_output_executor_interface () { }
  };
  
  class internal_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~internal_executor_interface () { }
  };
  
  class system_input_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~system_input_executor_interface () { }
  };

  class bind_executor_interface
  {
  public:
    virtual ~bind_executor_interface () { }
    virtual output_executor_interface& get_output () = 0;
    virtual input_executor_interface& get_input () = 0;
  };

}

#endif
