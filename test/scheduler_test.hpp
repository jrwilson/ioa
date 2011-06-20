#include "minunit.h"

#include <ioa/scheduler.hpp>
#include <ioa/generator.hpp>
#include "automaton2.hpp"
#include "instance_holder.hpp"
#include <ioa/automaton_helper.hpp>

#include <iostream>
#include <fcntl.h>

static bool goal_reached;

class create_instance_exists :
  public ioa::automaton
{
private:
  automaton2* m_instance;

   struct helper :
    public ioa::system_automaton_helper_interface
  {
    automaton2* m_instance;

    helper (automaton2* instance) :
      m_instance (instance)
    { }

    ioa::shared_ptr<ioa::generator_interface> get_generator () const {
      return ioa::shared_ptr<ioa::generator_interface> (new instance_holder<automaton2> (m_instance));
    }

    void instance_exists () {
      goal_reached = true;
      // Clean up.
      delete this;
    }
    
    void automaton_created (const ioa::aid_t aid) {
      // Okay.
    }

    void automaton_destroyed () {
      // Clean up.
      delete this;
    }
  };
  
public:  
  create_instance_exists () :
    m_instance (new automaton2 ())
  {
    create (new helper (m_instance));
    create (new helper (m_instance));
  }
};

static const char*
instance_exists ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<create_instance_exists> ());
  mu_assert (goal_reached);
  return 0;
}

class create_automaton_created :
  public ioa::automaton
{
private:

  struct helper :
    public ioa::system_automaton_helper_interface
  {
    ioa::shared_ptr<ioa::generator_interface> get_generator () const {
      return ioa::make_generator<automaton2> ();
    }

    void instance_exists () {
      assert (false);
    }

    void automaton_created (const ioa::aid_t aid) {
      goal_reached = true;
    }

    void automaton_destroyed () {
      // Clean up.
      delete this;
    }
  };

public:
  
  create_automaton_created () {
    create (new helper ());
  }
};

static const char*
automaton_created ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<create_automaton_created> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_output_automaton_dne :
  public ioa::automaton
{
private:

  template <class OI, class OM, class IH, class IM>
  struct helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
    typedef typename IH::instance II;

    automaton& m_automaton;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OM OI::*output_member_ptr,
	    IH* input_helper,
	    IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_handle (-1),
      m_output_member_ptr (output_member_ptr),
      m_input_helper (input_helper),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_input_helper->get_handle () != -1) {
       	m_input_handle = m_input_helper->get_handle ();
      }
      else {
	add_observable (m_input_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      goal_reached = true;
      delete this;
    }

    void input_automaton_dne () {
      assert (false);
    }

    void binding_exists () {
      assert (false);
    }

    void output_action_unavailable () {
      assert (false);
    }

    void input_action_unavailable () {
      assert (false);
    }

    void bound () {
      assert (false);
    }

    void unbound () {
      assert (false);
    }

    void observe (ioa::observable*) {
      m_input_handle = m_input_helper->get_handle ();
      m_automaton.bind (this);
    }
  };

public:
  
  bind_output_automaton_dne ()
  {
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    new helper<automaton2, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
output_automaton_dne ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_output_automaton_dne> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_input_automaton_dne :
  public ioa::automaton
{
private:

  template <class OH, class OM, class II, class IM>
  struct helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;

    automaton& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OH* output_helper,
	    OM OI::*output_member_ptr,
	    IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_helper (output_helper),
      m_output_handle (-1),
      m_output_member_ptr (output_member_ptr),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_output_helper->get_handle () != -1) {
       	m_output_handle = m_output_helper->get_handle ();
      }
      else {
	add_observable (m_output_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      assert (false);
    }

    void input_automaton_dne () {
      goal_reached = true;
      delete this;
    }

    void binding_exists () {
      assert (false);
    }

    void output_action_unavailable () {
      assert (false);
    }

    void input_action_unavailable () {
      assert (false);
    }

    void bound () {
      assert (false);
    }

    void unbound () {
      assert (false);
    }

    void observe (ioa::observable*) {
      m_output_handle = m_output_helper->get_handle ();
      m_automaton.bind (this);
    }
  };

public:
  
  bind_input_automaton_dne ()
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, automaton2, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, &automaton2::uv_up_input);
  }
};

