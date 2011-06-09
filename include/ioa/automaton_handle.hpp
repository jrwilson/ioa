#ifndef __automaton_handle_hpp__
#define __automaton_handle_hpp__

#include <ioa/aid.hpp>

namespace ioa {

  // Be be type-safe, we associate a type with an automaton ID and call it an automaton handle.

  template <typename T>
  class automaton_handle
  {
  private:
    aid_t m_aid;

  public:
    automaton_handle () :
      m_aid (-1)
    { }

    automaton_handle (aid_t a) :
      m_aid (a)
    { }

    operator aid_t () const {
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
