#ifndef __ast_automaton_hpp__
#define __ast_automaton_hpp__

#include <ioa.hpp>

#include <set>
#include <map>

typedef enum {
  SEARCH_MESSAGE
} search_t;

class ast_automaton :
  public ioa::dispatching_automaton
{

private:
  typedef enum {
    SEARCH,
    NOSEARCH
  } mode_t;
  size_t m_parent;
  bool m_reported;
  std::map<size_t, mode_t> m_send;
  std::set<size_t> m_nbrs;
  size_t m_i;
  size_t m_i0;

  bool parent_precondition () const {
    return m_parent != size_t(-1) && !m_reported && parent.is_bound ();
  }

  V_UP_OUTPUT (ast_automaton, parent, size_t) {
    std::pair<bool, size_t> retval;
    if(parent_precondition()) {
      m_reported = true;
      retval = std::make_pair(true, m_parent);
    }
    schedule();
    return retval;
  }

  bool send_precondition (size_t j) {
    return m_send[j] == SEARCH && send.is_bound (j);
  }

  //Unvalued, parameterized output.
  //@Param: ast_automaton is the class, send the name of the method
  //size_t is the parameter type, and j is the parameter.
  //this refers to what automaton is being sent to
  V_P_OUTPUT (ast_automaton, send, search_t, size_t, j) {
    std::pair<bool, search_t> retval;

    if(send_precondition(j)) {
      m_send[j] = NOSEARCH;
      retval = std::make_pair(true, SEARCH_MESSAGE);
    }

    schedule();
    return retval;
  }

  //Unvalued, parameterized input.
  //@Param: ast_automaton is the class, send the name of the method
  //int is the parameter type, and sender is the parameter.
  //this refers to what automaton we are receiving from
  V_P_INPUT (ast_automaton, receive, search_t, ignored, size_t, j) {
    if(m_i != m_i0 && m_parent == size_t(-1)) {
      m_parent = j;

      for(std::set<size_t>::const_iterator k = m_nbrs.begin ();
        k != m_nbrs.end ();
        ++k) {
        if(*k != j) {
          m_send[*k] = SEARCH;
        }
      }
    }

    schedule();
  }

  void schedule () {
    if (parent_precondition ()) {
      ioa::scheduler::schedule (this, &ast_automaton::parent);
    }
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin();
        pos != m_nbrs.end();
        ++pos) {
      if (send_precondition (*pos)) {
        ioa::scheduler::schedule (this, &ast_automaton::send, *pos);
      }
    }
  }

public:
  ast_automaton (size_t i, size_t i0, const std::set<size_t>& nbrs) :
    m_parent(-1),
    m_reported(false),
    m_nbrs(nbrs),
    m_i(i),
    m_i0(i0),
    ACTION (ast_automaton, parent),
    ACTION (ast_automaton, send),
    ACTION (ast_automaton, receive)
  {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin();
        pos != m_nbrs.end();
        ++pos) {
      if(i == i0) {
        m_send[*pos] = SEARCH;
      }
      else {
        m_send[*pos] = NOSEARCH;
      }
    }
  }

  void init () {
    schedule();
  }

};

#endif
