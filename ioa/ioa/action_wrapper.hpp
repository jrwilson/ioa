#ifndef __action_wrapper_hpp__
#define __action_wrapper_hpp__

namespace ioa {

  // TODO:  Improve the wrappers.
  
  template <class C>
  class uv_up_input_wrapper :
    public input,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    void (C::*m_member_ptr)();
    
  public:
    uv_up_input_wrapper (C& c,
			 void (C::*member_ptr) ()) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    void operator() () {
      (m_c.*m_member_ptr) ();
    }

    void bound () {
    }
    void unbound () {
    }
  };

  template <class C, class P>
  class uv_p_input_wrapper :
    public input,
    public no_value,
    public parameter<P>
  {
  private:
    C& m_c;
    void (C::*m_member_ptr) (P);
    
  public:
    uv_p_input_wrapper (C& c,
			void (C::*member_ptr)(P)) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    void operator() (P p) {
      (m_c.*m_member_ptr) (p);
    }
    
    void bound (P) {
    }

    void unbound (P) {
    }
  };

  template <class C, class T>
  class v_up_input_wrapper :
    public input,
    public value<T>,
    public no_parameter
  {
  private:
    C& m_c;
    void (C::*m_member_ptr)(const T&);
    
  public:
    v_up_input_wrapper (C& c,
			void (C::*member_ptr)(const T&))
      : m_c (c),
 	m_member_ptr (member_ptr)
    { }
    
    void operator() (const T& t) {
      (m_c.*m_member_ptr) (t);
    }
    
    void bound () { }
    void unbound () { }
  };

  template <class C, class T, class P>
  class v_p_input_wrapper :
    public input,
    public value<T>,
    public parameter<P>
  {
  private:
    C& m_c;
    void (C::*m_member_ptr)(const T&, P);
    
  public:
    v_p_input_wrapper (C& c,
		       void (C::*member_ptr)(const T&, P))
      : m_c (c),
 	m_member_ptr (member_ptr)
    { }
    
    void operator() (const T& t, P p) {
      (m_c.*m_member_ptr) (t, p);
    }
    
    void bound (P) { }
    void unbound (P) { }
  };

  template <class C>
  class uv_up_output_wrapper :
    public output,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    bool (C::*m_member_ptr)(void);
    
  public:
    uv_up_output_wrapper (C& c,
			  bool (C::*member_ptr) (void)) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    bool operator() () {
      return (m_c.*m_member_ptr) ();
    }

    void bound () {
    }
    void unbound () {
    }
  };

  template <class C, class P>
  class uv_p_output_wrapper :
    public output,
    public no_value,
    public parameter<P>
  {
  private:
    C& m_c;
    bool (C::*m_member_ptr)(P);
    
  public:
    uv_p_output_wrapper (C& c,
				   bool (C::*member_ptr) (P)) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    bool operator() (P p) {
      return (m_c.*m_member_ptr) (p);
    }

    void bound (P) {
    }

    void unbound (P) {
    }
  };

  template <class C, class T>
  class v_up_output_wrapper :
    public output,
    public value<T>,
    public no_parameter
  {
  private:
    C& m_c;
    std::pair<bool, T> (C::*m_member_ptr)(void);
    
  public:
    v_up_output_wrapper (C& c,
			 std::pair<bool, T> (C::*member_ptr)(void)) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    std::pair<bool, T> operator() () {
      return (m_c.*m_member_ptr) ();
    }
    
    void bound () {
    }
    
    void unbound () {
    }
  };

  template <class C, class T, class P>
  class v_p_output_wrapper :
    public output,
    public value<T>,
    public parameter<P>
  {
  private:
    C& m_c;
    std::pair<bool, T> (C::*m_member_ptr)(P);
    
  public:
    v_p_output_wrapper (C& c,
			std::pair<bool, T> (C::*member_ptr)(P)) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    std::pair<bool, T> operator() (P p) {
      return (m_c.*m_member_ptr) (p);
    }
    
    void bound (P) {
    }
    
    void unbound (P) {
    }
  };

  template <class C>
  class up_internal_wrapper :
    public internal,
    public no_parameter
  {
  private:
    C& m_c;
    void (C::*m_member_ptr)();
    
  public:
    up_internal_wrapper (C& c,
			 void (C::*member_ptr)()) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    void operator() () {
      (m_c.*m_member_ptr) ();
    }
  };

  template <class C, class P>
  class p_internal_wrapper :
    public internal,
    public parameter<P>
  {
  private:
    C& m_c;
    void (C::*m_member_ptr)(P);
    
  public:
    p_internal_wrapper (C& c,
			 void (C::*member_ptr)(P)) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    void operator() (P p) {
      (m_c.*m_member_ptr) (p);
    }
  };

  template <class C>
  class uv_event_wrapper :
    public event,
    public no_value
  {
  private:
    C& m_c;
    void (C::*m_member_ptr)();
    
  public:
    uv_event_wrapper (C& c,
		      void (C::*member_ptr)()) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    void operator() () {
      (m_c.*m_member_ptr) ();
    }
  };

  template <class C, class T>
  class v_event_wrapper :
    public event,
    public value<T>
  {
  private:
    C& m_c;
    void (C::*m_member_ptr)(const T&);
    
  public:
    v_event_wrapper (C& c,
		     void (C::*member_ptr)(const T&)) :
      m_c (c),
      m_member_ptr (member_ptr)
    { }
    
    void operator() (const T& t) {
      (m_c.*m_member_ptr) (t);
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

#define ACTION(c, name) name (*this, &c::_##name)

#endif
