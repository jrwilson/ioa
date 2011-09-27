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

#ifndef __bfs_automaton_hpp__
#define __bfs_automaton_hpp__

/*
  AsynchBFS Automaton
  Distributed Algorithms, p. 502.
*/

#include <ioa/ioa.hpp>

#include <iostream>
#include <set>
#include <map>
#include <queue>

class asynch_bfs_automaton:
  public ioa::automaton {

private:

  typedef std::queue<size_t> distq;

  size_t m_i;
  size_t m_i0;
  std::set<size_t> m_nbrs;

  size_t m_dist;
  size_t m_parent;
  std::map<size_t, distq *> m_send;

  bool send_precondition(size_t j) const {
    distq * q = m_send.find(j)->second;
    return (ioa::binding_count (&asynch_bfs_automaton::send, j) != 0) && !q->empty();
  }

  size_t send_effect (size_t j) {
    size_t retval = m_send[j]->front();
    m_send[j]->pop();
    return retval;
  }

  void send_schedule (size_t) const {
    schedule ();
  }

  void receive_effect (const size_t& m, size_t j){
    if (m+1 < m_dist){
      std::cout << m_i << ": (parent=" << m_parent << ",dist=" << m_dist << ") -> ";
      m_dist = m+1;
      m_parent = j;
      std::cout << "(parent=" << m_parent << ",dist=" << m_dist << ")" << std::endl;
      for (std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos!= m_nbrs.end(); pos++){
	if (*pos != j){
	  m_send[*pos]->push(m_dist);
	}
      }
    }
  }

  void receive_schedule (size_t) const {
    schedule ();
  }

  void schedule () const {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); pos++){
      if (send_precondition (*pos)) {
	ioa::schedule(&asynch_bfs_automaton::send, *pos);
      }
    }
  }

public:
  asynch_bfs_automaton (size_t i, size_t i0, const std::set<size_t> &nbrs) :
    m_i(i),
    m_i0(i0),
    m_nbrs(nbrs),
    m_dist(-1),
    m_parent(-1)
  {
    if (m_i == m_i0) m_dist = 0;

    for (std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); pos++){
      m_send.insert(make_pair(*pos, new distq()));
      if (i==i0){
	m_send[*pos]->push(0);
      }
    }
    schedule ();
  }

  ~asynch_bfs_automaton()
  {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); pos++){
      delete m_send[*pos];
    }
  }

  V_P_OUTPUT (asynch_bfs_automaton, send, size_t, size_t);
  V_P_INPUT (asynch_bfs_automaton, receive, size_t, size_t);

};

#endif
