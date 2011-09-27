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

#ifndef __channel_automaton_hpp__
#define __channel_automaton_hpp__

/*
  Channel I/O Automaton
  Distributed Algorithms, p. 204.
*/

#include <queue>
#include <ioa/ioa.hpp>

template <class T>
class channel_automaton :
  public ioa::automaton
{
private:
  std::queue<T> m_queue;

  void send_effect (const T& t) {
    m_queue.push (t);
  }
  
  void send_schedule () const {
    receive_schedule ();
  }

public:
  V_UP_INPUT (channel_automaton, send, T);

private:
  bool receive_precondition () const {
    return !m_queue.empty () && ioa::binding_count (&channel_automaton::receive) != 0;
  }

  T receive_effect () {
    T retval =  m_queue.front ();
    m_queue.pop ();
    return retval;
  }

  void receive_schedule () const {
    if (receive_precondition ()) {
      ioa::schedule (&channel_automaton::receive);
    }
  }

public:
  V_UP_OUTPUT (channel_automaton, receive, T);
};

#endif
