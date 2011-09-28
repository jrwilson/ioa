#ifndef __binding_set_hpp__
#define __binding_set_hpp__

#include <ioa/aid.hpp>
#include <ioa/action_executor.hpp>
#include <set>
#include <map>

namespace ioa {

  // TODO: .cpp file

  template <class T>
  class ptr_holder
  {
  private:
    T* m_ptr;

  public:
    ptr_holder (const T& ref) :
      m_ptr (ref.clone ())
    { }

    ptr_holder (const ptr_holder& other) :
      m_ptr (other.m_ptr->clone ())
    { }

    ~ptr_holder () {
      delete m_ptr;
    }

    T& operator* () const {
      return *m_ptr;
    }

    T* operator-> () const {
      return m_ptr;
    }

    bool operator< (const ptr_holder& other) const {
      return *m_ptr < *other.m_ptr;
    }
  };

  class binding_set
  {
  private:
    typedef ptr_holder<output_executor_interface> output_action_t;
    typedef ptr_holder<input_executor_interface> input_action_t;
    typedef std::map<std::pair<aid_t, void*>, std::pair<output_action_t, input_action_t> > key_to_action_type;
    typedef std::set<input_action_t> input_set_type;
    typedef std::set<std::pair<output_action_t, aid_t> > output_to_aid_type;
    typedef std::map<output_action_t, input_set_type> output_to_inputs_type;
  public:
    typedef std::set<std::pair<aid_t, void*> > key_set_type;
  private:
    typedef std::map<aid_t, key_set_type> aid_to_keys_type;

    input_set_type m_empty_inputs;

    key_to_action_type m_key_to_action;
    input_set_type m_input_set;
    output_to_aid_type m_output_to_aid;
    output_to_inputs_type m_output_to_inputs;
    aid_to_keys_type m_aid_to_keys;

  public:
    typedef input_set_type::const_iterator iterator;

    bool exists (aid_t const owner,
		 void* const key) const {
      return m_key_to_action.count (std::make_pair (owner, key)) != 0;
    }

    bool bound (const input_executor_interface& input_action) const {
      return m_input_set.count (input_action_t (input_action)) != 0;
    }

    bool bound (const output_executor_interface& output_action,
		aid_t const input_aid) const {
      return m_output_to_aid.count (std::make_pair (output_action_t (output_action), input_aid)) != 0;
    }

    size_t binding_count (const output_executor_interface& output_action) {
      output_to_inputs_type::const_iterator pos = m_output_to_inputs.find (output_action_t (output_action));
      if (pos != m_output_to_inputs.end ()) {
	return pos->second.size ();
      }
      else {
	return 0;
      }
    }

    size_t binding_count (const input_executor_interface& input_action) {
      return m_input_set.count (input_action_t (input_action));
    }

    iterator begin (const output_executor_interface& output_action) const {
      output_to_inputs_type::const_iterator pos = m_output_to_inputs.find (output_action_t (output_action));
      if (pos != m_output_to_inputs.end ()) {
	return pos->second.begin ();
      }
      else {
	return m_empty_inputs.begin ();
      }
    }

    iterator end (const output_executor_interface& output_action) const {
      output_to_inputs_type::const_iterator pos = m_output_to_inputs.find (output_action_t (output_action));
      if (pos != m_output_to_inputs.end ()) {
	return pos->second.end ();
      }
      else {
	return m_empty_inputs.end ();
      }
    }

    key_set_type keys (aid_t const aid) const {
      aid_to_keys_type::const_iterator pos = m_aid_to_keys.find (aid);
      if (pos != m_aid_to_keys.end ()) {
	return pos->second;
      }
      else {
	return key_set_type ();
      }
    }

    void bind (aid_t const owner,
	       void* const key,
	       const output_executor_interface& output_action,
	       const input_executor_interface& input_action) {
      const output_action_t out (output_action);
      const input_action_t in (input_action);
      m_key_to_action.insert (std::make_pair (std::make_pair (owner, key), std::make_pair (out, in)));
      m_input_set.insert (in);
      m_output_to_aid.insert (std::make_pair (out, input_action.get_aid ()));
      {
	output_to_inputs_type::iterator pos = m_output_to_inputs.find (out);
	if (pos == m_output_to_inputs.end ()) {
	  std::pair<output_to_inputs_type::iterator, bool> x = m_output_to_inputs.insert (std::make_pair (out, input_set_type ()));
	  pos = x.first;
	}
	pos->second.insert (in);
      }
      {
	aid_to_keys_type::iterator pos = m_aid_to_keys.find (owner);
	if (pos == m_aid_to_keys.end ()) {
	  std::pair<aid_to_keys_type::iterator, bool> x = m_aid_to_keys.insert (std::make_pair (owner, key_set_type ()));
	  pos = x.first;
	}
	pos->second.insert (std::make_pair (owner, key));
      }
      {
	aid_to_keys_type::iterator pos = m_aid_to_keys.find (output_action.get_aid ());
	if (pos == m_aid_to_keys.end ()) {
	  std::pair<aid_to_keys_type::iterator, bool> x = m_aid_to_keys.insert (std::make_pair (output_action.get_aid (), key_set_type ()));
	  pos = x.first;
	}
	pos->second.insert (std::make_pair (owner, key));
      }
      {
	aid_to_keys_type::iterator pos = m_aid_to_keys.find (input_action.get_aid ());
	if (pos == m_aid_to_keys.end ()) {
	  std::pair<aid_to_keys_type::iterator, bool> x = m_aid_to_keys.insert (std::make_pair (input_action.get_aid (), key_set_type ()));
	  pos = x.first;
	}
	pos->second.insert (std::make_pair (owner, key));
      }
      
    }

    void unbind (aid_t const owner,
		 void* const key) {
      std::pair<aid_t, void*> owner_key (owner, key);

      key_to_action_type::iterator pos = m_key_to_action.find (owner_key);
      std::pair<output_action_t, input_action_t> p (pos->second.first, pos->second.second);
      m_key_to_action.erase (pos);
      m_input_set.erase (p.second);
      m_output_to_aid.erase (std::make_pair (p.first, p.second->get_aid ()));
      {
	output_to_inputs_type::iterator pos2 = m_output_to_inputs.find (p.first);
	pos2->second.erase (p.second);
	if (pos2->second.empty ()) {
	  m_output_to_inputs.erase (pos2);
	}
      }
      {
	aid_to_keys_type::iterator pos2 = m_aid_to_keys.find (owner);
	pos2->second.erase (owner_key);
	if (pos2->second.empty ()) {
	  m_aid_to_keys.erase (pos2);
	}
      }
      {
	aid_to_keys_type::iterator pos2 = m_aid_to_keys.find (p.first->get_aid ());
	pos2->second.erase (owner_key);
	if (pos2->second.empty ()) {
	  m_aid_to_keys.erase (pos2);
	}
      }
      {
	aid_to_keys_type::iterator pos2 = m_aid_to_keys.find (p.second->get_aid ());
	pos2->second.erase (owner_key);
	if (pos2->second.empty ()) {
	  m_aid_to_keys.erase (pos2);
	}
      }
    }
  };

}

#endif
