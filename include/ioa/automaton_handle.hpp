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
