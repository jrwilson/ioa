#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <set>
#include <map>
#include <boost/thread.hpp>

namespace ioa {

  class automaton;

  class handle_interface
  {
  public:
    virtual ~handle_interface () { }
    virtual void invalidate () = 0;
    virtual bool valid () const = 0;
    virtual automaton* get_automaton () const = 0;
  };

  class automaton_handle_interface :
    public handle_interface
  {
  public:
    virtual ~automaton_handle_interface () { }
  };

  class parameter_handle_interface :
    public handle_interface
  {
  public:
    virtual ~parameter_handle_interface () { }
  };

  class automaton :
    public boost::mutex {

  private:
    boost::mutex m_handle_mutex;
    std::set<automaton_handle_interface*> m_handles;
    std::set<const void*> m_parameters;
    typedef std::map<parameter_handle_interface*, const void*> parameter_handle_set_type;
    parameter_handle_set_type m_parameter_handles;
    
  public:
    typedef boost::unique_lock<boost::mutex> lock_type;

    virtual ~automaton () {
      std::for_each (m_handles.begin (),
		     m_handles.end (),
		     std::mem_fun (&automaton_handle_interface::invalidate));
      for (parameter_handle_set_type::iterator pos = m_parameter_handles.begin ();
	   pos != m_parameter_handles.end ();
	   ++pos) {
	(pos->first)->invalidate ();
      }
    };

    bool is_handle (automaton_handle_interface* handle) {
      return m_handles.find (handle) != m_handles.end ();
    }

    bool is_declared (const void* parameter) const {
      return m_parameters.find (parameter) != m_parameters.end();
    }

    void declare (const void* parameter) {
      m_parameters.insert (parameter);
    }

    virtual void* get_instance () const = 0;

    // TODO:  Make it so only handles can call these functions.
    void add (automaton_handle_interface* handle) {
      boost::unique_lock<boost::mutex> lock (m_handle_mutex);
      BOOST_ASSERT (handle != 0);
      m_handles.insert (handle);
    }

    void add (parameter_handle_interface* handle, const void* parameter) {
      boost::unique_lock<boost::mutex> lock (m_handle_mutex);
      BOOST_ASSERT (handle != 0);
      BOOST_ASSERT (is_declared (parameter));
      m_parameter_handles.insert (std::make_pair (handle, parameter));
    }

    void remove (automaton_handle_interface* handle) {
      boost::unique_lock<boost::mutex> lock (m_handle_mutex);
      BOOST_ASSERT (handle != 0);
      m_handles.erase (handle);
    }
    
    void remove (parameter_handle_interface* handle, const void* parameter) {
      boost::unique_lock<boost::mutex> lock (m_handle_mutex);
      BOOST_ASSERT (handle != 0);
      BOOST_ASSERT (is_declared (parameter));
      m_parameter_handles.erase (handle);
    }

  };
  
  template <class Instance>
  class typed_automaton :
    public automaton {
    
  private:
    std::auto_ptr<Instance> m_instance;

    // No copy or assignment.
    typed_automaton (const typed_automaton&) { }
    typed_automaton& operator= (const typed_automaton&) { return *this; }

  public:
    typed_automaton (Instance* instance = 0) :
      m_instance (instance) { }

    ~typed_automaton () { }

    void* get_instance () const {
      return m_instance.get ();
    }

    Instance* get_typed_instance () const {
      return m_instance.get ();
    }

  };

  template <class A>
  class automaton_handle_impl :
    public automaton_handle_interface {
    
  private:
    bool m_valid;
    A* m_automaton;

    void add() {
      if (m_valid) {
	m_automaton->add(this);
      }
    }

    void remove() {
      if (m_valid) {
	m_automaton->remove(this);
      }
    }

  public:
    automaton_handle_impl(A* automaton = 0)
      : m_automaton(automaton) {
      m_valid = m_automaton != 0;
      add();
    }

    automaton_handle_impl(const automaton_handle_impl& handle)
      : m_valid(handle.m_valid),
	m_automaton(handle.m_automaton) {
      add();
    }

    automaton_handle_impl& operator=(const automaton_handle_impl& handle) {
      if (this != &handle) {
	remove();
	m_valid = handle.m_valid;
	m_automaton = handle.m_automaton;
	add();
      }
      return *this;
    }

    ~automaton_handle_impl() {
      remove ();
    }

    bool valid () const {
      return m_valid;
    }

    bool operator==(const automaton_handle_impl& handle) const {
      BOOST_ASSERT (m_valid);
      BOOST_ASSERT (handle.m_valid);
      return m_automaton == handle.m_automaton;
    }

    bool operator!=(const automaton_handle_impl& handle) const {
      BOOST_ASSERT (m_valid);
      BOOST_ASSERT (handle.m_valid);
      return m_automaton != handle.m_automaton;
    }

    bool operator<(const automaton_handle_impl& handle) const {
      BOOST_ASSERT (m_valid);
      BOOST_ASSERT (handle.m_valid);
      return m_automaton < handle.m_automaton;
    }

    A* get_automaton () const {
      BOOST_ASSERT (m_valid);
      return m_automaton;
    }

  protected:
    void invalidate() {
      m_valid = false;
    }

    //friend class typename A;
  };

