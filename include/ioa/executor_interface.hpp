#ifndef __executor_interface_hpp__
#define __executor_interface_hpp__

#include <ioa/aid.hpp>
#include <cstdlib>

namespace ioa {

  class scheduler_interface;

  class action_executor_interface
  {
  public:
    virtual ~action_executor_interface () { }
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
    virtual input_executor_interface* clone () const = 0;
    virtual void set_auto_parameter (const aid_t) = 0;
    virtual void lock () = 0;
    virtual void unlock () = 0;
  };

  class unvalued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~unvalued_input_executor_interface () { }
    virtual void operator() (scheduler_interface&) const = 0;
  };

  template <typename T>
  class valued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~valued_input_executor_interface () { }
    virtual void operator() (scheduler_interface&, const T& t) const = 0;
  };

  class local_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~local_executor_interface () { }
    //virtual void operator() (scheduler_interface&) const = 0;
  };
  
  class output_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~output_executor_interface () { }
    virtual output_executor_interface* clone () const = 0;
    virtual void set_auto_parameter (const aid_t) = 0;
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
