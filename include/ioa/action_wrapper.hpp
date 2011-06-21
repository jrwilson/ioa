#ifndef __action_wrapper_hpp__
#define __action_wrapper_hpp__

#include <ioa/observer.hpp>
#include <ioa/action.hpp>

// TODO:  Eliminate redundancy.

namespace ioa {

  enum recent_op_t {
    NOOP,
    BOUND,
    UNBOUND
  };
    
  template <class C, void (C::*action_ptr) ()>
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
      (c.*action_ptr) ();
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

  template <class C, class P, void (C::*action_ptr) (P)>
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
      (c.*action_ptr) (p);
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

  template <class C, void (C::*action_ptr) (aid_t)>
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
      (c.*action_ptr) (p);
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

  template <class C, class T, void (C::*action_ptr) (const T&)>
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
      (c.*action_ptr) (t);
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

  template <class C, class T, class P, void (C::*action_ptr)(const T&, P) >
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
      (c.*action_ptr) (t, p);
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

  template <class C, class T, void (C::*action_ptr)(const T&, aid_t) >
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
      (c.*action_ptr) (t, p);
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

  template <class C, bool (C::*precondition_ptr) () const, void (C::*action_ptr) ()>
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
      (c.*action_ptr) ();
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

  template <class C, class P, bool (C::*precondition_ptr) (P) const, void (C::*action_ptr)(P)>
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
      (c.*action_ptr) (p);
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

  template <class C, bool (C::*precondition_ptr) (aid_t) const, void (C::*action_ptr)(aid_t)>
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
      (c.*action_ptr) (p);
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

  template <class C, class T, bool (C::*precondition_ptr) () const, T (C::*action_ptr) (void)>
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
      return (c.*action_ptr) ();
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

  template <class C, class T, class P, bool (C::*precondition_ptr) (P) const, T (C::*action_ptr)(P)>
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
      return (c.*action_ptr) (p);
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

  template <class C, class T, bool (C::*precondition_ptr) (aid_t) const, T (C::*action_ptr)(aid_t)>
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
      return (c.*action_ptr) (p);
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

}

#define UV_UP_INPUT(c, name) \
  typedef ioa::uv_up_input_wrapper<c, &c::name##_effect> name##_type; \
  name##_type name;

#define UV_P_INPUT(c, name, param_type)	\
  typedef ioa::uv_p_input_wrapper<c, param_type, &c::name##_effect> name##_type; \
  name##_type name;

#define UV_AP_INPUT(c, name)	\
  typedef ioa::uv_ap_input_wrapper<c, &c::name##_effect> name##_type; \
  name##_type name;

#define V_UP_INPUT(c, name, type)			\
  typedef ioa::v_up_input_wrapper<c, type, &c::name##_effect> name##_type;	\
  name##_type name;

#define V_P_INPUT(c, name, type, param_type)	\
  typedef ioa::v_p_input_wrapper<c, type, param_type, &c::name##_effect> name##_type;	\
  name##_type name;

#define V_AP_INPUT(c, name, type)			\
  typedef ioa::v_ap_input_wrapper<c, type, &c::name##_effect> name##_type;	\
  name##_type name;

#define UV_UP_OUTPUT(c, name) \
  typedef ioa::uv_up_output_wrapper<c, &c::name##_precondition, &c::name##_effect> name##_type; \
  name##_type name;

#define UV_P_OUTPUT(c, name, param_type)	\
  typedef ioa::uv_p_output_wrapper<c, param_type, &c::name##_precondition, &c::name##_effect> name##_type; \
  name##_type name;

#define UV_AP_OUTPUT(c, name) \
  typedef ioa::uv_ap_output_wrapper<c, &c::name##_precondition, &c::name##_effect> name##_type; \
  name##_type name;

#define V_UP_OUTPUT(c, name, type)			\
  typedef ioa::v_up_output_wrapper<c, type, &c::name##_precondition, &c::name##_effect> name##_type;	\
  name##_type name;

#define V_P_OUTPUT(c, name, type, param_type)	\
  typedef ioa::v_p_output_wrapper<c, type, param_type, &c::name##_precondition, &c::name##_effect> name##_type;	\
  name##_type name;

#define V_AP_OUTPUT(c, name, type)			\
  typedef ioa::v_ap_output_wrapper<c, type, &c::name##_precondition, &c::name##_effect> name##_type;	\
  name##_type name;

#define UP_INTERNAL(c, name) \
  ioa::up_internal_wrapper<c, &c::name##_precondition, &c::name##_effect> name;

#define P_INTERNAL(c, name, param_type)	\
  ioa::p_internal_wrapper<c, param_type, &c::name##_precondition, &c::name##_effect> name;

#endif
