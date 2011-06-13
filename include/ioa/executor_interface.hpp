#ifndef __executor_interface_hpp__
#define __executor_interface_hpp__

#include <ioa/action.hpp>

namespace ioa {

  class action_executor_interface
  {
  public:
    virtual ~action_executor_interface () { }
    virtual bool fetch_instance () = 0;
    virtual const action_interface& get_action () const = 0;
  };

  class input_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~input_executor_interface () { }
    virtual input_executor_interface* clone () const = 0;
    virtual void bound () const = 0;
    virtual void unbound () const = 0;
  };

  class unvalued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~unvalued_input_executor_interface () { }
    virtual void operator() () const = 0;
  };

  template <typename T>
  class valued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~valued_input_executor_interface () { }
    virtual void operator() (const T& t) const = 0;
  };

  class local_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~local_executor_interface () { }
    virtual void operator() () const = 0;
  };
  
  class output_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~output_executor_interface () { }
    virtual output_executor_interface* clone () const = 0;
    virtual bool involves_output (const action_interface&) const = 0;
    virtual bool involves_input (const action_interface&) const = 0;
    virtual bool involves_input_automaton (const aid_t) const = 0;
    virtual bool involves_binding (const action_interface&,
				   const action_interface&,
				   const aid_t) const = 0;
    virtual bool involves_aid_key (const aid_t,
				   void* const) const = 0;
    virtual bool empty () const = 0;
    virtual size_t size () const = 0;
    virtual void bind (const input_executor_interface&,
		       const aid_t,
		       void* const) = 0;
    virtual void unbind (const aid_t,
			 void* const) = 0;
    virtual void unbind_automaton (const aid_t) = 0;
    virtual void bound () const = 0;
    virtual void unbound () const = 0;
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
  
  class event_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~event_executor_interface () { }
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
