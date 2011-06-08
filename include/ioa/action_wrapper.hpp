#ifndef __action_wrapper_hpp__
#define __action_wrapper_hpp__

#include <ioa/scheduler.hpp>
#include <ioa/observer.hpp>

namespace ioa {

  template <class C, void (C::*member_function_ptr) ()>
  class uv_up_input_wrapper :
    public input,
    public no_value,
    public no_parameter,
    public observable
  {
  private:
    bool m_bind_status;
    
  public:
    uv_up_input_wrapper () :
      m_bind_status (false)
    { }
    
    void operator() (C& c) {
      (c.*member_function_ptr) ();
    }

    void bound () {
      m_bind_status = true;
      notify_observers ();
    }

    void unbound () {
      m_bind_status = false;
      notify_observers ();
    }

    bool is_bound () const {
      return m_bind_status;
    }

  };

  template <class C, class P, void (C::*member_function_ptr) (P)>
  class uv_p_input_wrapper :
    public input,
    public no_value,
    public parameter<P>,
    public observable
  {
  private:
    std::set<P> m_parameters;
    
  public:
    void operator() (C& c, P p) {
      (c.*member_function_ptr) (p);
    }
    
    void bound (P p) {
      m_parameters.insert (p);
      notify_observers ();
    }

    void unbound (P p) {
      m_parameters.erase (p);
      notify_observers ();
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, class T, void (C::*member_function_ptr) (const T&)>
  class v_up_input_wrapper :
    public input,
    public value<T>,
    public no_parameter,
    public observable
  {
  private:
    bool m_bind_status;
    
  public:
    v_up_input_wrapper () :
      m_bind_status (false)
    { }
    
    void operator() (C& c, const T& t) {
      (c.*member_function_ptr) (t);
    }
    
    void bound () {
      m_bind_status = true;
      notify_observers ();
    }

    void unbound () {
      m_bind_status = false;
      notify_observers ();
    }

    bool is_bound () const {
      return m_bind_status;
    }
  };

  template <class C, class T, class P, void (C::*member_function_ptr)(const T&, P) >
  class v_p_input_wrapper :
    public input,
    public value<T>,
    public parameter<P>,
    public observable
  {
  private:
    std::set<P> m_parameters;
    
  public:
    void operator() (C& c, const T& t, P p) {
      (c.*member_function_ptr) (t, p);
    }
    
    void bound (P p) {
      m_parameters.insert (p);
      notify_observers ();
    }

    void unbound (P p) {
      m_parameters.erase (p);
      notify_observers ();
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, bool (C::*member_function_ptr) ()>
  class uv_up_output_wrapper :
    public output,
    public no_value,
    public no_parameter,
    public observable
  {
  private:
    uv_up_output_wrapper C::*m_member_object_ptr;
    bool m_bind_status;
    
  public:
    uv_up_output_wrapper (uv_up_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr),
      m_bind_status (false)
    { }
    
    bool operator() (C& c) {
      return (c.*member_function_ptr) ();
    }

    void bound () {
      m_bind_status = true;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr);
      notify_observers ();
    }

    void unbound () {
      m_bind_status = false;
      notify_observers ();
    }

    bool is_bound () const {
      return m_bind_status;
    }
  };

  template <class C, class P, bool (C::*member_function_ptr)(P)>
  class uv_p_output_wrapper :
    public output,
    public no_value,
    public parameter<P>,
    public observable
  {
  private:
    uv_p_output_wrapper C::*m_member_object_ptr;
    std::set<P> m_parameters;
    
  public:
    uv_p_output_wrapper (uv_p_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }
    
    bool operator() (C& c, P p) {
      return (c.*member_function_ptr) (p);
    }

    void bound (P p) {
      m_parameters.insert (p);
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr, p);
      notify_observers ();
    }

    void unbound (P p) {
      m_parameters.erase (p);
      notify_observers ();
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, class T, std::pair<bool, T> (C::*member_function_ptr) (void)>
  class v_up_output_wrapper :
    public output,
    public value<T>,
    public no_parameter,
    public observable
  {
  private:
    v_up_output_wrapper C::*m_member_object_ptr;
    bool m_bind_status;
    
  public:
    v_up_output_wrapper (v_up_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr),
      m_bind_status (false)
    { }
    
    std::pair<bool, T> operator() (C& c) {
      return (c.*member_function_ptr) ();
    }
    
    void bound () {
      m_bind_status = true;
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr);
      notify_observers ();
    }
    
    void unbound () {
      m_bind_status = false;
      notify_observers ();
    }

    bool is_bound () const {
      return m_bind_status;
    }
  };

  template <class C, class T, class P, std::pair<bool, T> (C::*member_function_ptr)(P)>
  class v_p_output_wrapper :
    public output,
    public value<T>,
    public parameter<P>,
    public observable
  {
  private:
    v_p_output_wrapper C::*m_member_object_ptr;
    std::set<P> m_parameters;
    
  public:
    v_p_output_wrapper (v_p_output_wrapper C::*member_object_ptr) :
      m_member_object_ptr (member_object_ptr)
    { }
    
    std::pair<bool, T> operator() (C& c, P p) {
      return (c.*member_function_ptr) (p);
    }
    
    void bound (P p) {
      m_parameters.insert (p);
      // We schedule the action because the precondition might test is_bound ().
      scheduler::schedule (m_member_object_ptr, p);
      notify_observers ();
    }
    
    void unbound (P p) {
      m_parameters.erase (p);
      notify_observers ();
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }

  };

  template <class C, void (C::*member_function_ptr) ()>
  struct up_internal_wrapper :
    public internal,
    public no_parameter
  {
    void operator() (C& c) {
      (c.*member_function_ptr) ();
    }
  };

  template <class C, class P, void (C::*member_function_ptr)(P)>
  struct p_internal_wrapper :
    public internal,
    public parameter<P>
  {
    void operator() (C& c, P p) {
      (c.*member_function_ptr) (p);
    }
  };

  // template <class C>
  // class uv_event_wrapper :
  //   public event,
  //   public no_value
  // {
  // private:
  //   C& m_c;
  //   void (C::*m_member_function_ptr)();
    
  // public:
  //   uv_event_wrapper (C& c,
  // 		      void (C::*member_function_ptr)(),
  // 		      uv_event_wrapper C::*member_object_ptr) :
  //     m_c (c),
  //     m_member_function_ptr (member_function_ptr)
  //   { }
    
  //   void operator() (C&) {
  //     (m_c.*m_member_function_ptr) ();
  //   }
  // };

}

#define DECLARE_UV_UP_INPUT(c, name) \
  private: \
  void _##name (); \
  public: \
  typedef ioa::uv_up_input_wrapper<c, &c::_##name> name##_type; \
  name##_type name; \
  private:

#define DEFINE_UV_UP_INPUT(c, name) \
  void c::_##name ()

#define DECLARE_UV_P_INPUT(c, name, param_type, param)	\
  private: \
  void _##name (param_type param); \
  public: \
  typedef ioa::uv_p_input_wrapper<c, param_type, &c::_##name> name##_type; \
  name##_type name; \
  private:

#define DEFINE_UV_P_INPUT(c, name, param_type, param)	\
  void c::_##name (param_type param)

#define DECLARE_V_UP_INPUT(c, name, type, var)			\
  private: \
  void _##name (const type & var); \
  public: \
  typedef ioa::v_up_input_wrapper<c, type, &c::_##name> name##_type;	\
  name##_type name; \
  private:

#define DEFINE_V_UP_INPUT(c, name, type, var)			\
  void c::_##name (const type & var)

#define DECLARE_V_P_INPUT(c, name, type, var, param_type, param)	\
  private: \
  void _##name (const type & var, param_type param); \
  public: \
  typedef ioa::v_p_input_wrapper<c, type, param_type, &c::_##name> name##_type;	\
  name##_type name; \
  private:

#define DEFINE_V_P_INPUT(c, name, type, var, param_type, param)	\
  void c::_##name (const type & var, param_type param)

#define DECLARE_UV_UP_OUTPUT(c, name) \
  private: \
  bool _##name (); \
  public: \
  typedef ioa::uv_up_output_wrapper<c, &c::_##name> name##_type;	\
  name##_type name; \
private:

#define DEFINE_UV_UP_OUTPUT(c, name) \
  bool c::_##name ()

#define DECLARE_UV_P_OUTPUT(c, name, param_type, param)	\
  private: \
  bool _##name (param_type param); \
  public: \
  typedef ioa::uv_p_output_wrapper<c, param_type, &c::_##name> name##_type; \
  name##_type name; \
  private:

#define DEFINE_UV_P_OUTPUT(c, name, param_type, param)	\
  bool c::_##name (param_type param)

#define DECLARE_V_UP_OUTPUT(c, name, type)			\
  private: \
  std::pair<bool, type> _##name (); \
  public: \
  typedef ioa::v_up_output_wrapper<c, type, &c::_##name> name##_type;	\
  name##_type name; \
  private:

#define DEFINE_V_UP_OUTPUT(c, name, type)			\
  std::pair<bool, type> c::_##name ()

#define DECLARE_V_P_OUTPUT(c, name, type, param_type, param)	\
  private: \
  std::pair<bool, type> _##name (param_type param); \
  public: \
  typedef ioa::v_p_output_wrapper<c, type, param_type, &c::_##name> name##_type;	\
  name##_type name; \
  private:

#define DEFINE_V_P_OUTPUT(c, name, type, param_type, param)	\
  std::pair<bool, type> c::_##name (param_type param)

#define DECLARE_UP_INTERNAL(c, name) \
  private: \
  void _##name (); \
  ioa::up_internal_wrapper<c, &c::_##name> name;

#define DEFINE_UP_INTERNAL(c, name) \
  void c::_##name ()

#define DECLARE_P_INTERNAL(c, name, param_type, param)	\
  private: \
  void _##name (param_type param); \
  ioa::p_internal_wrapper<c, param_type, &c::_##name> name;

#define DEFINE_P_INTERNAL(c, name, param_type, param)	\
  void c::_##name (param_type param)

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
