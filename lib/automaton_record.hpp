#ifndef __automaton_record_hpp__
#define __automaton_record_hpp__

#include <ioa/aid.hpp>
#include "mutex.hpp"
#include "sequential_set.hpp"

#include <memory>
#include <map>

namespace ioa {

  class system_scheduler_interface;
  class automaton;

  class automaton_record :
    public mutex
  {
  private:
    system_scheduler_interface& m_system_scheduler;
    std::auto_ptr<automaton> m_instance;
    aid_t m_aid;
    std::map<void*, automaton_record*> m_children;
    void* m_key;
    automaton_record* m_parent;
    std::set<void*> m_bind_keys;
    
  public:
    automaton_record (system_scheduler_interface&,
		      automaton* instance,
		      aid_t const aid);
    ~automaton_record ();
    const aid_t get_aid () const;
    automaton* get_instance () const;
    bool create_key_exists (void* const key) const;
    void add_child (void* const key,
		    automaton_record* child);
    void remove_child (void* const key);
    automaton_record* get_child (void* const key) const;
    std::pair<void*, automaton_record*> get_first_child () const;
    void set_parent (void* const key,
		     automaton_record* parent);
    void* get_key () const;
    automaton_record* get_parent () const;
    bool bind_key_exists (void* const key) const;
    void add_bind_key (void* const key);
    void remove_bind_key (void* const key);
  };
  
}

#endif
