#ifndef __action_wrapper_hpp__
#define __action_wrapper_hpp__

#include "scheduler.hpp"

namespace ioa {

  struct unparameterized_bind_callback
  {
    void bound () { }
    void unbound () { }
  };

  template <class P>
  struct parameterized_bind_callback
  {
    void bound (P p) { }
    void unbound (P p) { }
  };

  template <class C, class B = unparameterized_bind_callback>
  class uv_up_input_wrapper :
    public input,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)();
    bool m_bind_status;
    B m_b;
    
  public:
    uv_up_input_wrapper (C& c,
			 void (C::*member_function_ptr) (),
			 uv_up_input_wrapper C::*member_object_ptr,
			 B b = B ()) :
      m_c (c),
      m_member_function_ptr (member_function_ptr),
      m_bind_status (false),
      m_b (b)
    { }
    
    void operator() () {
      (m_c.*m_member_function_ptr) ();
    }

    void bound () {
      m_bind_status = true;
      m_b.bound ();
    }

    void unbound () {
      m_bind_status = false;
      m_b.unbound ();
    }

    bool is_bound () const {
      return m_bind_status;
    }
  };

  template <class C, class P, class B = parameterized_bind_callback<P> >
  class uv_p_input_wrapper :
    public input,
    public no_value,
    public parameter<P>
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr) (P);
    std::set<P> m_parameters;
    B m_b;
    
  public:
    uv_p_input_wrapper (C& c,
			void (C::*member_function_ptr)(P),
			uv_p_input_wrapper C::*member_object_ptr,
			B b = B ()) :
      m_c (c),
      m_member_function_ptr (member_function_ptr),
      m_b (b)
    { }
    
    void operator() (P p) {
      (m_c.*m_member_function_ptr) (p);
    }
    
    void bound (P p) {
      m_parameters.insert (p);
      m_b.bound (p);
    }

    void unbound (P p) {
      m_parameters.erase (p);
      m_b.unbound (p);
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, class T, class B = unparameterized_bind_callback>
  class v_up_input_wrapper :
    public input,
    public value<T>,
    public no_parameter
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)(const T&);
    bool m_bind_status;
    B m_b;
    
  public:
    v_up_input_wrapper (C& c,
			void (C::*member_function_ptr)(const T&),
			v_up_input_wrapper C::*member_object_ptr,
			B b = B ())
      : m_c (c),
 	m_member_function_ptr (member_function_ptr),
	m_bind_status (false),
	m_b (b)
    { }
    
    void operator() (const T& t) {
      (m_c.*m_member_function_ptr) (t);
    }
    
    void bound () {
      m_bind_status = true;
      m_b.bound ();
    }

    void unbound () {
      m_bind_status = false;
      m_b.unbound ();
    }

    bool is_bound () const {
      return m_bind_status;
    }
  };

  template <class C, class T, class P, class B = parameterized_bind_callback<P> >
  class v_p_input_wrapper :
    public input,
    public value<T>,
    public parameter<P>
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)(const T&, P);
    std::set<P> m_parameters;
    B m_b;
    
  public:
    v_p_input_wrapper (C& c,
		       void (C::*member_function_ptr)(const T&, P),
		       v_p_input_wrapper C::*member_object_ptr,
		       B b = B ())
      : m_c (c),
 	m_member_function_ptr (member_function_ptr),
	m_b (b)
    { }
    
    void operator() (const T& t, P p) {
      (m_c.*m_member_function_ptr) (t, p);
    }
    
    void bound (P p) {
      m_parameters.insert (p);
      m_b.bound (p);
    }

    void unbound (P p) {
      m_parameters.erase (p);
      m_b.unbound (p);
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, class B = unparameterized_bind_callback>
  class uv_up_output_wrapper :
    public output,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    bool (C::*m_member_function_ptr)(void);
    uv_up_output_wrapper C::*m_member_object_ptr;
    bool m_bind_status;
    B m_b;
    
  public:
    uv_up_output_wrapper (C& c,
			  bool (C::*member_function_ptr) (void),
			  uv_up_output_wrapper C::*member_object_ptr,
			  B b = B ()) :
      m_c (c),
      m_member_function_ptr (member_function_ptr),
      m_member_object_ptr (member_object_ptr),
      m_bind_status (false),
      m_b (b)
    { }
    
    bool operator() () {
      return (m_c.*m_member_function_ptr) ();
    }

    void bound () {
      m_bind_status = true;
      // We schedule the action because the precondition might test is_bound ().
      scheduler.schedule (&m_c, m_member_object_ptr);
      m_b.bound ();
    }

    void unbound () {
      m_bind_status = false;
      m_b.unbound ();
    }

    bool is_bound () const {
      return m_bind_status;
    }
  };

  template <class C, class P, class B = parameterized_bind_callback<P> >
  class uv_p_output_wrapper :
    public output,
    public no_value,
    public parameter<P>
  {
  private:
    C& m_c;
    bool (C::*m_member_function_ptr)(P);
    uv_p_output_wrapper C::*m_member_object_ptr;
    std::set<P> m_parameters;
    B m_b;
    
  public:
    uv_p_output_wrapper (C& c,
			 bool (C::*member_function_ptr) (P),
			 uv_p_output_wrapper C::*member_object_ptr,
			 B b = B()) :
      m_c (c),
      m_member_function_ptr (member_function_ptr),
      m_member_object_ptr (member_object_ptr),
      m_b (b)
    { }
    
    bool operator() (P p) {
      return (m_c.*m_member_function_ptr) (p);
    }

    void bound (P p) {
      m_parameters.insert (p);
      // We schedule the action because the precondition might test is_bound ().
      scheduler.schedule (&m_c, m_member_object_ptr, p);
      m_b.bound (p);
    }

    void unbound (P p) {
      m_parameters.erase (p);
      m_b.unbound (p);
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }
  };

  template <class C, class T, class B = unparameterized_bind_callback>
  class v_up_output_wrapper :
    public output,
    public value<T>,
    public no_parameter
  {
  private:
    C& m_c;
    std::pair<bool, T> (C::*m_member_function_ptr)(void);
    v_up_output_wrapper C::*m_member_object_ptr;
    bool m_bind_status;
    B m_b;
    
  public:
    v_up_output_wrapper (C& c,
			 std::pair<bool, T> (C::*member_function_ptr)(void),
			 v_up_output_wrapper C::*member_object_ptr,
			 B b = B ()) :
      m_c (c),
      m_member_function_ptr (member_function_ptr),
      m_member_object_ptr (member_object_ptr),
      m_bind_status (false),
      m_b (b)
    { }
    
    std::pair<bool, T> operator() () {
      return (m_c.*m_member_function_ptr) ();
    }
    
    void bound () {
      m_bind_status = true;
      // We schedule the action because the precondition might test is_bound ().
      scheduler.schedule (&m_c, m_member_object_ptr);
      m_b.bound ();
    }
    
    void unbound () {
      m_bind_status = false;
      m_b.unbound ();
    }

    bool is_bound () const {
      return m_bind_status;
    }
  };

  template <class C, class T, class P, class B = parameterized_bind_callback<P> >
  class v_p_output_wrapper :
    public output,
    public value<T>,
    public parameter<P>
  {
  private:
    C& m_c;
    std::pair<bool, T> (C::*m_member_function_ptr)(P);
    v_p_output_wrapper C::*m_member_object_ptr;
    std::set<P> m_parameters;
    B m_b;
    
  public:
    v_p_output_wrapper (C& c,
			std::pair<bool, T> (C::*member_function_ptr)(P),
			v_p_output_wrapper C::*member_object_ptr,
			B b = B()) :
      m_c (c),
      m_member_function_ptr (member_function_ptr),
      m_member_object_ptr (member_object_ptr),
      m_b (b)
    { }
    
    std::pair<bool, T> operator() (P p) {
      return (m_c.*m_member_function_ptr) (p);
    }
    
    void bound (P p) {
      m_parameters.insert (p);
      // We schedule the action because the precondition might test is_bound ().
      scheduler.schedule (&m_c, m_member_object_ptr, p);
      m_b.bound (p);
    }
    
    void unbound (P p) {
      m_parameters.erase (p);
      m_b.unbound (p);
    }

    void is_bound (P p) const {
      return m_parameters.count (p) != 0;
    }

  };

  template <class C>
  class up_internal_wrapper :
    public internal,
    public no_parameter
  {
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

  template <class C, class P>
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

  template <class C, class T>
  class v_event_wrapper :
    public event,
    public value<T>
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)(const T&);
    
  public:
    v_event_wrapper (C& c,
		     void (C::*member_function_ptr)(const T&),
		     v_event_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }
    
    void operator() (const T& t) {
      (m_c.*m_member_function_ptr) (t);
    }
  };

}