static const char*
input_automaton_dne ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_input_automaton_dne> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_binding_exists :
  public ioa::automaton
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OH* output_helper,
	    OM OI::*output_member_ptr,
	    IH* input_helper,
	    IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_helper (output_helper),
      m_output_member_ptr (output_member_ptr),
      m_input_helper (input_helper),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_output_helper->get_handle () != -1) {
       	m_output_handle = m_output_helper->get_handle ();
      }
      else {
	add_observable (m_output_helper);
      }

      if (m_input_helper->get_handle () != -1) {
       	m_input_handle = m_input_helper->get_handle ();
      }
      else {
	add_observable (m_input_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      assert (false);
    }

    void input_automaton_dne () {
      assert (false);
    }

    void binding_exists () {
      goal_reached = true;
      delete this;
    }

    void output_action_unavailable () {
      assert (false);
    }

    void input_action_unavailable () {
      assert (false);
    }

    void bound () {
      // Okay.
    }

    void unbound () {
      // Cleanup.
      delete this;
    }

    void observe (ioa::observable*) {
      m_output_handle = m_output_helper->get_handle ();
      m_input_handle = m_input_helper->get_handle ();
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton.bind (this);
      }
    }
  };

public:
  
  bind_binding_exists ()
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
binding_exists ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_binding_exists> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_input_action_unavailable :
  public ioa::automaton
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OH* output_helper,
	    OM OI::*output_member_ptr,
	    IH* input_helper,
	    IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_helper (output_helper),
      m_output_member_ptr (output_member_ptr),
      m_input_helper (input_helper),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_output_helper->get_handle () != -1) {
       	m_output_handle = m_output_helper->get_handle ();
      }
      else {
	add_observable (m_output_helper);
      }

      if (m_input_helper->get_handle () != -1) {
       	m_input_handle = m_input_helper->get_handle ();
      }
      else {
	add_observable (m_input_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      assert (false);
    }

    void input_automaton_dne () {
      assert (false);
    }

    void binding_exists () {
      assert (false);
    }

    void output_action_unavailable () {
      assert (false);
    }

    void input_action_unavailable () {
      goal_reached = true;
      delete this;
    }

    void bound () {
      // Okay.
    }

    void unbound () {
      // Cleanup.
      delete this;
    }

    void observe (ioa::observable*) {
      m_output_handle = m_output_helper->get_handle ();
      m_input_handle = m_input_helper->get_handle ();
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton.bind (this);
      }
    }
  };

public:
  
  bind_input_action_unavailable ()
  {
    ioa::automaton_helper<automaton2>* m_output1 = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_output2 = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output1, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output2, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
input_action_unavailable ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_input_action_unavailable> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_output_action_unavailable :
  public ioa::automaton
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OH* output_helper,
	    OM OI::*output_member_ptr,
	    IH* input_helper,
	    IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_helper (output_helper),
      m_output_member_ptr (output_member_ptr),
      m_input_helper (input_helper),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_output_helper->get_handle () != -1) {
       	m_output_handle = m_output_helper->get_handle ();
      }
      else {
	add_observable (m_output_helper);
      }

      if (m_input_helper->get_handle () != -1) {
       	m_input_handle = m_input_helper->get_handle ();
      }
      else {
	add_observable (m_input_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      assert (false);
    }

    void input_automaton_dne () {
      assert (false);
    }

    void binding_exists () {
      assert (false);
    }

    void output_action_unavailable () {
      goal_reached = true;
      delete this;
    }

    void input_action_unavailable () {
      assert (false);
    }

    void bound () {
      // Okay.
    }

    void unbound () {
      // Cleanup.
      delete this;
    }

    void observe (ioa::observable*) {
      m_output_handle = m_output_helper->get_handle ();
      m_input_handle = m_input_helper->get_handle ();
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton.bind (this);
      }
    }
  };

public:
  
  bind_output_action_unavailable ()
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input2_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input2);
  }
};

static const char*
output_action_unavailable ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_output_action_unavailable> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_bound :
  public ioa::automaton
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OH* output_helper,
	    OM OI::*output_member_ptr,
	    IH* input_helper,
	    IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_helper (output_helper),
      m_output_member_ptr (output_member_ptr),
      m_input_helper (input_helper),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_output_helper->get_handle () != -1) {
       	m_output_handle = m_output_helper->get_handle ();
      }
      else {
	add_observable (m_output_helper);
      }

      if (m_input_helper->get_handle () != -1) {
       	m_input_handle = m_input_helper->get_handle ();
      }
      else {
	add_observable (m_input_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      assert (false);
    }

    void input_automaton_dne () {
      assert (false);
    }

    void binding_exists () {
      assert (false);
    }

    void output_action_unavailable () {
      assert (false);
    }

    void input_action_unavailable () {
      assert (false);
    }

    void bound () {
      goal_reached = true;
    }

    void unbound () {
      // Cleanup.
      delete this;
    }

    void observe (ioa::observable*) {
      m_output_handle = m_output_helper->get_handle ();
      m_input_handle = m_input_helper->get_handle ();
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton.bind (this);
      }
    }
  };

