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
#include <map>

namespace ioa {

  class automaton_record :
    public mutex
  {
  private:
    std::auto_ptr<automaton_interface> m_instance;
    aid_t m_aid;
    sequential_set<bid_t> m_bids;
    std::map<void*, automaton_record*> m_children;
    void* m_key;
    automaton_record* m_parent;
    
  public:
    automaton_record (automaton_interface* instance,
		      aid_t const aid);
    ~automaton_record ();
    const aid_t get_aid () const;
    automaton_interface* get_instance () const;
    bid_t take_bid ();
    void replace_bid (const bid_t bid);
    bool create_key_exists (void* const key) const;
    void add_child (void* const key,
		    automaton_record* child);
    void remove_child (void* const key);
    std::pair<void*, automaton_record*> get_first_child () const;
    void set_parent (void* const key,
		     automaton_record* parent);
    void* get_key () const;
    automaton_record* get_parent () const;
  };
  
}

#endif
