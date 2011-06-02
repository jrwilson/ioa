#include <ioa/automaton_record.hpp>

namespace ioa {

  automaton_record_interface::automaton_record_interface (automaton_interface* instance,
							  const aid_t aid) :
    m_instance (instance),
    m_aid (aid),
    m_parent (0)
  { }

  automaton_record_interface::~automaton_record_interface () {
    assert (m_children.empty ());
  }

  const aid_t automaton_record_interface::get_aid () const {
    return m_aid;
  }

  automaton_interface* automaton_record_interface::get_instance () const {
    return m_instance.get ();
  }

  bid_t automaton_record_interface::take_bid () {
    return m_bids.take ();
  }

  void automaton_record_interface::replace_bid (const bid_t bid) {
    m_bids.replace (bid);
  }

  void automaton_record_interface::add_child (automaton_record_interface* child) {
    m_children.insert (child);
    child->set_parent (this);
  }

  void automaton_record_interface::remove_child (automaton_record_interface* child) {
    m_children.erase (child);
  }

  automaton_record_interface* automaton_record_interface::get_child () const {
    if (m_children.empty ()) {
      return 0;
    }
    else {
      return *(m_children.begin ());
    }
  }

  void automaton_record_interface::set_parent (automaton_record_interface* parent) {
    m_parent = parent;
  }
    
  automaton_record_interface* automaton_record_interface::get_parent () const {
    return m_parent;
  }

}
