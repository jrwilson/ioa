#ifndef __automaton_record_hpp__
#define __automaton_record_hpp__

#include <ioa/mutex.hpp>
#include <ioa/automaton_interface.hpp>
#include <ioa/sequential_set.hpp>
#include <ioa/system_scheduler.hpp>
#include <ioa/bid.hpp>

#include <memory>
#include <cassert>

namespace ioa {

  class automaton_record_interface :
    public mutex
  {
  private:
    std::auto_ptr<automaton_interface> m_instance;
    aid_t m_aid;
    sequential_set<bid_t> m_bids;
    std::set<automaton_record_interface*> m_children;
    automaton_record_interface* m_parent;

  public:
    automaton_record_interface (automaton_interface* instance,
				const aid_t aid);
    virtual ~automaton_record_interface ();
    const aid_t get_aid () const;
    automaton_interface* get_instance () const;
    bid_t take_bid ();
    void replace_bid (const bid_t bid);
    void add_child (automaton_record_interface* child);
    void remove_child (automaton_record_interface* child);
    automaton_record_interface* get_child () const;
    void set_parent (automaton_record_interface* parent);
    automaton_record_interface* get_parent () const;
  };

  class root_automaton_record :
    public automaton_record_interface
  {
  public:
    root_automaton_record (automaton_interface* instance,
			   const aid_t aid) :
      automaton_record_interface (instance, aid)
    { }
  };

  template <class P, class D>
  class automaton_record :
    public automaton_record_interface
  {
  private:
    P* m_parent_instance;
    D& m_d;
    
  public:
    automaton_record (automaton_interface* instance,
		      const aid_t aid,
		      P* parent_instance,
		      D& d) :
      automaton_record_interface (instance, aid),
      m_parent_instance (parent_instance),
      m_d (d)
    { }
    
    ~automaton_record () {
      assert (this->get_parent () != 0);
      aid_t parent_aid = this->get_parent ()->get_aid ();
      system_scheduler::set_current_aid (parent_aid, *m_parent_instance);
      m_parent_instance->automaton_destroyed (m_d);
      system_scheduler::clear_current_aid ();
    }

  };

}

#endif
