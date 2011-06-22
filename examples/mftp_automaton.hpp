#ifndef __mftp_automaton_hpp__
#define	__mftp_automaton_hpp__

#include "periodic_timer.hpp"
#include "file.hpp"

#include <cstdlib>
#include <queue>
#include <vector>
#include <string.h>
#include <math.h>

#include <iostream>

class mftp_automaton :
  public ioa::automaton
{
private:
  enum send_state {
    SEND_READY,
    SEND_COMPLETE_WAIT
  };

  std::auto_ptr<ioa::self_helper<mftp_automaton> > m_self;
  File m_file;
  std::queue<message> my_req;
  std::vector<bool> their_req;
  uint32_t their_req_count;
  std::vector<bool> have;
  uint32_t have_count;
  std::vector<bool> valid;
  std::queue<message> sendq;
  send_state m_send_state;

public:
  mftp_automaton (const File& file,
		  const bool have_it) :
    m_self (new ioa::self_helper<mftp_automaton> ()),
    m_file (file),
    their_req (m_file.m_fragment_count),
    their_req_count (0),
    have (m_file.m_fragment_count),
    have_count (have_it ? m_file.m_fragment_count : 0),
    valid (m_file.m_fragment_count),
    m_send_state (SEND_READY)
  {
    for(size_t i = 0; i < have.size(); i++) {
      have[i] = have_it;
      valid[i] = have_it;
    }

    ioa::automaton_helper<periodic_timer>* fragment_timer = new ioa::automaton_helper<periodic_timer> (this, ioa::make_generator<periodic_timer> ());
    ioa::make_bind_helper (this,
			   fragment_timer,
			   &periodic_timer::interrupt,
			   m_self.get (),
			   &mftp_automaton::fragment_interrupt);

    ioa::automaton_helper<periodic_timer>* request_timer = new ioa::automaton_helper<periodic_timer> (this, ioa::make_generator<periodic_timer> ());
    ioa::make_bind_helper (this,
			   request_timer,
			   &periodic_timer::interrupt,
			   m_self.get (),
			   &mftp_automaton::request_interrupt);

    schedule ();
  }

private:

  bool send_precondition () const {
    return !sendq.empty() && m_send_state == SEND_READY;
  }

  message send_effect () {
    message m = sendq.front();
    sendq.pop();
    m_send_state = SEND_COMPLETE_WAIT;
    schedule ();
    return m;
  }

public:

  V_UP_OUTPUT (mftp_automaton, send, message);

private:

  void send_complete_effect () {
    m_send_state = SEND_READY;
    schedule ();
  }

public:

  UV_UP_INPUT (mftp_automaton, send_complete);

private:

  void receive_effect (const message& m) {
    switch (m.header.message_type) {
    case FRAGMENT:
      {
	if (m.frag.fid == m_file.m_fileid &&
	    m.frag.offset < m.frag.fid.hashed_length &&
	    m.frag.offset % FRAGMENT_SIZE == 0) {
	  // It's in range and it's one of ours.
	  uint32_t idx = (m.frag.offset / FRAGMENT_SIZE);

	  // Somebody else wanted it and now has just seen it.
	  if (their_req[idx]) {
	    their_req[idx] = false;
	    --their_req_count;
	  }

	  if (!have[idx]) {
	    // We don't have it.
	    uint32_t expected_length = FRAGMENT_SIZE;
	    if (idx + 1 == m_file.m_fragment_count) {
	      expected_length = m_file.m_fileid.hashed_length - m.frag.offset;
	    }

	    if (m.frag.length == expected_length) {
	      // It has the correct size.
	      // Copy it.
	      memcpy (m_file.get_data_ptr () + m.frag.offset, m.frag.data, expected_length);
	      // Have it.
	      have[idx] = true;
	      ++have_count;
	      std::cout << "Now I have " << have_count << "/" << m_file.m_fragment_count << std::endl;
	      // TODO:  Validate it.
	    }
	  }
	}
      }
      break;
    case REQUEST:
      {
	if (m.req.fid == m_file.m_fileid &&
	    m.req.offset % FRAGMENT_SIZE == 0 &&
	    m.req.offset + m.req.length <= m.req.fid.hashed_length) {
	  // It's in range and it's one of ours.
	  // We have it.
	  for (uint32_t offset = m.req.offset;
	       offset < m.req.length;
	       offset += FRAGMENT_SIZE) {
	    uint32_t idx = offset / FRAGMENT_SIZE;
	    if (have[idx] && !their_req[idx]) {
	      their_req[idx] = true;
	      ++their_req_count;
	    }
	  }
	}
      }
      break;
    default:
      // Wonky message.
      break;
    }

    schedule ();
  }

public:

  V_UP_INPUT (mftp_automaton, receive, message);

private:

  void fragment_interrupt_effect () {
    // Purpose is to produce a randomly selected requested fragment.
    if (their_req_count != 0) {
      // Get a random index.
      uint32_t randy = get_random_index ();
      their_req[randy] = false;
      --their_req_count;

      // Get the fragment for that index.
      sendq.push (get_fragment (randy));
    }

    schedule();
  }

  UV_UP_INPUT (mftp_automaton, fragment_interrupt);

  void request_interrupt_effect () {
    // Purpose is to move from my_req to send queue.
    if (!my_req.empty ()) {
      sendq.push (my_req.front ());
      my_req.pop ();
    }

    schedule ();
  }

  UV_UP_INPUT (mftp_automaton, request_interrupt);

  // TODO:  Announce_interrupt

  bool replenish_request_queue_precondition () const {
    return have_count != m_file.m_fragment_count && my_req.empty ();
  }

  void replenish_request_queue_effect () {
    uint32_t start_idx = 0;
    uint32_t end_idx;

    while (start_idx < m_file.m_fragment_count) {
      
      // Move start until we don't have one.
      for (;
	   start_idx < m_file.m_fragment_count && have[start_idx] == true;
	   ++start_idx)
	;;

      if (start_idx < m_file.m_fragment_count) {
	for (end_idx = start_idx + 1;
	     end_idx < m_file.m_fragment_count && have[end_idx] == false;
	     ++end_idx)
	  ;;
	
	// Range is [start_idx, end_idx).
	uint32_t expected_length = (end_idx - start_idx) * FRAGMENT_SIZE;
	if (end_idx == m_file.m_fragment_count) {
	  expected_length = m_file.m_fileid.hashed_length - start_idx * FRAGMENT_SIZE;
	}
	
	my_req.push (message (request_type (), m_file.m_fileid, start_idx * FRAGMENT_SIZE, expected_length));
	
	start_idx = end_idx;
      }
    }

    schedule ();
  }

  UP_INTERNAL (mftp_automaton, replenish_request_queue);


  uint32_t get_random_index () {
    assert (their_req_count != 0);
    uint32_t rf = rand () % their_req.size();
    for (; !their_req[rf]; rf = (rf + 1) % their_req.size ()) { }
    return rf;
  }

  message get_fragment (uint32_t idx) {
    uint32_t offset = idx * FRAGMENT_SIZE;
    uint32_t expected_length = FRAGMENT_SIZE;
    if (idx + 1 == m_file.m_fragment_count) {
      expected_length = m_file.m_fileid.hashed_length - offset;
    }
    return message (fragment_type (), m_file.m_fileid, offset, expected_length, m_file.get_data_ptr() + offset);
  }

  void schedule () const {
    if (replenish_request_queue_precondition ()) {
      ioa::schedule (&mftp_automaton::replenish_request_queue);
    }
    if (send_precondition ()) {
      ioa::schedule (&mftp_automaton::send);
    }
  }

};


#endif
