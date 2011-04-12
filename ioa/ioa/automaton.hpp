#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <set>
#include <boost/thread.hpp>

namespace ioa {

  class system;

  template <class Instance>
  class automaton_handle;

  class abstract_automaton : public boost::mutex {
  public:
    typedef boost::unique_lock<boost::mutex> lock_type;
    virtual ~abstract_automaton() { };
    virtual void* get_instance() const = 0;
  };

  template <class Instance>
  class automaton : public abstract_automaton {
  private:
    std::auto_ptr<Instance> m_instance;
    std::set<automaton_handle<Instance>*> m_handles;
    std::set<const void*> m_parameters;
    // No copy or assignment.
    automaton(const automaton&) { }
    automaton& operator=(const automaton&) { }

  public:
    automaton(Instance* instance = 0) :
      m_instance(instance) { }

    ~automaton() {
      std::for_each (m_handles.begin (),
		     m_handles.end (),
		     std::mem_fun (&automaton_handle<Instance>::invalidate));
    }

    void* get_instance() const {
      return m_instance.get();
    }

    Instance* get_typed_instance() const {
      return m_instance.get();
    }

    bool is_handle(automaton_handle<Instance>* handle) const {
      return m_handles.find (handle) != m_handles.end ();
    }

    bool is_declared(const void* parameter) const {
      return m_parameters.find(parameter) != m_parameters.end();
    }

    void declare(const void* parameter) {
      m_parameters.insert(parameter);
    }

  protected:

    void add(automaton_handle<Instance>* handle) {
      BOOST_ASSERT(handle != 0);
      m_handles.insert (handle);
    }

    void remove(automaton_handle<Instance>* handle) {
      BOOST_ASSERT(handle != 0);
      m_handles.erase (handle);
    }

    friend class automaton_handle<Instance>;
  };

  template <class Instance>
  class automaton_handle {
  private:
    bool m_valid;
    automaton<Instance>* m_automaton;

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
    automaton_handle(automaton<Instance>* automaton = 0)
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

    automaton<Instance>* get_automaton() const {
      return m_automaton;
    }

  protected:
    void invalidate() {
      m_valid = false;
    }

    friend class automaton<Instance>;
  };

}

#endif
