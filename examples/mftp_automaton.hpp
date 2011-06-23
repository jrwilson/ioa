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

#define FRAGMENT_TIME_MICROSECONDS 1000
#define REQUEST_TIME_SECONDS 1
#define ANNOUNCEMENT_TIME_SECONDS 7

class mftp_automaton :
  public ioa::automaton
{
private:
  enum send_state {
    SEND_READY,
    SEND_COMPLETE_WAIT
  };

  std::auto_ptr<ioa::self_manager<mftp_automaton> > m_self;
  File m_file;
  std::vector<bool> their_req;
  uint32_t their_req_count;
  std::vector<bool> have;
  uint32_t have_count;
  std::vector<bool> valid;
  std::queue<message*> sendq;
  send_state m_send_state;
  uint32_t start_idx;

public:
  mftp_automaton (const File& file,
		  const bool have_it) :
    m_self (new ioa::self_manager<mftp_automaton> ()),
    m_file (file),
    their_req (m_file.m_fragment_count),
    their_req_count (0),
    have (m_file.m_fragment_count),
    have_count (have_it ? m_file.m_fragment_count : 0),
    valid (m_file.m_fragment_count),
    m_send_state (SEND_READY),
    start_idx (0)
  {
    for(size_t i = 0; i < have.size(); i++) {
      have[i] = have_it;
      valid[i] = have_it;
    }

    ioa::automaton_manager<periodic_timer>* fragment_timer = new ioa::automaton_manager<periodic_timer> (this, ioa::make_generator<periodic_timer> (ioa::time (0, FRAGMENT_TIME_MICROSECONDS)));
    ioa::make_bind_helper (this,
			   fragment_timer,
			   &periodic_timer::interrupt,
			   m_self.get (),
			   &mftp_automaton::fragment_interrupt);

    ioa::automaton_manager<periodic_timer>* request_timer = new ioa::automaton_manager<periodic_timer> (this, ioa::make_generator<periodic_timer> (ioa::time (REQUEST_TIME_SECONDS, 0)));
    ioa::make_bind_helper (this,
			   request_timer,
			   &periodic_timer::interrupt,
			   m_self.get (),
			   &mftp_automaton::request_interrupt);

    ioa::automaton_manager<periodic_timer>* announcement_timer = new ioa::automaton_manager<periodic_timer> (this, ioa::make_generator<periodic_timer> (ioa::time (ANNOUNCEMENT_TIME_SECONDS, 0)));
    ioa::make_bind_helper (this,
			   announcement_timer,
			   &periodic_timer::interrupt,
			   m_self.get (),
			   &mftp_automaton::announcement_interrupt);


    schedule ();
  }

  ~mftp_automaton () {
    while (!sendq.empty ()) {
      delete sendq.front ();
      sendq.pop ();
    }
  }

private:

  bool send_precondition () const {
    return !sendq.empty() && m_send_state == SEND_READY;
  }

  message send_effect () {
    std::cout << __func__ << std::endl;
    std::auto_ptr<message> m (sendq.front());
    sendq.pop();

    m_send_state = SEND_COMPLETE_WAIT;
    schedule ();
    return *m;
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
              /*
               * Printed out the file to make sure it was a valid transfer.
              if (have_count == m_file.m_fragment_count) {
                std::string st (m_file.get_data_ptr(), m_file.get_data_ptr() + static_cast<size_t>(m_file.m_fileid.original_length));
                std::cout << st << std::endl;
              }
              */
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
      std::cout << __func__ << std::endl;
      // Get a random index.
      uint32_t randy = get_random_request_index ();
      their_req[randy] = false;
      --their_req_count;

      // Get the fragment for that index.
      sendq.push (get_fragment (randy));
      std::cout << their_req_count << " " << sendq.size() << std::endl;
    }

    schedule();
  }

  UV_UP_INPUT (mftp_automaton, fragment_interrupt);

  void request_interrupt_effect () {
    //have_count != m_file.m_fragment_count && my_req.empty ();
    std::cout << __func__ << std::endl;
    uint32_t end_idx;

    if (sendq.empty () && have_count < m_file.m_fragment_count) {

      if (start_idx > m_file.m_fragment_count) {
        start_idx = 0;
      }
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

        message* mp = new message (request_type (), m_file.m_fileid, start_idx * FRAGMENT_SIZE, expected_length);
        sendq.push (mp);

        //sendq.push (message (request_type (), m_file.m_fileid, start_idx * FRAGMENT_SIZE, expected_length));
        start_idx = end_idx;
      }
    }
    /*
    // Purpose is to move from my_req to send queue.
    if (!my_req.empty ()) {
      std::cout << __func__ << std::endl;
      sendq.push (my_req.front ());
      my_req.pop ();
    }
    */

    schedule ();
  }

  UV_UP_INPUT (mftp_automaton, request_interrupt);

  void announcement_interrupt_effect () {
    std::cout << "In " << __func__ << std::endl;
    if (have_count > 0 && sendq.empty()) {
      std::cout << "Announcing...\n";
      sendq.push (get_fragment (get_random_index ()));
    }
  }

  UV_UP_INPUT (mftp_automaton, announcement_interrupt);

  uint32_t get_random_index () {
    uint32_t rf = rand () % m_file.m_fragment_count;
    for (; !have[rf]; rf = (rf + 1) % have.size ()) { }
    return rf;  
  }

  uint32_t get_random_request_index () {
    assert (their_req_count != 0);
    uint32_t rf = rand () % their_req.size();
    for (; !their_req[rf]; rf = (rf + 1) % their_req.size ()) { }
    return rf;
  }

  message* get_fragment (uint32_t idx) {
    uint32_t offset = idx * FRAGMENT_SIZE;
    uint32_t expected_length = FRAGMENT_SIZE;
    if (idx + 1 == m_file.m_fragment_count) {
      expected_length = m_file.m_fileid.hashed_length - offset;
    }
    message* mp = new message (fragment_type (), m_file.m_fileid, offset, expected_length, m_file.get_data_ptr() + offset);
    return mp;
  }

  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&mftp_automaton::send);
    }
  }

};


#endif
