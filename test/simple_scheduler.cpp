#include "minunit.h"

#include <ioa/simple_scheduler.hpp>
#include <ioa/generator.hpp>
#include "automaton2.hpp"
#include "instance_holder.hpp"
#include <ioa/automaton_helper.hpp>

static bool goal_reached;

class create_instance_exists :
  public ioa::automaton_interface
{
private:
  automaton2* m_instance;

   struct helper :
    public ioa::automaton_helper_interface
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
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<create_instance_exists> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class create_automaton_created :
  public ioa::automaton_interface
{
private:

  struct helper :
    public ioa::automaton_helper_interface
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
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<create_automaton_created> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_output_automaton_dne :
  public ioa::automaton_interface
{
private:

  template <class OI, class OM, class IH, class IM>
  struct helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
    typedef typename IH::instance II;

    automaton_interface& m_automaton;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
      m_input_handle = m_input_helper->get_handle ();
      m_automaton.bind (this);
    }
  };

public:
  
  bind_output_automaton_dne ()
  {
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    new helper<automaton2, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
output_automaton_dne ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_output_automaton_dne> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_input_automaton_dne :
  public ioa::automaton_interface
{
private:

  template <class OH, class OM, class II, class IM>
  struct helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;

    automaton_interface& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
      m_output_handle = m_output_helper->get_handle ();
      m_automaton.bind (this);
    }
  };

public:
  
  bind_input_automaton_dne ()
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, automaton2, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, &automaton2::uv_up_input);
  }
};

static const char*
input_automaton_dne ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_input_automaton_dne> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_binding_exists :
  public ioa::automaton_interface
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton_interface& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
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
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
binding_exists ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_binding_exists> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_input_action_unavailable :
  public ioa::automaton_interface
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton_interface& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
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
    ioa::automaton_helper<automaton2>* m_output1 = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_output2 = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output1, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output2, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
input_action_unavailable ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_input_action_unavailable> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_output_action_unavailable :
  public ioa::automaton_interface
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton_interface& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
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
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input2_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input2);
  }
};

static const char*
output_action_unavailable ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_output_action_unavailable> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_bound :
  public ioa::automaton_interface
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton_interface& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
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
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
bound ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_bound> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_unbound :
  public ioa::automaton_interface
{
private:

  template <class OH, class OM, class IH, class IM>
  struct helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    bool is_bound;
    automaton_interface& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
      m_output_handle = m_output_helper->get_handle ();
      m_input_handle = m_input_helper->get_handle ();
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton.bind (this);
      }
    }
  };

  helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type>* m_helper;

  UP_INTERNAL (bind_unbound, poll) {
    // We poll until the automaton is created.  Then we destroy it.
    if (!m_helper->is_bound) {
      ioa::scheduler::schedule (&bind_unbound::poll);
    }
    else {
      unbind (m_helper);
    }
  }

public:
  
  bind_unbound () :
    ACTION (bind_unbound, poll)
  {
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    m_helper = new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
    ioa::scheduler::schedule (&bind_unbound::poll);
  }
};

static const char*
unbound ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_unbound> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class bind_unbound2 :
  public ioa::automaton_interface
{
private:

  template <class OH, class OM, class IH, class IM>
  class helper :
    public ioa::bind_helper_interface,
    public ioa::observer
  {
  public:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    automaton_interface& m_automaton;
    OH* m_output_helper;
    ioa::automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    IH* m_input_helper;
    ioa::automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;
    
    helper (automaton_interface& automaton,
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
      return ioa::make_bind_executor (ioa::make_action (m_output_handle, m_output_member_ptr),
				      ioa::make_action (m_input_handle, m_input_member_ptr));
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

    void observe () {
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
    ioa::automaton_helper<automaton2>* m_output = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* m_input = new ioa::automaton_helper<automaton2> (*this, ioa::make_generator<automaton2> ());
    new helper<ioa::automaton_helper<automaton2>, automaton2::uv_up_output_type, ioa::automaton_helper<automaton2>, automaton2::uv_up_input_type> (*this, m_output, &automaton2::uv_up_output, m_input, &automaton2::uv_up_input);
  }
};

static const char*
unbound2 ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<bind_unbound2> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class destroy_automaton_destroyed :
  public ioa::automaton_interface
{
private:

  struct helper :
    public ioa::automaton_helper_interface
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

  UP_INTERNAL (destroy_automaton_destroyed, poll) {
    // We poll until the automaton is created.  Then we destroy it.
    if (!m_helper->created) {
      ioa::scheduler::schedule (&destroy_automaton_destroyed::poll);
    }
    else {
      destroy (m_helper);
    }
  }

public:  
  destroy_automaton_destroyed () :
    m_helper (new helper ()),
    ACTION (destroy_automaton_destroyed, poll)
  {
    create (m_helper);
    ioa::scheduler::schedule (&destroy_automaton_destroyed::poll);
  }
};

static const char*
automaton_destroyed ()
{
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<destroy_automaton_destroyed> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
  return 0;
}

class destroy_automaton_destroyed2 :
  public ioa::automaton_interface
{
private:

  struct helper :
    public ioa::automaton_helper_interface
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
  ioa::scheduler::clear ();
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<destroy_automaton_destroyed2> ());
  mu_assert (goal_reached);
  ioa::scheduler::clear ();
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

  return 0;
}