public:
  
  bind_bound ()
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
bound ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_bound> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_unbound :
  public ioa::automaton
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    bool is_bound;
    automaton& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OH* output_helper,
	    OM OI::*output_member_ptr,
	    IH* input_helper,
	    IM II::*input_member_ptr) :
      is_bound (false),
      m_automaton (automaton),
      m_output_helper (output_helper),
      m_output_member_ptr (output_member_ptr),
      m_input_helper (input_helper),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_output_helper->get_handle () != -1) {
       	m_output_handle = m_output_helper->get_handle ();
      }
      else {
	add_observable (m_output_helper);
      }

      if (m_input_helper->get_handle () != -1) {
       	m_input_handle = m_input_helper->get_handle ();
      }
      else {
	add_observable (m_input_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      assert (false);
    }

    void input_automaton_dne () {
      assert (false);
    }

    void binding_exists () {
      assert (false);
    }

    void output_action_unavailable () {
      assert (false);
    }

    void input_action_unavailable () {
      assert (false);
    }

    void bound () {
      is_bound = true;
    }

    void unbound () {
      // Cleanup.
      goal_reached = true;
      delete this;
    }

    void observe (ioa::observable*) {
      m_output_handle = m_output_helper->get_handle ();
      m_input_handle = m_input_helper->get_handle ();
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton.bind (this);
      }
    }
  };

  helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type>* m_helper;

  bool poll_precondition () const {
    return true;
  }

  void poll_action () {
    // We poll until the automaton is created.  Then we destroy it.
    if (!m_helper->is_bound) {
      ioa::schedule (&bind_unbound::poll);
    }
    else {
      unbind (m_helper);
    }
  }

  UP_INTERNAL (bind_unbound, poll);

public:
  
  bind_unbound ()
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    ioa::schedule (&bind_unbound::poll);
  }
};

static const char*
unbound ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_unbound> ());
  mu_assert (goal_reached);
  return 0;
}

class bind_unbound2 :
  public ioa::automaton
{
private:

  template <class OH, class OM, class IH, class IM>
  class helper :
    public ioa::system_bind_helper_interface,
    public ioa::observer
  {
  public:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton& automaton,
	    OH* output_helper,
	    OM OI::*output_member_ptr,
	    IH* input_helper,
	    IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_helper (output_helper),
      m_output_member_ptr (output_member_ptr),
      m_input_helper (input_helper),
      m_input_member_ptr (input_member_ptr)
    {
      if (m_output_helper->get_handle () != -1) {
       	m_output_handle = m_output_helper->get_handle ();
      }
      else {
	add_observable (m_output_helper);
      }

      if (m_input_helper->get_handle () != -1) {
       	m_input_handle = m_input_helper->get_handle ();
      }
      else {
	add_observable (m_input_helper);
      }
    }

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (m_output_handle, m_output_member_ptr,
				      m_input_handle, m_input_member_ptr);
    }

    void output_automaton_dne () {
      assert (false);
    }

    void input_automaton_dne () {
      assert (false);
    }

    void binding_exists () {
      assert (false);
    }

    void output_action_unavailable () {
      assert (false);
    }

    void input_action_unavailable () {
      assert (false);
    }

    void bound () {
      assert (false);
    }

    void unbound () {
      // Cleanup.
      goal_reached = true;
      delete this;
    }

    void observe (ioa::observable*) {
      m_output_handle = m_output_helper->get_handle ();
      m_input_handle = m_input_helper->get_handle ();
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton.bind (this);
	m_automaton.unbind (this);
      }
    }
  };

public:
  
  bind_unbound2 ()
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
unbound2 ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<bind_unbound2> ());
  mu_assert (goal_reached);
  return 0;
}

