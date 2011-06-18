#include "automaton_record.hpp"
#include <ioa/automaton.hpp>
#include <ioa/system_scheduler_interface.hpp>

#include <cassert>

namespace ioa {

  automaton_record::automaton_record (system_scheduler_interface& system_scheduler,
				      automaton* instance,
				      const aid_t aid) :
    m_system_scheduler (system_scheduler),
    m_instance (instance),
    m_aid (aid),
    m_key (0),
    m_parent (0)
  { }

  automaton_record::~automaton_record () {
    // Sanity check.
    assert (m_children.empty ());
  }

  const aid_t automaton_record::get_aid () const {
    return m_aid;
  }

  automaton* automaton_record::get_instance () const {
    return m_instance.get ();
  }

  bool automaton_record::create_key_exists (void* const key) const {
    return m_children.count (key) != 0;
  }

  void automaton_record::add_child (void* const key,
				    automaton_record* child) {
    m_children.insert (std::make_pair (key, child));
    // Tell the parent that the child was created.
    m_system_scheduler.automaton_created (m_aid, key, child->m_aid);
  }

  void automaton_record::remove_child (void* const key) {
    m_children.erase (key);
    // Tell the parent that the child was destroyed.
    m_system_scheduler.automaton_destroyed (m_aid, key);
  }

  automaton_record* automaton_record::get_child (void* const key) const {
    std::map<void*, automaton_record*>::const_iterator pos = m_children.find (key);
    if (pos != m_children.end ()) {
      return pos->second;
    }
    else {
      return 0;
    }
  }

  std::pair<void*, automaton_record*> automaton_record::get_first_child () const {
    if (m_children.empty ()) {
      return std::pair<void*, automaton_record*> (0, 0);
    }
    else {
      return *(m_children.begin ());
    }
  }

  void automaton_record::set_parent (void* const key,
				     automaton_record* parent) {
    m_key = key;
    m_parent = parent;
  }

  void* automaton_record::get_key () const {
    return m_key;
  }

  automaton_record* automaton_record::get_parent () const {
    return m_parent;
  }

  bool automaton_record::bind_key_exists (void* const key) const {
    return m_bind_keys.count (key) != 0;
  }

  void automaton_record::add_bind_key (void* const key) {
    m_bind_keys.insert (key);
  }

  void automaton_record::remove_bind_key (void* const key) {
    m_bind_keys.erase (key);
  }

}
