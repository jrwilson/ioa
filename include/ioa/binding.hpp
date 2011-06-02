#ifndef __binding_hpp__
#define __binding_hpp__

#include <ioa/action.hpp>
#include <ioa/bid.hpp>
#include <ioa/system_scheduler.hpp>
#include <ioa/automaton_locker.hpp>
#include <algorithm>

namespace ioa {

  class binding_record_interface
  {
  public:
    virtual ~binding_record_interface () { }
    virtual action_interface& output_action () = 0;
    virtual action_interface& input_action () = 0;
    virtual aid_t binder () const = 0;
    virtual bid_t bid () const = 0;
  };

  class unvalued_binding_record_interface :
    public binding_record_interface
  {
  public:
    virtual ~unvalued_binding_record_interface () { }
    virtual bool execute_output () const = 0;
    virtual void execute_input () const = 0;
  };

  template <class OI, class OM, class II, class IM, class I, class D>
  class unvalued_binding_record :
    public unvalued_binding_record_interface
  {
  private:
    const bid_t m_bid;
    OI& m_output_ref;
    action<OI, OM> m_output_action;
    II& m_input_ref;
    action<II, IM> m_input_action;
    I& m_binder_ref;
    const aid_t m_binder_aid;
    D& m_d;

  public:
    unvalued_binding_record (const bid_t bid,
			     OI& output_ref,
			     const action<OI, OM>& output_action,
			     II& input_ref,
			     const action<II, IM>& input_action,
			     I& binder_ref,
			     const aid_t binder_aid,
			     D& d) :
      m_bid (bid),
      m_output_ref (output_ref),
      m_output_action (output_action),
      m_input_ref (input_ref),
      m_input_action (input_action),
      m_binder_ref (binder_ref),
      m_binder_aid (binder_aid),
      m_d (d)
    {
      system_scheduler::set_current_aid (m_binder_aid, m_binder_ref);
      m_binder_ref.bound (m_bid, m_d);
      system_scheduler::clear_current_aid ();
      
      system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
      m_output_action.bound (m_output_ref);
      system_scheduler::clear_current_aid ();

      system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
      m_input_action.bound (m_input_ref);
      system_scheduler::clear_current_aid ();
    }

    virtual ~unvalued_binding_record () {
      system_scheduler::set_current_aid (m_binder_aid, m_binder_ref);
      m_binder_ref.unbound (m_d);
      system_scheduler::clear_current_aid ();
      
      system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
      m_output_action.unbound (m_output_ref);
      system_scheduler::clear_current_aid ();

      system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
      m_input_action.unbound (m_input_ref);
      system_scheduler::clear_current_aid ();
    }

    bool execute_output () const {
      system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
      bool retval = m_output_action (m_output_ref);
      system_scheduler::clear_current_aid ();
      return retval;
    }

    void execute_input () const {
      system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
      m_input_action (m_input_ref);
      system_scheduler::clear_current_aid ();
    }

    action_interface& output_action () {
      return m_output_action;
    }

    action_interface& input_action () {
      return m_input_action;
    }

    aid_t binder () const {
      return m_binder_aid;
    }

    bid_t bid () const {
      return m_bid;
    }
  };

  template <class T>
  class valued_binding_record_interface :
    public binding_record_interface
  {
  public:
    virtual ~valued_binding_record_interface () { }
    virtual const std::pair<bool, T> execute_output () const = 0;
    virtual void execute_input (const T&) const = 0;
  };