class destroy_automaton_destroyed :
  public ioa::automaton
{
private:

  struct helper :
    public ioa::system_automaton_helper_interface
  {
    bool created;

    helper () :
      created (false)
    { }

    ioa::shared_ptr<ioa::generator_interface> get_generator () const {
      return ioa::make_generator<automaton2> ();
    }

    void instance_exists () {
      assert (false);
    }

    void automaton_created (const ioa::aid_t aid) {
      created = true;
    }

    void automaton_destroyed () {
      goal_reached = true;
      // Clean up.
      delete this;
    }
  };

  helper* m_helper;

  bool poll_precondition () const {
    return true;
  }

  void poll_action () {
    // We poll until the automaton is created.  Then we destroy it.
    if (!m_helper->created) {
      ioa::schedule (&destroy_automaton_destroyed::poll);
    }
    else {
      destroy (m_helper);
    }
  }

  UP_INTERNAL (destroy_automaton_destroyed, poll);

public:  
  destroy_automaton_destroyed () :
    m_helper (new helper ())
  {
    create (m_helper);
    ioa::schedule (&destroy_automaton_destroyed::poll);
  }
};

static const char*
automaton_destroyed ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<destroy_automaton_destroyed> ());
  mu_assert (goal_reached);
  return 0;
}

class destroy_automaton_destroyed2 :
  public ioa::automaton
{
private:

  struct helper :
    public ioa::system_automaton_helper_interface
  {
    ioa::shared_ptr<ioa::generator_interface> get_generator () const {
      return ioa::make_generator<automaton2> ();
    }

    void instance_exists () {
      assert (false);
    }

    void automaton_created (const ioa::aid_t aid) {
      assert (false);
    }

    void automaton_destroyed () {
      goal_reached = true;
      // Clean up.
      delete this;
    }
  };

public:  
  destroy_automaton_destroyed2 ()
  {
    helper* h = new helper ();
    // Create and destroy a helper before it is created.
    create (h);
    destroy (h);
  }
};

static const char*
automaton_destroyed2 ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<destroy_automaton_destroyed2> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_automaton :
  public ioa::automaton {
private:

  bool action_precondition () const {
    return true;
  }

  void action_action () {
    goal_reached = true;
  }

  UV_UP_OUTPUT (schedule_automaton, action);

public:
  schedule_automaton ()
  {
    ioa::schedule (&schedule_automaton::action);
  }
  
};

static const char*
schedule ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_automatonp :
  public ioa::automaton {
private:

  bool action_precondition (int param) const {
    assert (param == 18887235);
    return true;
  }

  void action_action (int param) {
    assert (param == 18887235);
    goal_reached = true;
  }

  UV_P_OUTPUT (schedule_automatonp, action, int);

public:
  schedule_automatonp ()
  {
    ioa::schedule (&schedule_automatonp::action, 18887235);
  }
  
};

static const char*
schedulep ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_automatonp> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_after_automaton :
  public ioa::automaton {
private:

  time_t m_schedule_time;

  bool action_precondition () const {
    return true;
  }

  int action_action () {
    time_t execute_time = time (0);
    // Should be within three seconds of each other.
    assert (execute_time - m_schedule_time <= 3);
    goal_reached = true;
    return 0;
  }

  V_UP_OUTPUT (schedule_after_automaton, action, int);

public:
  schedule_after_automaton ()
  {
    m_schedule_time = time (0);
    ioa::schedule_after (&schedule_after_automaton::action, ioa::time (1, 0));
  }
  
};

static const char*
schedule_after ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_after_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_afterp_automaton :
  public ioa::automaton {
private:

  time_t m_schedule_time;

  bool action_precondition (int param) const {
    assert (param == 512);
    return true;
  }

  int action_action (int param) {
    assert (param == 512);
    time_t execute_time = time (0);
    // Should be within three seconds of each other.
    assert (execute_time - m_schedule_time <= 3);
    goal_reached = true;
    return 0;
  }

  V_P_OUTPUT (schedule_afterp_automaton, action, int, int);

public:
  schedule_afterp_automaton ()
  {
    m_schedule_time = time (0);
    ioa::schedule_after (&schedule_afterp_automaton::action, 512, ioa::time (1, 0));
  }
  
};

