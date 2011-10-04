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

#ifndef __tcp_ring_automaton_hpp__
#define __tpc_ring_automaton_hpp__

#include <ioa/ioa.hpp>
#include <iostream>

// In seconds.
#define CONNECTION_TIMEOUT 10

class tcp_ring_automaton :
  public ioa::automaton,
  private ioa::observer
{
private:
  ioa::handle_manager<tcp_ring_automaton> m_self;
  const ioa::inet_address m_recv_address;
  const ioa::inet_address m_send_address;
  ioa::automaton_manager<ioa::tcp_acceptor_automaton>* m_acceptor;
  ioa::automaton_manager<ioa::tcp_connection_automaton>* m_predecessor;
  bool m_accept_flag;
  ioa::automaton_manager<ioa::tcp_connection_automaton>* m_successor;
  ioa::automaton_manager<ioa::tcp_connector_automaton>* m_connector;
  bool m_successor_connected;
  bool m_clear_to_send;
  std::queue<std::string> m_send;
  std::queue<std::string> m_recv;

public:
  tcp_ring_automaton (const ioa::inet_address& recv_address,
		     const ioa::inet_address& send_address) :
    m_self (ioa::get_aid ()),
    m_recv_address (recv_address),
    m_send_address (send_address)
  {
    schedule ();
  }

private:
  void observe (ioa::observable*) {
    // Something related to our connections changed.
    schedule ();
  }

  void schedule () const {
    if (create_acceptor_precondition ()) {
      ioa::schedule (&tcp_ring_automaton::create_acceptor);
    }
    if (create_predecessor_precondition ()) {
      ioa::schedule (&tcp_ring_automaton::create_predecessor);
    }
    if (create_successor_precondition ()) {
      ioa::schedule (&tcp_ring_automaton::create_successor);
    }
    if (accept_precondition (m_acceptor)) {
      ioa::schedule (&tcp_ring_automaton::accept, m_acceptor);
    }
    if (connect_precondition ()) {
      ioa::schedule (&tcp_ring_automaton::connect);
    }
    if (send_successor_precondition (m_successor)) {
      ioa::schedule (&tcp_ring_automaton::send_successor, m_successor);
    }
    if (receive_precondition ()) {
      ioa::schedule (&tcp_ring_automaton::receive);
    }
  }

  bool create_acceptor_precondition () const {
    return m_acceptor == 0;
  }

  void create_acceptor_effect () {
    std::cout << "Listening on " << m_recv_address.address_str () << ":" << m_recv_address.port () << std::endl;
    m_acceptor = ioa::make_automaton_manager (this, ioa::make_allocator<ioa::tcp_acceptor_automaton> (m_recv_address));
    ioa::make_binding_manager (this,
    			       &m_self, &tcp_ring_automaton::accept, m_acceptor,
    			       m_acceptor, &ioa::tcp_acceptor_automaton::accept);

    ioa::make_binding_manager (this, 
    			       m_acceptor, &ioa::tcp_acceptor_automaton::error,
    			       &m_self, &tcp_ring_automaton::acceptor_error, m_acceptor);
  }

  void create_acceptor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_ring_automaton, create_acceptor);

  void acceptor_error_effect (const int& err,
			      ioa::automaton_manager<ioa::tcp_acceptor_automaton>* a) {
    assert (a == m_acceptor);
    std::cout << "Accepting failed...retrying in " << CONNECTION_TIMEOUT << " seconds" << std::endl;
    ioa::schedule_after (&tcp_ring_automaton::retry_acceptor, ioa::time (CONNECTION_TIMEOUT, 0));
  }

  void acceptor_error_schedule (ioa::automaton_manager<ioa::tcp_acceptor_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_ring_automaton, acceptor_error, int, ioa::automaton_manager<ioa::tcp_acceptor_automaton>*);

  bool retry_acceptor_precondition () const {
    return m_acceptor != 0;
  }

  void retry_acceptor_effect () {
    m_acceptor->destroy ();
    m_acceptor = 0;
    if (m_predecessor != 0) {
      m_predecessor->destroy ();
      m_predecessor = 0;
    }
    m_accept_flag = false;
  }
  
  void retry_acceptor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_ring_automaton, retry_acceptor);

  bool create_predecessor_precondition () const {
    return m_predecessor == 0;
  }

  void create_predecessor_effect () {
    m_predecessor = ioa::make_automaton_manager (this, ioa::make_allocator<ioa::tcp_connection_automaton> ());
    add_observable (m_predecessor);
    ioa::make_binding_manager (this,
    			       m_predecessor, &ioa::tcp_connection_automaton::receive,
    			       &m_self, &tcp_ring_automaton::receive_predecessor, m_predecessor);
    ioa::make_binding_manager (this,
    			       m_predecessor, &ioa::tcp_connection_automaton::connected,
    			       &m_self, &tcp_ring_automaton::predecessor_connected, m_predecessor);
    ioa::make_binding_manager (this,
    			       m_predecessor, &ioa::tcp_connection_automaton::error,
    			       &m_self, &tcp_ring_automaton::predecessor_error, m_predecessor);

    m_accept_flag = false;
  }

  void create_predecessor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_ring_automaton, create_predecessor);

  void predecessor_connected_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>* p) {
    assert (m_predecessor == p);
    std::cout << "Predecessor connected" << std::endl;
  }

  void predecessor_connected_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (tcp_ring_automaton, predecessor_connected, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  void predecessor_error_effect (const int& val,
				 ioa::automaton_manager<ioa::tcp_connection_automaton>* p) {
    assert (m_predecessor == p);
    std::cout << "Predecessor disconnected" << std::endl;
    m_predecessor->destroy ();
    m_predecessor = 0;
    m_accept_flag = false;
  }

  void predecessor_error_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_ring_automaton, predecessor_error, int, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  bool create_successor_precondition () const {
    return m_successor == 0;
  }

  void create_successor_effect () {
    m_successor = ioa::make_automaton_manager (this, ioa::make_allocator<ioa::tcp_connection_automaton> ());
    add_observable (m_successor);
    ioa::make_binding_manager (this,
			       &m_self, &tcp_ring_automaton::send_successor, m_successor,
    			       m_successor, &ioa::tcp_connection_automaton::send);
    ioa::make_binding_manager (this,
    			       m_successor, &ioa::tcp_connection_automaton::send_complete,
    			       &m_self, &tcp_ring_automaton::send_complete, m_successor);
    ioa::make_binding_manager (this,
    			       m_successor, &ioa::tcp_connection_automaton::connected,
    			       &m_self, &tcp_ring_automaton::successor_connected, m_successor);
    ioa::make_binding_manager (this,
    			       m_successor, &ioa::tcp_connection_automaton::error,
    			       &m_self, &tcp_ring_automaton::successor_error, m_successor);
  }

  void create_successor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_ring_automaton, create_successor);

  void successor_connected_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>* s) {
    assert (s == m_successor);
    std::cout << "Succesor connected" << std::endl;
    m_successor_connected = true;
    m_clear_to_send = true;
  }

  void successor_connected_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (tcp_ring_automaton, successor_connected, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  void successor_error_effect (const int& val,
			       ioa::automaton_manager<ioa::tcp_connection_automaton>* s) {
    assert (s == m_successor);
    std::cout << "Succesor disconnected" << std::endl;
    m_successor->destroy ();
    m_successor = 0;
    m_connector->destroy ();
    m_connector = 0;
    m_successor_connected = false;
  }

  void successor_error_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_ring_automaton, successor_error, int, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  bool accept_precondition (ioa::automaton_manager<ioa::tcp_acceptor_automaton>* a) const {
    return a == m_acceptor &&
      !m_accept_flag &&
      m_predecessor != 0 &&
      m_predecessor->get_state () == ioa::automaton_manager_interface::CREATED &&
      ioa::binding_count (&tcp_ring_automaton::accept, a) != 0;
  }

  ioa::automaton_handle<ioa::tcp_connection_automaton> accept_effect (ioa::automaton_manager<ioa::tcp_acceptor_automaton>*) {
    m_accept_flag = true;
    return m_predecessor->get_handle ();
  }

  void accept_schedule (ioa::automaton_manager<ioa::tcp_acceptor_automaton>*) const {
    schedule ();
  }

  V_P_OUTPUT (tcp_ring_automaton, accept, ioa::automaton_handle<ioa::tcp_connection_automaton>, ioa::automaton_manager<ioa::tcp_acceptor_automaton>*);

  bool connect_precondition () const {
    return m_connector == 0 &&
      m_successor != 0 &&
      m_successor->get_state () == ioa::automaton_manager_interface::CREATED;
  }

  void connect_effect () {
    std::cout << "Connecting to " << m_send_address.address_str () << ":" << m_send_address.port () << std::endl;
    m_connector = ioa::make_automaton_manager (this, ioa::make_allocator<ioa::tcp_connector_automaton> (m_send_address, m_successor->get_handle ()));

    ioa::make_binding_manager (this, 
			       m_connector, &ioa::tcp_connector_automaton::error,
			       &m_self, &tcp_ring_automaton::connector_error, m_connector);
  }

  void connect_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_ring_automaton, connect);

  void connector_error_effect (const int& err,
			       ioa::automaton_manager<ioa::tcp_connector_automaton>* c) {
    assert (c == m_connector);
    std::cout << "Connection failed...retrying in " << CONNECTION_TIMEOUT << " seconds" << std::endl;
    ioa::schedule_after (&tcp_ring_automaton::retry_connector, ioa::time (CONNECTION_TIMEOUT, 0));
  }

  void connector_error_schedule (ioa::automaton_manager<ioa::tcp_connector_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_ring_automaton, connector_error, int, ioa::automaton_manager<ioa::tcp_connector_automaton>*);

  bool retry_connector_precondition () const {
    return m_connector != 0;
  }

  void retry_connector_effect () {
    m_connector->destroy ();
    m_connector = 0;
  }
  
  void retry_connector_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_ring_automaton, retry_connector);

  bool send_successor_precondition (ioa::automaton_manager<ioa::tcp_connection_automaton>* s) const {
    return s == m_successor && m_successor_connected && m_clear_to_send && !m_send.empty () && ioa::binding_count (&tcp_ring_automaton::send_successor, s) != 0;
  }

  std::string send_successor_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>*)  {
    m_clear_to_send = false;
    std::string retval = m_send.front ();
    m_send.pop ();
    return retval;
  }

  void send_successor_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  V_P_OUTPUT (tcp_ring_automaton, send_successor, std::string, ioa::automaton_manager<ioa::tcp_connection_automaton>*);
  
  void send_complete_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>* s) {
    if (s == m_successor) {
      m_clear_to_send = true;
    }
  }

  void send_complete_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (tcp_ring_automaton, send_complete, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  void receive_predecessor_effect (const std::string& val,
				   ioa::automaton_manager<ioa::tcp_connection_automaton>* p) {
    if (p == m_predecessor) {
      m_recv.push (val);
    }
  }

  void receive_predecessor_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }
  
  V_P_INPUT (tcp_ring_automaton, receive_predecessor, std::string, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  void send_effect (const std::string& val) {
    if (m_successor_connected) {
      m_send.push (val);
    }
  }

  void send_schedule () const {
    schedule ();
  }
public:
  V_UP_INPUT (tcp_ring_automaton, send, std::string);

private:
  bool receive_precondition () const {
    return !m_recv.empty () && ioa::binding_count (&tcp_ring_automaton::receive) != 0;
  }

  std::string receive_effect () {
    std::string retval = m_recv.front ();
    m_recv.pop ();
    return retval;
  }

  void receive_schedule () const {
    schedule ();
  }
public:
  V_UP_OUTPUT (tcp_ring_automaton, receive, std::string);
};

#endif
