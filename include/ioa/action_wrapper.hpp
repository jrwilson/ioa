#ifndef __action_wrapper_hpp__
#define __action_wrapper_hpp__

#include <ioa/observer.hpp>
#include <ioa/action.hpp>

// TODO:  Eliminate redundancy.

namespace ioa {
  
  template <class C, void (C::*schedule_ptr) () const>
  class auto_scheduler
  {
  private:
    const C& m_ref;

  public:
    auto_scheduler (const C& ref) :
      m_ref (ref)
    { }

    ~auto_scheduler () {
      (m_ref.*schedule_ptr) ();
    }
  };

  template <class C, class P, void (C::*schedule_ptr) (P) const>
  class pauto_scheduler
  {
  private:
    const C& m_ref;
    P m_p;

  public:
    pauto_scheduler (const C& ref,
		    P& p) :
      m_ref (ref),
      m_p (p)
    { }

    ~pauto_scheduler () {
      (m_ref.*schedule_ptr) (m_p);
    }
  };

  enum recent_op_t {
    NOOP,
    BOUND,
    UNBOUND
  };
    
  template <class C, void (C::*effect_ptr) (), void (C::*schedule_ptr) () const>
  struct uv_up_input_wrapper :
    public input,
    public no_value,
    public no_parameter,
    public observable
  {
    recent_op_t recent_op;

    uv_up_input_wrapper () :
      recent_op (NOOP)
    { }

    void operator() (C& c) {
      auto_scheduler<C, schedule_ptr> as (c);
      (c.*effect_ptr) ();
    }
    
    void bound () {
      recent_op = BOUND;
      notify_observers ();
    }
    
    void unbound () {
      recent_op = UNBOUND;
      notify_observers ();
    }
  };

