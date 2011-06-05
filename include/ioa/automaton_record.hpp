#ifndef __automaton_record_hpp__
#define __automaton_record_hpp__

#include <ioa/mutex.hpp>
#include <ioa/automaton_interface.hpp>
#include <ioa/aid.hpp>
#include <ioa/sequential_set.hpp>
#include <ioa/bid.hpp>
#include <ioa/system_scheduler.hpp>

#include <memory>
#include <cassert>


namespace ioa {

  class automaton_record :
    public mutex
  {
  private:
    std::auto_ptr<automaton_interface> m_instance;
    aid_t m_aid;
    sequential_set<bid_t> m_bids;
    std::set<automaton_record*> m_children;
    automaton_record* m_parent;
    
  public:
    automaton_record (automaton_interface* instance,
		      const aid_t aid);
    ~automaton_record ();
    const aid_t get_aid () const;
    automaton_interface* get_instance () const;
    bid_t take_bid ();
    void replace_bid (const bid_t bid);
    void add_child (automaton_record* child);
    void remove_child (automaton_record* child);
    automaton_record* get_child () const;
    void set_parent (automaton_record* parent);
    automaton_record* get_parent () const;
  };
  
}

#endif