  template <class OI, class OM, class II, class IM, class I, class D>
  class valued_binding_record :
    public valued_binding_record_interface<typename OM::value_type>
  {
  private:
    typedef typename OM::value_type T;

    const bid_t m_bid;
    OI& m_output_ref;
    action<OI, OM> m_output_action;
    II& m_input_ref;
    action<II, IM> m_input_action;
    I& m_binder_ref;
    const aid_t m_binder_aid;
    D& m_d;

  public:
    valued_binding_record (const bid_t bid,
			   OI& output_ref,
			   const action<OI, OM>& output_action,
			   II& input_ref,
			   const action<II, IM>& input_action,
			   I& binder_ref,
			   const aid_t binder_aid,
			   D& d) :
      m_bid (bid),
      m_output_ref (output_ref),
      m_output_action (output_action),
      m_input_ref (input_ref),
      m_input_action (input_action),
      m_binder_ref (binder_ref),
      m_binder_aid (binder_aid),
      m_d (d)
    {
      system_scheduler::set_current_aid (m_binder_aid, m_binder_ref);
      m_binder_ref.bound (m_bid, m_d);
      system_scheduler::clear_current_aid ();
      
      system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
      m_output_action.bound (m_output_ref);
      system_scheduler::clear_current_aid ();

      system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
      m_input_action.bound (m_input_ref);
      system_scheduler::clear_current_aid ();
    }

    virtual ~valued_binding_record () {
      system_scheduler::set_current_aid (m_binder_aid, m_binder_ref);
      m_binder_ref.unbound (m_d);
      system_scheduler::clear_current_aid ();
      
      system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
      m_output_action.unbound (m_output_ref);
      system_scheduler::clear_current_aid ();

      system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
      m_input_action.unbound (m_input_ref);
      system_scheduler::clear_current_aid ();
    }

    const std::pair<bool, T> execute_output () const {
      system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
      std::pair<bool, T> retval = m_output_action (m_output_ref);
      system_scheduler::clear_current_aid ();
      return retval;
    }
    
    void execute_input (const T& t) const {
      system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
      m_input_action (m_input_ref, t);
      system_scheduler::clear_current_aid ();
    }

    action_interface& output_action () {
      return m_output_action;
    }

    action_interface& input_action () {
      return m_input_action;
    }

    aid_t binder () const {
      return m_binder_aid;
    }

    bid_t bid () const {
      return m_bid;
    }
  };

  class binding_equal
  {
  private:
    const action_interface& m_output;
    const action_interface& m_input;
    const aid_t m_binder;
    
  public:
    binding_equal (const action_interface& o,
  		   const action_interface& i,
  		   const aid_t binder) :
      m_output (o),
      m_input (i),
      m_binder (binder)
    { }
    
    bool operator() (binding_record_interface* const& x) const {
      return m_output == x->output_action () &&
  	m_input == x->input_action ()
  	&& m_binder == x->binder ();
    }
  };
  
  class input_equal
  {
  private:
    const action_interface& m_input;
    
  public:
    input_equal (const action_interface& i) :
      m_input (i)
    { }
    
    bool operator() (binding_record_interface* const& x) const {
      return m_input == x->input_action ();
    }
  };
  
  class input_automaton_equal
  {
  private:
    aid_t m_automaton;
    
  public:
    input_automaton_equal (const aid_t automaton) :
      m_automaton (automaton)
    { }
    
    bool operator() (binding_record_interface* const& x) const {
      return m_automaton == x->input_action ().get_aid ();
    }
  };
  
  class aid_bid_equal
  {
  private:
    const aid_t m_aid;
    const bid_t m_bid;

  public:
    aid_bid_equal (const aid_t aid,
  		   const bid_t bid) :
      m_aid (aid),
      m_bid (bid)
    { }

    bool operator() (binding_record_interface* const& x) const {
      return m_aid == x->binder () && m_bid == x->bid ();
    }
  };

  class binding_interface
  {
  public:
    virtual ~binding_interface () { }
    virtual bool involves_output (const action_interface& output) const = 0;
    virtual bool involves_binding (const action_interface& output,
  				   const action_interface& input,
  				   const aid_t binder) const = 0;
    virtual bool involves_input (const action_interface& input) const = 0;
    virtual bool involves_input_automaton (const aid_t automaton) const = 0;
    virtual bool involves_aid_bid (const aid_t binder,
  				   const bid_t bid) const = 0;
    virtual bool empty () const = 0;
    virtual void execute () = 0;
    virtual void unbind (const aid_t binder,
  			 const bid_t bid) = 0;
    virtual void unbind_automaton (const aid_t automaton) = 0;
  };
  

