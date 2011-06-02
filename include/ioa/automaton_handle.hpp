#ifndef __automaton_handle_hpp__
#define __automaton_handle_hpp__

#include <ioa/aid.hpp>

namespace ioa {

  // Be be type-safe, we associate a type with an automaton ID and call it an automaton handle.

  template <class T>
  class automaton_handle
  {
  private:
    aid_t m_aid;

  public:
    automaton_handle () :
      m_aid (-1)
    { }

    // The only entity that should strike new handles is the system.
    // However, if TEST_AUTOMATON_CTOR is defined, then anyone can strike new handles.
#ifndef TEST_AUTOMATON_CTOR
  private:
    friend class system;
#endif
    automaton_handle (aid_t a) :
      m_aid (a)
    { }

  public:

    aid_t aid () const {
      return m_aid;
    }

    automaton_handle& operator= (const automaton_handle& handle) {
      if (this != &handle) {
	m_aid = handle.m_aid;
      }
      return *this;
    }

    bool operator== (const automaton_handle& handle) const {
      return m_aid == handle.m_aid;
    }
  };

}

#endif
