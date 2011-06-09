#ifndef __action_wrapper_hpp__
#define __action_wrapper_hpp__

#include <ioa/scheduler.hpp>
#include <ioa/observer.hpp>

// TODO:  Eliminate redundancy.

namespace ioa {

  template <class C, void (C::*action_ptr) ()>
  class uv_up_input_wrapper :
    public input,
    public no_value,
    public no_parameter,
    public observable
  {
  private:
<<<<<<< HEAD
    C& m_c;
    void (C::*m_member_function_ptr)();
    bool m_bind_status;

=======
    bool m_bound;
    
>>>>>>> new_syscall_interface
  public:
    uv_up_input_wrapper () :
      m_bound (false)
    { }
<<<<<<< HEAD

    void operator() () {
      (m_c.*m_member_function_ptr) ();
=======
    
    void operator() (C& c) {
      (c.*action_ptr) ();
>>>>>>> new_syscall_interface
    }

    void bound () {
      assert (!m_bound);
      m_bound = true;
      notify_observers ();
    }

    void unbound () {
      assert (m_bound);
      m_bound = false;
      notify_observers ();
    }

    bool is_bound () const {
      return m_bound;
    }

  };

<<<<<<< HEAD
  template <class C, typename P>
=======
  template <class C, class P, void (C::*action_ptr) (P)>
>>>>>>> new_syscall_interface
  class uv_p_input_wrapper :
    public input,
    public no_value,
    public parameter<P>,
    public observable
  {
  private:
    std::set<P> m_parameters;
<<<<<<< HEAD

  public:
    uv_p_input_wrapper (C& c,
			void (C::*member_function_ptr)(P),
			uv_p_input_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }

    void operator() (P p) {
      (m_c.*m_member_function_ptr) (p);
=======
    P m_recent_parameter;
    
  public:
    void operator() (C& c, P p) {
      (c.*action_ptr) (p);
>>>>>>> new_syscall_interface
    }

    void bound (P p) {
      assert (m_parameters.count (p) == 0);
      m_parameters.insert (p);
      m_recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      assert (m_parameters.count (p) == 1);
      m_parameters.erase (p);
      m_recent_parameter = p;
      notify_observers ();
    }

<<<<<<< HEAD
=======
    P recent_parameter () const {
      return m_recent_parameter;
    }

>>>>>>> new_syscall_interface
    bool is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

<<<<<<< HEAD
  template <class C, typename T>
=======
  template <class C, class T, void (C::*action_ptr) (const T&)>
>>>>>>> new_syscall_interface
  class v_up_input_wrapper :
    public input,
    public value<T>,
    public no_parameter,
    public observable
  {
  private:
<<<<<<< HEAD
    C& m_c;
    void (C::*m_member_function_ptr)(const T&);
    bool m_bind_status;

=======
    bool m_bound;
    
>>>>>>> new_syscall_interface
  public:
    v_up_input_wrapper () :
      m_bound (false)
    { }
<<<<<<< HEAD

    void operator() (const T& t) {
      (m_c.*m_member_function_ptr) (t);
=======
    
    void operator() (C& c, const T& t) {
      (c.*action_ptr) (t);
>>>>>>> new_syscall_interface
    }

    void bound () {
      assert (!m_bound);
      m_bound = true;
      notify_observers ();
    }

    void unbound () {
      assert (m_bound);
      m_bound = false;
      notify_observers ();
    }

    bool is_bound () const {
      return m_bound;
    }
  };

<<<<<<< HEAD
  template <class C, typename T, typename P >
=======
  template <class C, class T, class P, void (C::*action_ptr)(const T&, P) >
>>>>>>> new_syscall_interface
  class v_p_input_wrapper :
    public input,
    public value<T>,
    public parameter<P>,
    public observable
  {
  private:
    std::set<P> m_parameters;
<<<<<<< HEAD

  public:
    v_p_input_wrapper (C& c,
		       void (C::*member_function_ptr)(const T&, P),
		       v_p_input_wrapper C::*member_object_ptr)
      : m_c (c),
 	m_member_function_ptr (member_function_ptr)
    { }

    void operator() (const T& t, P p) {
      (m_c.*m_member_function_ptr) (t, p);
=======
    P m_recent_parameter;
    
  public:
    void operator() (C& c, const T& t, P p) {
      (c.*action_ptr) (t, p);
>>>>>>> new_syscall_interface
    }

    void bound (P p) {
      assert (m_parameters.count (p) == 0);
      m_parameters.insert (p);
      m_recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      assert (m_parameters.count (p) == 1);
      m_parameters.erase (p);
      m_recent_parameter = p;
      notify_observers ();
    }

<<<<<<< HEAD
    bool is_bound (P p) const {
=======
    P recent_parameter () const {
      return m_recent_parameter;
    }

    void is_bound (P p) const {
>>>>>>> new_syscall_interface
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, bool (C::*precondition_ptr) () const, void (C::*action_ptr) ()>
  class uv_up_output_wrapper :
    public output,
    public no_value,
    public no_parameter,
    public observable
  {
  private:
    uv_up_output_wrapper C::*m_member_object_ptr;
<<<<<<< HEAD
    bool m_bind_status;

=======
    size_t m_bind_count;
    
>>>>>>> new_syscall_interface
  public:
    uv_up_output_wrapper (uv_up_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr),
      m_bind_count (0)
    { }
<<<<<<< HEAD

    bool operator() () {
      return (m_c.*m_member_function_ptr) ();
=======
    
    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }

    void operator() (C& c) {
      (c.*action_ptr) ();
>>>>>>> new_syscall_interface
    }

    void bound () {
      ++m_bind_count;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr);
      notify_observers ();
    }

    void unbound () {
      assert (m_bind_count > 0);
      --m_bind_count;
      notify_observers ();
    }

    size_t bind_count () const {
      return m_bind_count;
    }
  };

<<<<<<< HEAD
  template <class C, typename P>
=======
  template <class C, class P, bool (C::*precondition_ptr) (P) const, void (C::*action_ptr)(P)>
>>>>>>> new_syscall_interface
  class uv_p_output_wrapper :
    public output,
    public no_value,
    public parameter<P>,
    public observable
  {
  private:
    uv_p_output_wrapper C::*m_member_object_ptr;
<<<<<<< HEAD
    std::set<P> m_parameters;

=======
    std::map<P, size_t> m_parameters;
    P m_recent_parameter;
    
>>>>>>> new_syscall_interface
  public:
    uv_p_output_wrapper (uv_p_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }
<<<<<<< HEAD

    bool operator() (P p) {
      return (m_c.*m_member_function_ptr) (p);
=======
    
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
>>>>>>> new_syscall_interface
    }

    void operator() (C& c, P p) {
      (c.*action_ptr) (p);
    }
    
    void bound (P p) {
      if (m_parameters.find (p) == m_parameters.end ()) {
	m_parameters.insert (std::make_pair (p, 0));
      }
      ++m_parameters[p];
      m_recent_parameter = p;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr, p);
      notify_observers ();
    }

    void unbound (P p) {
      assert (m_parameters.find (p) != m_parameters.end ());
      --m_parameters[p];
      if (m_parameters[p] == 0) {
	m_parameters.erase (p);
      }
      m_recent_parameter = p;
      notify_observers ();
    }

<<<<<<< HEAD
    bool is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, typename T>
=======
    P recent_parameter () const {
      return m_recent_parameter;
    }

    size_t bind_count (P p) const {
      typename std::map<P, size_t>::const_iterator pos = m_parameters.find (p);
      if (pos != m_parameters.end ()) {
	return pos->second;
      }
      else {
	return 0;
      }
    }
  };

  template <class C, class T, bool (C::*precondition_ptr) () const, T (C::*action_ptr) (void)>
>>>>>>> new_syscall_interface
  class v_up_output_wrapper :
    public output,
    public value<T>,
    public no_parameter,
    public observable
  {
  private:
    v_up_output_wrapper C::*m_member_object_ptr;
<<<<<<< HEAD
    bool m_bind_status;

=======
    size_t m_bind_count;
    
>>>>>>> new_syscall_interface
  public:
    v_up_output_wrapper (v_up_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr),
      m_bind_count (0)
    { }
<<<<<<< HEAD

    std::pair<bool, T> operator() () {
      return (m_c.*m_member_function_ptr) ();
=======
    
    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }
    
    T operator() (C& c) {
      return (c.*action_ptr) ();
>>>>>>> new_syscall_interface
    }

    void bound () {
      ++m_bind_count;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr);
      notify_observers ();
    }

    void unbound () {
      assert (m_bind_count > 0);
      --m_bind_count;
      notify_observers ();
    }

    size_t bind_count () const {
      return m_bind_count;
    }
  };

<<<<<<< HEAD
  template <class C, typename T, typename P>
=======
  template <class C, class T, class P, bool (C::*precondition_ptr) (P) const, T (C::*action_ptr)(P)>
>>>>>>> new_syscall_interface
  class v_p_output_wrapper :
    public output,
    public value<T>,
    public parameter<P>,
    public observable
  {
  private:
    v_p_output_wrapper C::*m_member_object_ptr;
<<<<<<< HEAD
    std::set<P> m_parameters;

=======
    std::map<P, size_t> m_parameters;
    P m_recent_parameter;
    
>>>>>>> new_syscall_interface
  public:
    v_p_output_wrapper (v_p_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }
<<<<<<< HEAD

    std::pair<bool, T> operator() (P p) {
      return (m_c.*m_member_function_ptr) (p);
=======
    
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
    }

    T operator() (C& c, P p) {
      return (c.*action_ptr) (p);
>>>>>>> new_syscall_interface
    }

    void bound (P p) {
      if (m_parameters.find (p) == m_parameters.end ()) {
	m_parameters.insert (std::make_pair (p, 0));
      }
      ++m_parameters[p];
      m_recent_parameter = p;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr, p);
      notify_observers ();
    }

    void unbound (P p) {
      assert (m_parameters.find (p) != m_parameters.end ());
      --m_parameters[p];
      if (m_parameters[p] == 0) {
	m_parameters.erase (p);
      }
      m_recent_parameter = p;
      notify_observers ();
    }

<<<<<<< HEAD
    bool is_bound (P p) const {
      return m_parameters.count (p) != 0;
=======
    P recent_parameter () const {
      return m_recent_parameter;
    }

    size_t bind_count (P p) const {
      typename std::map<P, size_t>::const_iterator pos = m_parameters.find (p);
      if (pos != m_parameters.end ()) {
	return pos->second;
      }
      else {
	return 0;
      }
>>>>>>> new_syscall_interface
    }

  };

  template <class C, bool (C::*precondition_ptr) () const, void (C::*action_ptr) ()>
  struct up_internal_wrapper :
    public internal,
    public no_parameter
  {
<<<<<<< HEAD
  private:
    C& m_c;
    void (C::*m_member_function_ptr)();

  public:
    up_internal_wrapper (C& c,
			 void (C::*member_function_ptr)(),
			 up_internal_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }

    void operator() () {
      (m_c.*m_member_function_ptr) ();
    }
  };

  template <class C, typename P>
  class p_internal_wrapper :
    public internal,
    public parameter<P>
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)(P);

  public:
    p_internal_wrapper (C& c,
			void (C::*member_function_ptr)(P),
			p_internal_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }

    void operator() (P p) {
      (m_c.*m_member_function_ptr) (p);
    }
  };

  template <class C>
  class uv_event_wrapper :
    public event,
    public no_value
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)();

  public:
    uv_event_wrapper (C& c,
		      void (C::*member_function_ptr)(),
		      uv_event_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }

    void operator() () {
      (m_c.*m_member_function_ptr) ();
    }
  };
=======
    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }

