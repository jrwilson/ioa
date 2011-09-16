#ifndef __automaton_set_hpp__
#define __automaton_set_hpp__

#include <set>
#include <map>

#include <ioa/aid.hpp>
#include <ioa/automaton_base.hpp>

namespace ioa {

  // TODO: .cpp file.
  
  class automaton_set
  {
    // TODO:  Use hashes.
  public:
    typedef std::map<void*, aid_t> key_set_type;

  private:
    typedef std::map<aid_t, automaton_base*> aid_set_type;
    typedef std::map<aid_t, key_set_type> parent_key_type;

    aid_t m_next_aid;
    std::set<automaton_base*> m_instances;
    aid_set_type m_aids;
    parent_key_type m_parent_key;

  public:
    automaton_set () :
      m_next_aid (0)
    { }

    aid_t create (automaton_base* instance) {
      aid_t aid;
      do {
	aid = m_next_aid++;
	if (aid < 0) {
	  aid = 0;
	}
      } while (m_aids.count (aid) != 0);
      m_instances.insert (instance);
      m_aids.insert (std::make_pair (aid, instance));
      m_parent_key.insert (std::make_pair (aid, key_set_type ()));
      return aid;
    }

    aid_t create (aid_t parent,
		  void* key,
		  automaton_base* instance) {
      aid_t aid = create (instance);
      m_parent_key[parent].insert (std::make_pair (key, aid));
      return aid;      
    }

    bool exists (aid_t parent,
		 void* key) const {
      parent_key_type::const_iterator pos = m_parent_key.find (parent);
      if (pos != m_parent_key.end ()) {
	return pos->second.count (key) != 0;
      }
      else {
	return false;
      }
    }

    bool exists (automaton_base* instance) const {
      return m_instances.count (instance) != 0;
    }

    bool exists (aid_t aid) const {
      return m_aids.count (aid) != 0;
    }
    
    key_set_type keys (aid_t parent) const {
      return m_parent_key.find (parent)->second;
    }

    automaton_base* destroy (aid_t parent,
			     void* key) {
      key_set_type::iterator pos1 = m_parent_key[parent].find (key);
      const aid_t child = pos1->second;
      m_parent_key[parent].erase (pos1);

      parent_key_type::iterator pos3 = m_parent_key.find (child);
      m_parent_key.erase (pos3);

      aid_set_type::iterator pos2 = m_aids.find (child);
      automaton_base* instance = pos2->second;
      m_aids.erase (pos2);
      
      m_instances.erase (instance);

      return instance;
    }
  };

}

#endif