  template <class A, class P>
  class parameter_handle_impl :
    public parameter_handle_interface {
  private:
    bool m_valid;
    A* m_automaton;
    P* m_parameter;

    void add () {
      if (m_valid) {
	m_automaton->add (this, m_parameter);
      }
    }

    void remove () {
      if (m_valid) {
	m_automaton->remove (this, m_parameter);
      }
    }

  public:
    parameter_handle_impl (A* automaton = 0,
			   P* parameter = 0) :
      m_automaton (automaton),
      m_parameter (parameter) {
      m_valid = m_automaton != 0;
      add ();
    }

    parameter_handle_impl (const parameter_handle_impl& handle)
      : m_valid (handle.m_valid),
	m_automaton (handle.m_automaton),
	m_parameter (handle.m_parameter) {
      add ();
    }

    parameter_handle_impl& operator=(const parameter_handle_impl& handle) {
      if (this != &handle) {
	remove();
	m_valid = handle.m_valid;
	m_automaton = handle.m_automaton;
	m_parameter = handle.m_parameter;
	add ();
      }
      return *this;
    }

    ~parameter_handle_impl () {
      remove ();
    }

    bool valid () const {
      return m_valid;
    }

    bool operator== (const parameter_handle_impl& handle) const {
      BOOST_ASSERT (m_valid);
      BOOST_ASSERT (handle.m_valid);
      return m_automaton == handle.m_automaton && m_parameter == handle.m_parameter;
    }

    bool operator!= (const parameter_handle_impl& handle) const {
      BOOST_ASSERT (m_valid);
      BOOST_ASSERT (handle.m_valid);
      return !(m_automaton == handle.m_automaton && m_parameter == handle.m_parameter);
    }

    A* get_automaton () const {
      BOOST_ASSERT (m_valid);
      return m_automaton;
    }

    P* get_parameter () const {
      BOOST_ASSERT (m_valid);
      return m_parameter;
    }

  protected:
    void invalidate () {
      m_valid = false;
    }

  };

  // Untyped handles for the system.
  class generic_automaton_handle :
    public automaton_handle_impl<automaton> {
  public:
    generic_automaton_handle (automaton* a = 0) :
      automaton_handle_impl<automaton> (a)
    { }
  };

  std::ostream& operator<<(std::ostream& output, const ioa::generic_automaton_handle& ai);

  template <class Parameter>
  class generic_parameter_handle :
    public parameter_handle_impl<automaton, Parameter> {
  public:
    generic_parameter_handle (automaton* a = 0,
			      Parameter* parameter = 0) :
      parameter_handle_impl<automaton, Parameter> (a, parameter)
    { }

    generic_parameter_handle (const generic_parameter_handle& handle) :
      parameter_handle_impl<automaton, Parameter> (handle)
    { }

    operator generic_automaton_handle() const {
      if (this->valid ()) {
	return generic_automaton_handle (this->get_automaton ());
      }
      else {
	return generic_automaton_handle ();
      }
    }
  };

  // Typed handles for the user.
  template <class Instance>
  class automaton_handle :
    public automaton_handle_impl<typed_automaton<Instance> > {
  public:
    automaton_handle (typed_automaton<Instance>* automaton = 0) :
      automaton_handle_impl<typed_automaton<Instance> > (automaton)
    { }

    operator generic_automaton_handle() const {
      if (this->valid ()) {
	return generic_automaton_handle (this->get_automaton ());
      }
      else {
	return generic_automaton_handle ();
      }
    }
  };

  template <class Instance, class Parameter>
  class parameter_handle :
    public parameter_handle_impl<typed_automaton<Instance>, Parameter> {
  public:
    parameter_handle (typed_automaton<Instance>* automaton = 0,
		      Parameter* parameter = 0) :
      parameter_handle_impl<typed_automaton<Instance>, Parameter> (automaton, parameter)
    { }

    operator generic_parameter_handle<Parameter>() const {
      if (this->valid ()) {
	return generic_parameter_handle<Parameter> (this->get_automaton (), this->get_parameter ());
      }
      else {
	return generic_parameter_handle<Parameter> ();
      }
    }
  };

}

#endif