    void operator() (C& c) {
      (c.*action_ptr) ();
    }
  };

  template <class C, class P, bool (C::*precondition_ptr) (P) const, void (C::*action_ptr)(P)>
  struct p_internal_wrapper :
    public internal,
    public parameter<P>
  {
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
    }

    void operator() (C& c, P p) {
      (c.*action_ptr) (p);
    }
  };

  // template <class C>
  // class uv_event_wrapper :
  //   public event,
  //   public no_value
  // {
  // private:
  //   C& m_c;
  //   void (C::*m_action_ptr)();
    
  // public:
  //   uv_event_wrapper (C& c,
  // 		      void (C::*action_ptr)(),
  // 		      uv_event_wrapper C::*member_object_ptr) :
  //     m_c (c),
  //     m_action_ptr (action_ptr)
  //   { }
    
  //   void operator() (C&) {
  //     (m_c.*m_action_ptr) ();
  //   }
  // };
>>>>>>> new_syscall_interface

}

#define UV_UP_INPUT(c, name) \
  typedef ioa::uv_up_input_wrapper<c, &c::name##_action> name##_type; \
  name##_type name;

#define UV_P_INPUT(c, name, param_type)	\
  typedef ioa::uv_p_input_wrapper<c, param_type, &c::name##_action> name##_type; \
  name##_type name;

