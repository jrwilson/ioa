#include <ioa/automaton_record.hpp>

namespace ioa {

  automaton_record::automaton_record (automaton_interface* instance,
							  const aid_t aid) :
    m_instance (instance),
    m_aid (aid),
    m_parent (0)
  { }

  automaton_record::~automaton_record () {
    // Sanity check.
    assert (m_children.empty ());
    
    if (this->get_parent () != 0) {
      // Inform the parent that this child is destroyed.
      aid_t parent_aid = this->get_parent ()->get_aid ();
      system_scheduler::automaton_destroyed (parent_aid, m_aid);
    }     
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

  void automaton_record::add_child (automaton_record* child) {
    m_children.insert (child);
    child->set_parent (this);
  }

  void automaton_record::remove_child (automaton_record* child) {
    m_children.erase (child);
  }

  automaton_record* automaton_record::get_child () const {
    if (m_children.empty ()) {
      return 0;
    }
    else {
      return *(m_children.begin ());
    }
  }

  void automaton_record::set_parent (automaton_record* parent) {
    m_parent = parent;
  }
    
  automaton_record* automaton_record::get_parent () const {
    return m_parent;
  }

}
