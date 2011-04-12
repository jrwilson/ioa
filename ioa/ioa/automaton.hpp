#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <set>
#include <map>
#include <boost/thread.hpp>

namespace ioa {

  class automaton;

  class handle_interface {
  public:
    virtual ~handle_interface () { }
    virtual void invalidate () = 0;
    virtual bool valid () const = 0;
    virtual automaton* get_automaton () const = 0;
  };

  class automaton :
    public boost::mutex {

  private:
    boost::mutex m_handle_mutex;
    std::set<handle_interface*> m_handles;
    std::set<const void*> m_parameters;
    typedef std::map<handle_interface*, const void*> parameter_handle_set_type;
    parameter_handle_set_type m_parameter_handles;
    
  public:
    typedef boost::unique_lock<boost::mutex> lock_type;

    virtual ~automaton () {
      std::for_each (m_handles.begin (),
		     m_handles.end (),
		     std::mem_fun (&handle_interface::invalidate));
      for (parameter_handle_set_type::iterator pos = m_parameter_handles.begin ();
	   pos != m_parameter_handles.end ();
	   ++pos) {
	(pos->first)->invalidate ();
      }
    };

    bool is_handle (handle_interface* handle) const {
      return m_handles.find (handle) != m_handles.end () ||
	m_parameter_handles.find (handle) != m_parameter_handles.end ();
    }

    bool is_declared (const void* parameter) const {
      return m_parameters.find (parameter) != m_parameters.end();
    }

    void declare (const void* parameter) {
      m_parameters.insert (parameter);
    }

    virtual void* get_instance () const = 0;

    // TODO:  Make it so only handles can call these functions.
    void add (handle_interface* handle) {
      boost::unique_lock<boost::mutex> lock (m_handle_mutex);
      BOOST_ASSERT (handle != 0);
      m_handles.insert (handle);
    }

    void add (handle_interface* handle, const void* parameter) {
      boost::unique_lock<boost::mutex> lock (m_handle_mutex);
      BOOST_ASSERT (handle != 0);
      BOOST_ASSERT (is_declared (parameter));
      m_parameter_handles.insert (std::make_pair (handle, parameter));
    }

    void remove (handle_interface* handle) {
      boost::unique_lock<boost::mutex> lock (m_handle_mutex);
      BOOST_ASSERT (handle != 0);
      m_handles.erase (handle);
    }
    
    void remove (handle_interface* handle, const void* parameter) {
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

  template <class Instance>
  class automaton_handle :
    public handle_interface {

  private:
    bool m_valid;
    typed_automaton<Instance>* m_automaton;

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
    automaton_handle(typed_automaton<Instance>* automaton = 0)
      : m_automaton(automaton) {
      m_valid = m_automaton != 0;
      add();
    }

    automaton_handle(const automaton_handle& handle)
      : m_valid(handle.m_valid),
	m_automaton(handle.m_automaton) {
      add();
    }

    automaton_handle& operator=(const automaton_handle& handle) {
      if (this != &handle) {
	remove();
	m_valid = handle.m_valid;
	m_automaton = handle.m_automaton;
	add();
      }
      return *this;
    }

    ~automaton_handle() {
      remove ();
    }

    bool valid() const {
      return m_valid;
    }

    bool operator==(const automaton_handle& handle) const {
      return m_valid && handle.m_valid && m_automaton == handle.m_automaton;
    }

    bool operator!=(const automaton_handle& handle) const {
      return !(*this == handle);
    }

    typed_automaton<Instance>* get_automaton() const {
      return m_automaton;
    }

  protected:
    void invalidate() {
      m_valid = false;
    }

    friend class typed_automaton<Instance>;
  };

  template <class Instance, class Parameter>
  class parameter_handle :
    public handle_interface {
  private:
    bool m_valid;
    typed_automaton<Instance>* m_automaton;
    Parameter* m_parameter;

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
    parameter_handle (typed_automaton<Instance>* automaton = 0,
		      Parameter* parameter = 0) :
      m_automaton (automaton),
      m_parameter (parameter) {
      m_valid = m_automaton != 0;
      add ();
    }

    parameter_handle (const parameter_handle& handle)
      : m_valid (handle.m_valid),
	m_automaton (handle.m_automaton),
	m_parameter (handle.m_parameter) {
      add ();
    }

    parameter_handle& operator=(const parameter_handle& handle) {
      if (this != &handle) {
	remove();
	m_valid = handle.m_valid;
	m_automaton = handle.m_automaton;
	m_parameter = handle.m_parameter;
	add ();
      }
      return *this;
    }

    ~parameter_handle () {
      remove ();
    }

    bool valid () const {
      return m_valid;
    }

    bool operator== (const parameter_handle& handle) const {
      return m_valid && handle.m_valid && m_automaton == handle.m_automaton && m_parameter == handle.m_parameter;
    }

    bool operator!= (const parameter_handle& handle) const {
      return !(*this == handle);
    }

    typed_automaton<Instance>* get_automaton () const {
      return m_automaton;
    }

    Parameter* get_parameter () const {
      return m_parameter;
    }

  protected:
    void invalidate () {
      m_valid = false;
    }

  };

}

#endif