#define V_UP_INPUT(c, name, type)			\
  typedef ioa::v_up_input_wrapper<c, type, &c::name##_action> name##_type;	\
  name##_type name;

#define V_P_INPUT(c, name, type, param_type)	\
  typedef ioa::v_p_input_wrapper<c, type, param_type, &c::name##_action> name##_type;	\
  name##_type name;

#define UV_UP_OUTPUT(c, name) \
  typedef ioa::uv_up_output_wrapper<c, &c::name##_precondition, &c::name##_action> name##_type; \
  name##_type name;

#define UV_P_OUTPUT(c, name, param_type)	\
  typedef ioa::uv_p_output_wrapper<c, param_type, &c::name##_precondition, &c::name##_action> name##_type; \
  name##_type name;

#define V_UP_OUTPUT(c, name, type)			\
  typedef ioa::v_up_output_wrapper<c, type, &c::name##_precondition, &c::name##_action> name##_type;	\
  name##_type name;

#define V_P_OUTPUT(c, name, type, param_type)	\
  typedef ioa::v_p_output_wrapper<c, type, param_type, &c::name##_precondition, &c::name##_action> name##_type;	\
  name##_type name;

#define UP_INTERNAL(c, name) \
  ioa::up_internal_wrapper<c, &c::name##_precondition, &c::name##_action> name;

#define P_INTERNAL(c, name, param_type)	\
  ioa::p_internal_wrapper<c, param_type, &c::name##_precondition, &c::name##_action> name;

#define UV_EVENT(c, name)			\
  public: \
  typedef ioa::uv_event_wrapper<c> name##_type;	\
  name##_type name; \
  private: \
  void _##name ()

#define V_EVENT(c, name, type, var)			\
  public: \
  typedef ioa::v_event_wrapper<c, type> name##_type;	\
  name##_type name; \
  private: \
  void _##name (const type & var)

#define ACTION(c, name) name (&c::name)

#endif