  template <class C, class P, void (C::*effect_ptr) (P), void (C::*schedule_ptr) (P) const>
  struct uv_p_input_wrapper :
    public input,
    public no_value,
    public parameter<P>,
    public observable
  {
    recent_op_t recent_op;
    P recent_parameter;

    uv_p_input_wrapper () :
      recent_op (NOOP)
    { }
    
    void operator() (C& c, P p) {
      pauto_scheduler<C, P, schedule_ptr> as (c, p);
      (c.*effect_ptr) (p);
    }

    void bound (P p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, void (C::*effect_ptr) (aid_t), void (C::*schedule_ptr) (aid_t) const>
  struct uv_ap_input_wrapper :
    public input,
    public no_value,
    public auto_parameter,
    public observable
  {
    recent_op_t recent_op;
    aid_t recent_parameter;

    uv_ap_input_wrapper () :
      recent_op (NOOP)
    { }
    
    void operator() (C& c, aid_t p) {
      pauto_scheduler<C, aid_t, schedule_ptr> as (c, p);
      (c.*effect_ptr) (p);
    }

    void bound (aid_t p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (aid_t p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, class T, void (C::*effect_ptr) (const T&), void (C::*schedule_ptr) () const>
  struct v_up_input_wrapper :
    public input,
    public value<T>,
    public no_parameter,
    public observable
  {
    recent_op_t recent_op;

    v_up_input_wrapper () :
      recent_op (NOOP)
    { }

    void operator() (C& c, const T& t) {
      auto_scheduler<C, schedule_ptr> as (c);
      (c.*effect_ptr) (t);
    }
    
    void bound () {
      recent_op = BOUND;
      notify_observers ();
    }

    void unbound () {
      recent_op = UNBOUND;
      notify_observers ();
    }
  };

  template <class C, class T, class P, void (C::*effect_ptr)(const T&, P), void (C::*schedule_ptr) (P) const>
  struct v_p_input_wrapper :
    public input,
    public value<T>,
    public parameter<P>,
    public observable
  {
    recent_op_t recent_op;
    P recent_parameter;

    v_p_input_wrapper () :
      recent_op (NOOP)
    { }
    
    void operator() (C& c, const T& t, P p) {
      pauto_scheduler<C, P, schedule_ptr> as (c, p);
      (c.*effect_ptr) (t, p);
    }

    void bound (P p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, class T, void (C::*effect_ptr)(const T&, aid_t), void (C::*schedule_ptr) (aid_t) const>
  struct v_ap_input_wrapper :
    public input,
    public value<T>,
    public auto_parameter,
    public observable
  {
    recent_op_t recent_op;
    aid_t recent_parameter;

    v_ap_input_wrapper () :
      recent_op (NOOP)
    { }
    
    void operator() (C& c, const T& t, aid_t p) {
      pauto_scheduler<C, aid_t, schedule_ptr> as (c, p);
      (c.*effect_ptr) (t, p);
    }

    void bound (aid_t p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (aid_t p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, bool (C::*precondition_ptr) () const, void (C::*effect_ptr) (), void (C::*schedule_ptr) () const>
  struct uv_up_output_wrapper :
    public output,
    public no_value,
    public no_parameter,
    public observable
  {
    recent_op_t recent_op;

    uv_up_output_wrapper () :
      recent_op (NOOP)
    { }

    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }

    void operator() (C& c) {
      auto_scheduler<C, schedule_ptr> as (c);
      (c.*effect_ptr) ();
    }

    void bound () {
      recent_op = BOUND;
      notify_observers ();
    }

    void unbound () {
      recent_op = UNBOUND;
      notify_observers ();
    }
  };

  template <class C, class P, bool (C::*precondition_ptr) (P) const, void (C::*effect_ptr)(P), void (C::*schedule_ptr) (P) const>
  struct uv_p_output_wrapper :
    public output,
    public no_value,
    public parameter<P>,
    public observable
  {
    recent_op_t recent_op;
    P recent_parameter;

    uv_p_output_wrapper () :
      recent_op (NOOP)
    { }
    
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
    }

    void operator() (C& c, P p) {
      pauto_scheduler<C, P, schedule_ptr> as (c, p);
      (c.*effect_ptr) (p);
    }
    
    void bound (P p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, bool (C::*precondition_ptr) (aid_t) const, void (C::*effect_ptr)(aid_t), void (C::*schedule_ptr) (aid_t) const>
  struct uv_ap_output_wrapper :
    public output,
    public no_value,
    public auto_parameter,
    public observable
  {
    recent_op_t recent_op;
    aid_t recent_parameter;

    uv_ap_output_wrapper () :
      recent_op (NOOP)
    { }
    
    bool precondition (C& c, aid_t p) const {
      return (c.*precondition_ptr) (p);
    }

    void operator() (C& c, aid_t p) {
      pauto_scheduler<C, aid_t, schedule_ptr> as (c, p);
      (c.*effect_ptr) (p);
    }
    
    void bound (aid_t p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (aid_t p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, class T, bool (C::*precondition_ptr) () const, T (C::*effect_ptr) (void), void (C::*schedule_ptr) () const>
  struct v_up_output_wrapper :
    public output,
    public value<T>,
    public no_parameter,
    public observable
  {
    recent_op_t recent_op;

    v_up_output_wrapper () :
      recent_op (NOOP)
    { }

    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }
    
    T operator() (C& c) {
      auto_scheduler<C, schedule_ptr> as (c);
      return (c.*effect_ptr) ();
    }

    void bound () {
      recent_op = BOUND;
      notify_observers ();
    }

    void unbound () {
      recent_op = UNBOUND;
      notify_observers ();
    }
  };

  template <class C, class T, class P, bool (C::*precondition_ptr) (P) const, T (C::*effect_ptr)(P), void (C::*schedule_ptr) (P) const>
  struct v_p_output_wrapper :
    public output,
    public value<T>,
    public parameter<P>,
    public observable
  {
    recent_op_t recent_op;
    P recent_parameter;

    v_p_output_wrapper () :
      recent_op (NOOP)
    { }
    
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
    }

    T operator() (C& c, P p) {
      pauto_scheduler<C, P, schedule_ptr> as (c, p);
      return (c.*effect_ptr) (p);
    }

    void bound (P p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (P p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, class T, bool (C::*precondition_ptr) (aid_t) const, T (C::*effect_ptr)(aid_t), void (C::*schedule_ptr) (aid_t) const>
  struct v_ap_output_wrapper :
    public output,
    public value<T>,
    public auto_parameter,
    public observable
  {
    recent_op_t recent_op;
    aid_t recent_parameter;

    v_ap_output_wrapper () :
      recent_op (NOOP)
    { }
    
    bool precondition (C& c, aid_t p) const {
      return (c.*precondition_ptr) (p);
    }

    T operator() (C& c, aid_t p) {
      pauto_scheduler<C, aid_t, schedule_ptr> as (c, p);
      return (c.*effect_ptr) (p);
    }

    void bound (aid_t p) {
      recent_op = BOUND;
      recent_parameter = p;
      notify_observers ();
    }

    void unbound (aid_t p) {
      recent_op = UNBOUND;
      recent_parameter = p;
      notify_observers ();
    }
  };

  template <class C, bool (C::*precondition_ptr) () const, void (C::*effect_ptr) (), void (C::*schedule_ptr) () const>
  struct up_internal_wrapper :
    public internal,
    public no_parameter
  {
    bool precondition (C& c) const {
      return (c.*precondition_ptr) ();
    }

    void operator() (C& c) {
      auto_scheduler<C, schedule_ptr> as (c);
      (c.*effect_ptr) ();
    }
  };

  template <class C, class P, bool (C::*precondition_ptr) (P) const, void (C::*effect_ptr)(P), void (C::*schedule_ptr) (P) const>
  struct p_internal_wrapper :
    public internal,
    public parameter<P>
  {
    bool precondition (C& c, P p) const {
      return (c.*precondition_ptr) (p);
    }

    void operator() (C& c, P p) {
      pauto_scheduler<C, P, schedule_ptr> as (c, p);
      (c.*effect_ptr) (p);
    }
  };

  template <class C, class T, void (C::*member_function_ptr)(T const &), void (C::*schedule_ptr) () const>
  struct system_input_wrapper :
    public system_input,
    public value<T>
  {
    void operator() (C& c, T const & t) {
      auto_scheduler<C, schedule_ptr> as (c);
      (c.*member_function_ptr) (t);
    }
  };

  template <class C, class T, bool (C::*precondition_ptr) () const, T (C::*member_function_ptr)(), void (C::*schedule_ptr) () const>
  struct system_output_wrapper :
    public system_output,
    public value<T>
  {
    bool precondition (C& c) {
      return (c.*precondition_ptr) ();
    }
    
    T operator() (C& c) {
      auto_scheduler<C, schedule_ptr> as (c);
      return (c.*member_function_ptr) ();
    }
  };

}

#define UV_UP_INPUT(c, name) \
  typedef ioa::uv_up_input_wrapper<c, &c::name##_effect, &c::name##_schedule> name##_type; \
  name##_type name;

#define UV_P_INPUT(c, name, param_type)	\
  typedef ioa::uv_p_input_wrapper<c, param_type, &c::name##_effect, &c::name##_schedule> name##_type; \
  name##_type name;

#define UV_AP_INPUT(c, name)	\
  typedef ioa::uv_ap_input_wrapper<c, &c::name##_effect, &c::name##_schedule> name##_type; \
  name##_type name;

#define V_UP_INPUT(c, name, type)			\
  typedef ioa::v_up_input_wrapper<c, type, &c::name##_effect, &c::name##_schedule> name##_type;	\
  name##_type name;

#define V_P_INPUT(c, name, type, param_type)	\
  typedef ioa::v_p_input_wrapper<c, type, param_type, &c::name##_effect, &c::name##_schedule> name##_type;	\
  name##_type name;

#define V_AP_INPUT(c, name, type)			\
  typedef ioa::v_ap_input_wrapper<c, type, &c::name##_effect, &c::name##_schedule> name##_type;	\
  name##_type name;

#define UV_UP_OUTPUT(c, name) \
  typedef ioa::uv_up_output_wrapper<c, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name##_type; \
  name##_type name;

#define UV_P_OUTPUT(c, name, param_type)	\
  typedef ioa::uv_p_output_wrapper<c, param_type, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name##_type; \
  name##_type name;

#define UV_AP_OUTPUT(c, name) \
  typedef ioa::uv_ap_output_wrapper<c, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name##_type; \
  name##_type name;

#define V_UP_OUTPUT(c, name, type)			\
  typedef ioa::v_up_output_wrapper<c, type, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name##_type;	\
  name##_type name;

#define V_P_OUTPUT(c, name, type, param_type)	\
  typedef ioa::v_p_output_wrapper<c, type, param_type, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name##_type;	\
  name##_type name;

#define V_AP_OUTPUT(c, name, type)			\
  typedef ioa::v_ap_output_wrapper<c, type, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name##_type;	\
  name##_type name;

#define UP_INTERNAL(c, name) \
  ioa::up_internal_wrapper<c, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name;

#define P_INTERNAL(c, name, param_type)	\
  ioa::p_internal_wrapper<c, param_type, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name;

#define SYSTEM_INPUT(c, name, type)		\
  typedef ioa::system_input_wrapper<c, type, &c::name##_effect, &c::name##_schedule> name##_type;	\
  name##_type name;

#define SYSTEM_OUTPUT(c, name, type)		\
  typedef ioa::system_output_wrapper<c, type, &c::name##_precondition, &c::name##_effect, &c::name##_schedule> name##_type; \
  name##_type name;

#endif