static const char*
schedule_afterp ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_afterp_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_read_ready_automaton :
  public ioa::automaton {
private:
  int m_fd[2];

  bool action_precondition () const {
    return true;
  }

  void action_action () {
    goal_reached = true;
  }

  UV_UP_OUTPUT (schedule_read_ready_automaton, action);

public:
  schedule_read_ready_automaton ()
  {
    int res = pipe (m_fd);
    if (res == -1) {
      perror ("pipe");
      exit (EXIT_FAILURE);
    }
    assert (fcntl (m_fd[0], F_SETFL, O_NONBLOCK) == 0);
    ioa::schedule_read_ready (&schedule_read_ready_automaton::action, m_fd[0]);
    char c = 0;
    write (m_fd[1], &c, 1);
  }
  
  ~schedule_read_ready_automaton () {
    close (m_fd[0]);
    close (m_fd[1]);
  }

};

static const char*
schedule_read_ready ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_read_ready_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_read_readyp_automaton :
  public ioa::automaton {
private:
  int m_fd[2];

  bool action_precondition (int fd) const {
    assert (fd == m_fd[0]);
    return true;
  }

  void action_action (int fd) {
    assert (fd == m_fd[0]);
    goal_reached = true;
  }

  UV_P_OUTPUT (schedule_read_readyp_automaton, action, int);

public:
  schedule_read_readyp_automaton ()
  {
    int res = pipe (m_fd);
    if (res == -1) {
      perror ("pipe");
      exit (EXIT_FAILURE);
    }
    assert (fcntl (m_fd[0], F_SETFL, O_NONBLOCK) == 0);
    assert (fcntl (m_fd[0], F_SETFL, O_NONBLOCK) == 0);
    ioa::schedule_read_ready (&schedule_read_readyp_automaton::action, m_fd[0], m_fd[0]);
    char c = 0;
    write (m_fd[1], &c, 1);
  }

  ~schedule_read_readyp_automaton () {
    close (m_fd[0]);
    close (m_fd[1]);
  }
  
};

static const char*
schedule_read_readyp ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_read_readyp_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_write_ready_automaton :
  public ioa::automaton {
private:
  int m_fd[2];

  bool action_precondition () const {
    return true;
  }

  void action_action () {
    goal_reached = true;
  }

  UV_UP_OUTPUT (schedule_write_ready_automaton, action);

public:
  schedule_write_ready_automaton ()
  {
    int res = pipe (m_fd);
    if (res == -1) {
      perror ("pipe");
      exit (EXIT_FAILURE);
    }
    assert (fcntl (m_fd[1], F_SETFL, O_NONBLOCK) == 0);
    ioa::schedule_write_ready (&schedule_write_ready_automaton::action, m_fd[1]);
  }
  
};

static const char*
schedule_write_ready ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_write_ready_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class schedule_write_readyp_automaton :
  public ioa::automaton {
private:
  int m_fd[2];

  bool action_precondition (int fd) const {
    assert (fd == m_fd[1]);
    return true;
  }

  void action_action (int fd) {
    assert (fd == m_fd[1]);
    goal_reached = true;
  }

  UV_P_OUTPUT (schedule_write_readyp_automaton, action, int);

public:
  schedule_write_readyp_automaton ()
  {
    int res = pipe (m_fd);
    if (res == -1) {
      perror ("pipe");
      exit (EXIT_FAILURE);
    }
    assert (fcntl (m_fd[1], F_SETFL, O_NONBLOCK) == 0);
    assert (fcntl (m_fd[1], F_SETFL, O_NONBLOCK) == 0);
    ioa::schedule_write_ready (&schedule_write_readyp_automaton::action, m_fd[1], m_fd[1]);
  }
  
};

static const char*
schedule_write_readyp ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  SCHEDULER_TYPE ss;
  ioa::run (ss, ioa::make_generator<schedule_write_readyp_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

const char*
all_tests ()
{
  mu_run_test (instance_exists);
  mu_run_test (automaton_created);
  mu_run_test (output_automaton_dne);
  mu_run_test (input_automaton_dne);
  mu_run_test (binding_exists);
  mu_run_test (input_action_unavailable);
  mu_run_test (output_action_unavailable);
  mu_run_test (bound);
  mu_run_test (unbound);
  mu_run_test (unbound2);
  mu_run_test (automaton_destroyed);
  mu_run_test (automaton_destroyed2);
  mu_run_test (schedule);
  mu_run_test (schedulep);
  mu_run_test (schedule_after);
  mu_run_test (schedule_afterp);
  mu_run_test (schedule_read_ready);
  mu_run_test (schedule_read_readyp);
  mu_run_test (schedule_write_ready);
  mu_run_test (schedule_write_readyp);

  return 0;
}