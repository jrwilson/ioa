#ifndef __automaton_interface_hpp__
#define __automaton_interface_hpp__

#include <ioa/shared_ptr.hpp>
#include <cassert>
#include <ioa/action.hpp>
#include <set>
#include <ioa/generator_interface.hpp>
#include <ioa/executor_interface.hpp>

#define COMMA ,

#define SYSTEM_INPUT(c, name, type, var)		\
  void _##name (type const & var); \
  public: \
  typedef ioa::system_input_wrapper<c, type, &c::_##name> name##_type;	\
  name##_type name;

#define SYSTEM_INPUT_DEF(c, name, type, var)	\
  void c::_##name (type const & var)

#define SYSTEM_OUTPUT(c, name, type)			\
  std::pair<bool, type> _##name (); \
  public: \
  typedef ioa::system_output_wrapper<c, type, &c::_##name> name##_type;	\
  name##_type name; \

#define SYSTEM_OUTPUT_DEF(c, name, type)			\
  std::pair<bool, type> c::_##name ()

#define ACTION(c, name) name (*this, &c::_##name, &c::name)

namespace ioa {

  template <class C, class T, void (C::*mem_fun_ptr)(T const &)>
  class system_input_wrapper :
    public system_input,
    public value<T>
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)(T const &);
    
  public:
    system_input_wrapper (C& c,
			  void (C::*member_function_ptr)(T const &),
			  system_input_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }
    
    void operator() (C&, T const & t) {
      (m_c.*m_member_function_ptr) (t);
    }
  };

  template <class C, class T, std::pair<bool, T> (C::*mem_fun_ptr)()>
  class system_output_wrapper :
    public system_output,
    public value<T>
  {
  private:
    C& m_c;
    std::pair<bool, T> (C::*m_member_function_ptr)(void);
    
  public:
    system_output_wrapper (C& c,
			   std::pair<bool, T> (C::*member_function_ptr)(void),
			   system_output_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }
    
    std::pair<bool, T> operator() (C&) {
      return (m_c.*m_member_function_ptr) ();
    }
  };

  class automaton_helper_interface
  {
  public:
    virtual ~automaton_helper_interface () { }
    virtual shared_ptr<generator_interface> get_generator () const = 0;
    virtual void instance_exists () = 0;
    virtual void automaton_created (const aid_t aid) = 0;
    virtual void automaton_destroyed () = 0;
  };

  class bind_helper_interface
  {
  public:
    virtual ~bind_helper_interface () { }
    virtual shared_ptr<bind_executor_interface> get_executor () const = 0;
    virtual void output_automaton_dne () = 0;
    virtual void input_automaton_dne () = 0;
    virtual void binding_exists () = 0;
    virtual void output_action_unavailable () = 0;
    virtual void input_action_unavailable () = 0;
    virtual void bound () = 0;
    virtual void unbound () = 0;
  };

  class automaton_interface {
  private:
    // Invariant:  m_create_send INTERSECT m_create_recv INTERSECT m_destroy_send = empty set
    // Basically, a helper is in one place or another.
    // The send sets are waiting to send the command.
    // The receive set is waiting for a response.
    std::set<automaton_helper_interface*> m_create_send;
    std::set<automaton_helper_interface*> m_create_recv;
    std::set<automaton_helper_interface*> m_destroy_send;
    std::set<automaton_helper_interface*> m_destroy_recv;
    // Same thing for binding.
    std::set<bind_helper_interface*> m_bind_send;
    std::set<bind_helper_interface*> m_bind_recv;
    std::set<bind_helper_interface*> m_unbind_send;
    std::set<bind_helper_interface*> m_unbind_recv;

  public:
    automaton_interface ();
    virtual ~automaton_interface ();
    void create (automaton_helper_interface* helper);
    void bind (bind_helper_interface* helper);
    void unbind (bind_helper_interface* helper);
    void destroy (automaton_helper_interface* helper);

  private:
    void schedule ();

    bool sys_create_precondition () const;
    SYSTEM_OUTPUT (automaton_interface, sys_create, std::pair<shared_ptr<generator_interface> COMMA void*>);
    bool sys_bind_precondition () const;
    SYSTEM_OUTPUT (automaton_interface, sys_bind, std::pair<shared_ptr<bind_executor_interface> COMMA void*>);
    bool sys_unbind_precondition () const;
    SYSTEM_OUTPUT (automaton_interface, sys_unbind, void*);
    bool sys_destroy_precondition () const;
    SYSTEM_OUTPUT (automaton_interface, sys_destroy, void*);

    SYSTEM_INPUT (automaton_interface, sys_create_key_exists, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_instance_exists, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_automaton_created, std::pair<void* COMMA aid_t>, t);
    SYSTEM_INPUT (automaton_interface, sys_bind_key_exists, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_output_automaton_dne, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_input_automaton_dne, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_binding_exists, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_output_action_unavailable, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_input_action_unavailable, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_bound, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_bind_key_dne, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_unbound, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_create_key_dne, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_automaton_destroyed, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_recipient_dne, void*, t);
    SYSTEM_INPUT (automaton_interface, sys_event_delivered, void*, t);
  };

}

#endif
