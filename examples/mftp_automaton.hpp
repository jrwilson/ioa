#ifndef __mftp_automaton_hpp__
#define	__mftp_automaton_hpp__

#include "periodic_timer.hpp"
#include "file.hpp"

#include <arpa/inet.h>
#include <cstdlib>
#include <queue>
#include <vector>
#include <cstring>
#include <math.h>

#include <iostream>

namespace mftp {

  const long FRAGMENT_TIME_MICROSECONDS = 1000;
  const long REQUEST_TIME_SECONDS = 1;
  const long ANNOUNCEMENT_TIME_SECONDS  = 7;

  class mftp_automaton :
    public ioa::automaton
  {
  private:
    enum send_state_t {
      SEND_READY,
      SEND_COMPLETE_WAIT
    };

    enum timer_state_t {
      SET_READY,
      INTERRUPT_WAIT,
    };

    ioa::self_manager<mftp_automaton> m_self;
    file m_file;
    const mfileid& m_mfileid;
    const fileid& m_fileid;
    std::vector<bool> m_their_req;
    uint32_t m_their_req_count;
    std::queue<message*> m_sendq;
    send_state_t m_send_state;
    timer_state_t m_fragment_timer_state;
    timer_state_t m_request_timer_state;
    timer_state_t m_announcement_timer_state;
  
  public:
    mftp_automaton (const file& file) :
      m_file (file),
      m_mfileid (file.get_mfileid ()),
      m_fileid (m_mfileid.get_fileid ()),
      m_their_req (m_mfileid.get_fragment_count ()),
      m_their_req_count (0),
      m_send_state (SEND_READY),
      m_fragment_timer_state (SET_READY),
      m_request_timer_state (SET_READY),
      m_announcement_timer_state (SET_READY)
    {
      ioa::automaton_manager<periodic_timer>* fragment_clock = new ioa::automaton_manager<periodic_timer> (this, ioa::make_generator<periodic_timer> (ioa::time (0, FRAGMENT_TIME_MICROSECONDS)));
      ioa::make_bind_helper (this,
			     &m_self,
			     &mftp_automaton::set_fragment_timer,
			     fragment_clock,
			     &periodic_timer::set);
      ioa::make_bind_helper (this,
			     fragment_clock,
			     &periodic_timer::interrupt,
			     &m_self,
			     &mftp_automaton::fragment_timer_interrupt);

    
      ioa::automaton_manager<periodic_timer>* request_clock = new ioa::automaton_manager<periodic_timer> (this, ioa::make_generator<periodic_timer> (ioa::time (REQUEST_TIME_SECONDS, 0)));
      ioa::make_bind_helper (this,
			     &m_self,
			     &mftp_automaton::set_request_timer,
			     request_clock,
			     &periodic_timer::set);
      ioa::make_bind_helper (this,
			     request_clock,
			     &periodic_timer::interrupt,
			     &m_self,
			     &mftp_automaton::request_timer_interrupt);
    
      ioa::automaton_manager<periodic_timer>* announcement_clock = new ioa::automaton_manager<periodic_timer> (this, ioa::make_generator<periodic_timer> (ioa::time (ANNOUNCEMENT_TIME_SECONDS, 0)));
      ioa::make_bind_helper (this,
			     &m_self,
			     &mftp_automaton::set_announcement_timer,
			     announcement_clock,
			     &periodic_timer::set);
      ioa::make_bind_helper (this,
			     announcement_clock,
			     &periodic_timer::interrupt,
			     &m_self,
			     &mftp_automaton::announcement_timer_interrupt);

      schedule ();
    }

    ~mftp_automaton () {
      while (!m_sendq.empty ()) {
	delete m_sendq.front ();
	m_sendq.pop ();
      }
    }

  private:
    bool send_precondition () const {
      return !m_sendq.empty () && m_send_state == SEND_READY;
    }

    message send_effect () {
      std::auto_ptr<message> m (m_sendq.front ());
      m_sendq.pop ();
      convert_to_network (m.get ());
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
    void receive_effect (const message& mess) {
      //message m;
      //copy_message (mess, m);

      //message* mp = &m;

      message m;
      convert_to_host(mess, m);

      //if(mess.header.message_type == REQUEST) {

        //std::cout << mess.req.start << " " << m.req.start << std::endl;
      //}
      //else {
        //std::cout << mess.frag.offset << " " << m.frag.offset << std::endl;
      //}

      switch (m.header.message_type) {
      case FRAGMENT:
      {
        // It must be a fragment from our file and the offset must be correct.
        if (m.frag.fid == m_fileid &&
            m.frag.offset < m_mfileid.get_final_length () &&
            m.frag.offset % FRAGMENT_SIZE == 0) {

          // Save the fragment.
          m_file.write_chunk (m.frag.offset, m.frag.data);

	  if (m_file.complete ()) {
	    std::string s (reinterpret_cast<char*> (m_file.get_data_ptr ()), m_mfileid.get_original_length ());
	    std::cout << s << std::endl;
	  }

          uint32_t idx = (m.frag.offset / FRAGMENT_SIZE);
          if (m_their_req[idx]) {
            // Somebody else wanted it and now has just seen it.
            m_their_req[idx] = false;
            --m_their_req_count;
          }
        }

      }
      break;

      case REQUEST:
      {
        // Request must be for our file and in range.
        if (m.req.fid == m_fileid &&
            m.req.start % FRAGMENT_SIZE == 0 &&
            m.req.stop % FRAGMENT_SIZE == 0 &&
            m.req.start < m.req.stop &&
            m.req.stop <= m_mfileid.get_final_length ()) {
          // We have it.
          for (uint32_t offset = m.req.start;
         offset < m.req.stop;
         offset += FRAGMENT_SIZE) {
            uint32_t idx = offset / FRAGMENT_SIZE;
            if (m_file.have (offset) && !m_their_req[idx]) {
        m_their_req[idx] = true;
        ++m_their_req_count;
            }
          }
        }
      }
      break;
      default:
      // Unkown message type.
      break;
      }

      schedule ();
    }

  public:

    V_UP_INPUT (mftp_automaton, receive, message);

  private:
    bool set_fragment_timer_precondition () const {
      return m_fragment_timer_state == SET_READY && m_their_req_count != 0 && ioa::bind_count (&mftp_automaton::set_fragment_timer) != 0;
    }

    void set_fragment_timer_effect () {
      m_fragment_timer_state = INTERRUPT_WAIT;
      schedule ();
    }

    UV_UP_OUTPUT (mftp_automaton, set_fragment_timer);

    void fragment_timer_interrupt_effect () {
      // Purpose is to produce a randomly selected requested fragment.
      if (m_their_req_count != 0 && m_sendq.empty()) {
	// Get a random index.
	uint32_t randy = get_random_request_index ();
	m_their_req[randy] = false;
	--m_their_req_count;

	// Get the fragment for that index.
	m_sendq.push (get_fragment (randy));
      }

      m_fragment_timer_state = SET_READY;
      schedule();
    }

    UV_UP_INPUT (mftp_automaton, fragment_timer_interrupt);

    bool set_request_timer_precondition () const {
      return m_request_timer_state == SET_READY && !m_file.complete () && ioa::bind_count (&mftp_automaton::set_request_timer) != 0;
    }

    void set_request_timer_effect () {
      m_request_timer_state = INTERRUPT_WAIT;
      schedule ();
    }

    UV_UP_OUTPUT (mftp_automaton, set_request_timer);

    void request_timer_interrupt_effect () {
      if (m_sendq.empty () && !m_file.complete ()) {

	std::pair<uint32_t, uint32_t> range = m_file.get_next_range ();
	m_sendq.push (new message (request_type (), m_fileid, range.first, range.second));
      }

      m_request_timer_state = SET_READY;
      schedule ();
    }

    UV_UP_INPUT (mftp_automaton, request_timer_interrupt);

    bool set_announcement_timer_precondition () const {
      return m_announcement_timer_state == SET_READY && ioa::bind_count (&mftp_automaton::set_announcement_timer) != 0;
    }

    void set_announcement_timer_effect () {
      m_announcement_timer_state = INTERRUPT_WAIT;
      schedule ();
    }

    UV_UP_OUTPUT (mftp_automaton, set_announcement_timer);

    void announcement_timer_interrupt_effect () {
      if (!m_file.empty () && m_sendq.empty()) {
	m_sendq.push (get_fragment (m_file.get_random_index ()));
      }
      m_announcement_timer_state = SET_READY;
      schedule();
    }

    UV_UP_INPUT (mftp_automaton, announcement_timer_interrupt);


    void convert_to_network (message* m) {
      switch (m->header.message_type) {
      case FRAGMENT:
        m->frag.fid.length = htonl (m->frag.fid.length);
        m->frag.fid.type = htonl (m->frag.fid.type);
        m->frag.offset = htonl (m->frag.offset);
	break;
      case REQUEST:
        m->req.fid.length = htonl (m->req.fid.length);
        m->req.fid.type = htonl (m->req.fid.type);
        m->req.start = htonl (m->req.start);
        m->req.stop = htonl (m->req.stop);
	break;
      default:
	// Unkown message type.
	break;
      }
      m->header.message_type = htonl (m->header.message_type);
    }

    void convert_to_host (const message& mess, message& m) {
      m.header.message_type = ntohl (mess.header.message_type);
      switch (m.header.message_type) {
      case FRAGMENT:
	memcpy (m.frag.fid.hash, mess.frag.fid.hash, HASH_SIZE);
        m.frag.fid.length = ntohl (mess.frag.fid.length);
        m.frag.fid.type = ntohl (mess.frag.fid.type);
        m.frag.offset = ntohl (mess.frag.offset);
        memcpy(m.frag.data, mess.frag.data, 512);
	break;
      case REQUEST:
	memcpy (m.req.fid.hash, mess.req.fid.hash, HASH_SIZE);
        m.req.fid.length = ntohl (mess.req.fid.length);
        m.req.fid.type = ntohl (mess.req.fid.type);
        m.req.start = ntohl (mess.req.start);
        m.req.stop = ntohl (mess.req.stop);
	break;
      }
    }

    uint32_t get_random_request_index () {
      assert (m_their_req_count != 0);
      uint32_t rf = rand () % m_their_req.size();
      for (; !m_their_req[rf]; rf = (rf + 1) % m_their_req.size ()) { }
      return rf;
    }

    message* get_fragment (uint32_t idx) {
      uint32_t offset = idx * FRAGMENT_SIZE;
      message* mp = new message (fragment_type (), m_fileid, offset, m_file.get_data_ptr() + offset);
      return mp;
    }

    void schedule () const {
      if (send_precondition ()) {
	ioa::schedule (&mftp_automaton::send);
      }

      if (set_fragment_timer_precondition ()) {
	ioa::schedule (&mftp_automaton::set_fragment_timer);
      }

      if (set_request_timer_precondition ()) {
	ioa::schedule (&mftp_automaton::set_request_timer);
      }

      if (set_announcement_timer_precondition ()) {
	ioa::schedule (&mftp_automaton::set_announcement_timer);
      }
    }

  };

}

#endif