#define UV_UP_INPUT(c, name) \
  public: \
  typedef ioa::uv_up_input_wrapper<c> name##_type; \
  name##_type name; \
  private:	 \
  void _##name ()

#define UV_P_INPUT(c, name, param_type, param)	\
  public: \
  typedef ioa::uv_p_input_wrapper<c, param_type> name##_type; \
  name##_type name; \
  private:	 \
  void _##name (param_type param)

#define V_UP_INPUT(c, name, type, var)			\
  public: \
  typedef ioa::v_up_input_wrapper<c, type> name##_type;	\
  name##_type name; \
  private: \
 void _##name (const type & var)

#define V_P_INPUT(c, name, type, var, param_type, param)	\
  public: \
  typedef ioa::v_p_input_wrapper<c, type, param_type> name##_type;	\
  name##_type name; \
  private: \
  void _##name (const type & var, param_type param)

#define UV_UP_OUTPUT(c, name) \
  public: \
  typedef ioa::uv_up_output_wrapper<c> name##_type; \
  name##_type name; \
  private:	 \
  bool _##name ()

#define UV_P_OUTPUT(c, name, param_type, param)	\
  public: \
  typedef ioa::uv_p_output_wrapper<c, param_type> name##_type; \
  name##_type name; \
  private:	 \
  bool _##name (param_type param)

#define V_UP_OUTPUT(c, name, type)			\
  public: \
  typedef ioa::v_up_output_wrapper<c, type> name##_type;	\
  name##_type name; \
  private: \
  std::pair<bool, type> _##name ()

#define V_P_OUTPUT(c, name, type, param_type, param)	\
  public: \
  typedef ioa::v_p_output_wrapper<c, type, param_type> name##_type;	\
  name##_type name; \
  private: \
  std::pair<bool, type> _##name (param_type param)

#define UP_INTERNAL(c, name) \
  ioa::up_internal_wrapper<c> name; \
  void _##name ()

#define P_INTERNAL(c, name, param_type, param)	\
  ioa::p_internal_wrapper<c, param_type> name; \
  void _##name (param_type param)

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

#define ACTION(c, name) name (*this, &c::_##name, &c::name)

#endif