  struct binding_record_compare
  {
    // We order the automata by aid so the locking is in order.
    bool operator() (binding_record_interface* const& x,
  		     binding_record_interface* const& y) const {
      return x->input_action ().get_aid () < y->input_action ().get_aid ();
    }
  };

  template <class B>
  class generic_binding_impl :
    public binding_interface
  {
  protected:
    typedef std::set<B*, binding_record_compare> set_type;
    set_type m_inputs;
    
  public:
    ~generic_binding_impl () {
      for (typename set_type::iterator pos = m_inputs.begin ();
  	   pos != m_inputs.end ();
  	   ++pos) {
  	delete (*pos);
      }
    }

  protected:
    action_interface& get_output () const {
      assert (!m_inputs.empty ());
      return (*m_inputs.begin ())->output_action ();
    }

  public:
    bool involves_output (const action_interface& output) const {
      if (!m_inputs.empty ()) {
    	return get_output () == output;
      }
      else {
    	return false;
      }
    }
    
    bool involves_binding (const action_interface& output,
  			   const action_interface& input,
  			   const aid_t binder) const {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   binding_equal (output, input, binder)) != m_inputs.end ();
    }
    
    bool involves_input (const action_interface& input) const {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   input_equal (input)) != m_inputs.end ();
    }
    
    bool
    involves_input_automaton (const aid_t automaton) const
    {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   input_automaton_equal (automaton)) != m_inputs.end ();
    }
    
    bool
    involves_aid_bid (const aid_t binder,
  		      const bid_t bid) const
    {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   aid_bid_equal (binder, bid)) != m_inputs.end ();
    }

    void unbind (const aid_t binder,
  		 const bid_t bid) {
      assert (!m_inputs.empty ());
      typename set_type::iterator pos = std::find_if (m_inputs.begin (),
						      m_inputs.end (),
						      aid_bid_equal (binder, bid));
      if (pos != m_inputs.end ()) {
  	binding_record_interface* ptr = *pos;
  	m_inputs.erase (pos);
  	delete ptr;
      }
    }

    bool empty () const {
      return m_inputs.empty ();
    }

    void execute () {
      bool output_processed;

      // Lock in order.
      output_processed = false;
      for (typename set_type::iterator pos = m_inputs.begin ();
  	   pos != m_inputs.end ();
  	   ++pos) {
  	if (!output_processed &&
  	    (*pos)->output_action ().get_aid () < (*pos)->input_action ().get_aid ()) {
	  automaton_locker::lock_automaton ((*pos)->output_action ().get_aid ());
  	  output_processed = true;
  	}
	automaton_locker::lock_automaton ((*pos)->input_action ().get_aid ());
      }

      // Execute.
      execute_dispatch ();

      // Unlock.
      output_processed = false;
      for (typename set_type::iterator pos = m_inputs.begin ();
  	   pos != m_inputs.end ();
  	   ++pos) {
  	if (!output_processed &&
  	    (*pos)->output_action ().get_aid () < (*pos)->input_action ().get_aid ()) {
	  automaton_locker::unlock_automaton ((*pos)->output_action ().get_aid ());
  	  output_processed = true;
  	}
	automaton_locker::unlock_automaton ((*pos)->input_action ().get_aid ());
      }

    }

    void unbind_automaton (const aid_t automaton) {
      if (get_output ().get_aid () == automaton) {
  	// Unbind all.
  	for (typename set_type::iterator pos = m_inputs.begin ();
  	     pos != m_inputs.end ();
  	     ++pos) {
  	  delete (*pos);
  	}
  	m_inputs.clear ();
      }
      else {
  	// Try the inputs.
  	for (typename set_type::iterator pos = m_inputs.begin ();
  	     pos != m_inputs.end ();
  	     ++pos) {
  	  if ((*pos)->input_action ().get_aid () == automaton ||
  	      (*pos)->binder () == automaton) {
  	    delete (*pos);
  	    m_inputs.erase (pos);
  	    break;
  	  }
  	}
      }
    }

  protected:
    virtual void execute_dispatch () = 0;
  };
  
  template <class VS, class VT> class binding_impl;

  template <>
  class binding_impl<unvalued, null_type> :
    public generic_binding_impl<unvalued_binding_record_interface>
  {
  public:

    template <class OI, class OM, class II, class IM, class I, class D>
    void bind (const bid_t bid,
	       OI& output_ref,
  	       const action<OI, OM>& output_action,
	       II& input_ref,
  	       const action<II, IM>& input_action,
  	       I& binder_ref,
  	       const aid_t binder_aid,
  	       D& d) {
      // Can't bind to self.
      assert (output_action.automaton.aid () != input_action.automaton.aid ());
      // Can't already involve input.
      assert (!involves_input_automaton (input_action.automaton.aid ()));
      // Sanity check.
      if (!m_inputs.empty ()) {
  	assert (output_action == get_output ());
      }
      
      unvalued_binding_record_interface* record = new unvalued_binding_record<OI, OM, II, IM, I, D> (bid, output_ref, output_action, input_ref, input_action, binder_ref, binder_aid, d);
      m_inputs.insert (record);
    }

  protected:
    typedef generic_binding_impl<unvalued_binding_record_interface>::set_type set_type;

    void execute_dispatch () {
      set_type::iterator out_pos = m_inputs.begin ();
      if ((*out_pos)->execute_output ()) {
  	for (set_type::iterator pos = m_inputs.begin ();
  	     pos != m_inputs.end ();
  	     ++pos) {
  	  (*pos)->execute_input ();
  	}
      }
    }
  };

  template <class T>
  class binding_impl<valued, T> :
    public generic_binding_impl<valued_binding_record_interface<T> >
  {
  public:

    template <class OI, class OM, class II, class IM, class I, class D>
    void bind (const bid_t bid,
	       OI& output_ref,
  	       const action<OI, OM>& output_action,
	       II& input_ref,
  	       const action<II, IM>& input_action,
  	       I& binder_ref,
  	       const aid_t binder_aid,
  	       D& d) {
      // Can't bind to self.
      assert (output_action.automaton.aid () != input_action.automaton.aid ());
      // Can't already involve input.
      assert (!involves_input_automaton (input_action.automaton.aid ()));
      // Sanity check.
      if (!this->m_inputs.empty ()) {
  	assert (output_action == this->get_output ());
      }
      
      valued_binding_record_interface<T>* record = new valued_binding_record<OI, OM, II, IM, I, D> (bid, output_ref, output_action, input_ref, input_action, binder_ref, binder_aid, d);
      this->m_inputs.insert (record);
    }

  protected:
    typedef typename generic_binding_impl<valued_binding_record_interface<T> >::set_type set_type;

    void execute_dispatch () {
      typename set_type::iterator out_pos = this->m_inputs.begin ();
      const std::pair<bool, T> r = (*out_pos)->execute_output ();
      if (r.first) {
  	for (typename set_type::iterator pos = this->m_inputs.begin ();
  	     pos != this->m_inputs.end ();
  	     ++pos) {
  	  (*pos)->execute_input (r.second);
  	}
      }
    }

  };
  
  template <class Member>
  class binding :
    public binding_impl<typename Member::value_status,
			typename Member::value_type>
  {
  public:
    typedef typename Member::value_status value_status;
    typedef typename Member::value_type value_type;

  };

}

#endif
