#include <ioa/automaton_record.hpp>

namespace ioa {

  automaton_record::automaton_record (automaton_interface* instance,
							  const aid_t aid) :
    m_instance (instance),
    m_aid (aid),
    m_key (0),
    m_parent (0)
  {
    // Initialize the automaton.
    system_scheduler::init (m_aid);
  }

  automaton_record::~automaton_record () {
    // Sanity check.
    assert (m_children.empty ());
  }

  const aid_t automaton_record::get_aid () const {
    return m_aid;
  }

  automaton_interface* automaton_record::get_instance () const {
    return m_instance.get ();
  }

  bid_t automaton_record::take_bid () {
    return m_bids.take ();
  }

  void automaton_record::replace_bid (const bid_t bid) {
    m_bids.replace (bid);
  }

  bool automaton_record::create_key_exists (void* const key) const {
    return m_children.count (key) != 0;
  }

  void automaton_record::add_child (void* const key,
				    automaton_record* child) {
    m_children.insert (std::make_pair (key, child));
    // Tell the parent that the child was created.
    system_scheduler::automaton_created (m_aid, key, child->m_aid);
  }

  void automaton_record::remove_child (void* const key) {
    m_children.erase (key);
    // Tell the parent that the child was destroyed.
    system_scheduler::automaton_destroyed (m_aid, key);
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

}
