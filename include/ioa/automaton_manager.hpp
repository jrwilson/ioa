/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __automaton_manager_hpp__
#define __automaton_manager_hpp__

#include <ioa/automaton_manager_interface.hpp>

namespace ioa {
  
  template <class I>
  class automaton_manager :
    public system_automaton_manager_interface,
    public automaton_manager_interface,
    public automaton_handle_interface<I>
  {
  public:
    typedef I instance;

  private:
    automaton* m_automaton;
    std::auto_ptr<typed_allocator_interface<I> > m_allocator;
    state_t m_state;
    automaton_handle<I> m_handle;

  public:
    automaton_manager (automaton* automaton,
		       std::auto_ptr<typed_allocator_interface<I> > allocator) :
      m_automaton (automaton),
      m_allocator (allocator),
      m_state (START)
    {
      m_automaton->create (this);
    }

  private:
    // Must use new/destroy.
    ~automaton_manager () { }

  public:

    void destroy () {
      m_automaton->destroy (this);
    }

    std::auto_ptr<allocator_interface> get_allocator () {
      return std::auto_ptr<allocator_interface> (m_allocator);
    }

    void created (const created_t result,
		  const aid_t aid) {
      switch (result) {
      case CREATE_KEY_EXISTS_RESULT:
	// System is not correct.
	assert (false);
	break;
      case INSTANCE_EXISTS_RESULT:
	m_state = INSTANCE_EXISTS;
	m_handle = -1;
	this->notify_observers ();
	delete this;
	break;
      case AUTOMATON_CREATED_RESULT:
	m_state = CREATED;
	m_handle = aid;
	this->notify_observers ();
	break;
      }
    }

    void destroyed (const destroyed_t result) {
      switch (result) {
      case CREATE_KEY_DNE_RESULT:
	// System is not correct.
	assert (false);
	break;	
      case AUTOMATON_DESTROYED_RESULT:
	m_state = DESTROYED;
	m_handle = -1;
	this->notify_observers ();
	delete this;
      }
    }
  
    state_t get_state () const {
      return m_state;
    }

    automaton_handle<I> get_handle () const {
      return m_handle;
    }

  };

  template <typename I>
  automaton_manager<I>* make_automaton_manager (automaton* automaton,
						std::auto_ptr<typed_allocator_interface<I> > allocator) {
    return new automaton_manager<I> (automaton, allocator);
  }

}

#endif
