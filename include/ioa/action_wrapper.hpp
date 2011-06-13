#ifndef __action_wrapper_hpp__
#define __action_wrapper_hpp__

#include <ioa/scheduler.hpp>
#include <ioa/observer.hpp>

// TODO:  Eliminate redundancy.

namespace ioa {

  template <class C, void (C::*action_ptr) ()>
  struct uv_up_input_wrapper :
    public input,
    public no_value,
    public no_parameter,
    public observable
  {
    void operator() (C& c) {
      (c.*action_ptr) ();
    }
    
    void bound () {
      notify_observers ();
    }
    
    void unbound () {
      notify_observers ();
    }
  };

  template <class C, class P, void (C::*action_ptr) (P)>
  struct uv_p_input_wrapper :
    public input,
    public no_value,
    public parameter<P>,
    public observable
  {
    P recent_parameter;
    
    void operator() (C& c, P p) {
      (c.*action_ptr) (p);
    }

    void bound (P p) {
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, class T, void (C::*action_ptr) (const T&)>
  struct v_up_input_wrapper :
    public input,
    public value<T>,
    public no_parameter,
    public observable
  {
    void operator() (C& c, const T& t) {
      (c.*action_ptr) (t);
    }
    
    void bound () {
      notify_observers ();
    }

    void unbound () {
      notify_observers ();
    }
  };

  template <class C, class T, class P, void (C::*action_ptr)(const T&, P) >
  struct v_p_input_wrapper :
    public input,
    public value<T>,
    public parameter<P>,
    public observable
  {
    P recent_parameter;
    
    void operator() (C& c, const T& t, P p) {
      (c.*action_ptr) (t, p);
    }

    void bound (P p) {
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, bool (C::*precondition_ptr) () const, void (C::*action_ptr) ()>
  struct uv_up_output_wrapper :
    public output,
    public no_value,
    public no_parameter,
    public observable
  {
    uv_up_output_wrapper C::*m_member_object_ptr;

    uv_up_output_wrapper (uv_up_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }

    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }

    void operator() (C& c) {
      (c.*action_ptr) ();
    }

    void bound () {
      // TODO:  Move this to system.
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr);
      notify_observers ();
    }

    void unbound () {
      notify_observers ();
    }
  };

  template <class C, class P, bool (C::*precondition_ptr) (P) const, void (C::*action_ptr)(P)>
  struct uv_p_output_wrapper :
    public output,
    public no_value,
    public parameter<P>,
    public observable
  {
    uv_p_output_wrapper C::*m_member_object_ptr;
    P recent_parameter;
    
    uv_p_output_wrapper (uv_p_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }
    
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
    }

    void operator() (C& c, P p) {
      (c.*action_ptr) (p);
    }
    
    void bound (P p) {
      recent_parameter = p;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr, p);
      notify_observers ();
    }

    void unbound (P p) {
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, class T, bool (C::*precondition_ptr) () const, T (C::*action_ptr) (void)>
  struct v_up_output_wrapper :
    public output,
    public value<T>,
    public no_parameter,
    public observable
  {
    v_up_output_wrapper C::*m_member_object_ptr;
    
    v_up_output_wrapper (v_up_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }
    
    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }
    
    T operator() (C& c) {
      return (c.*action_ptr) ();
    }

    void bound () {
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr);
      notify_observers ();
    }

    void unbound () {
      notify_observers ();
    }
  };

  template <class C, class T, class P, bool (C::*precondition_ptr) (P) const, T (C::*action_ptr)(P)>
  struct v_p_output_wrapper :
    public output,
    public value<T>,
    public parameter<P>,
    public observable
  {
    v_p_output_wrapper C::*m_member_object_ptr;
    P recent_parameter;
    
    v_p_output_wrapper (v_p_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }
    
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
    }

    T operator() (C& c, P p) {
      return (c.*action_ptr) (p);
    }

    void bound (P p) {
      recent_parameter = p;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr, p);
      notify_observers ();
    }

    void unbound (P p) {
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, bool (C::*precondition_ptr) () const, void (C::*action_ptr) ()>
  struct up_internal_wrapper :
    public internal,
    public no_parameter
  {
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
