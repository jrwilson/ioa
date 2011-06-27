#ifndef __mftp_automaton_hpp__
#define	__mftp_automaton_hpp__

#include <ioa/alarm_automaton.hpp>
#include "file.hpp"

#include <arpa/inet.h>
#include <cstdlib>
#include <queue>
#include <vector>
#include <cstring>
#include <math.h>

#include <iostream>

namespace mftp {

  class mftp_automaton :
    public ioa::automaton
  {
  private:
    const ioa::time m_fragment_interval;
    const ioa::time m_request_interval;
    const ioa::time m_announcement_interval;

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
      m_fragment_interval (0, 1000), // 1000 microseconds = 1 millisecond
      m_request_interval (1, 0), // 1 second
      m_announcement_interval (7, 0), // 7 seconds
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
      ioa::automaton_manager<ioa::alarm_automaton>* fragment_clock = new ioa::automaton_manager<ioa::alarm_automaton> (this, ioa::make_generator<ioa::alarm_automaton> ());
      ioa::make_bind_helper (this,
			     &m_self,
			     &mftp_automaton::set_fragment_timer,
			     fragment_clock,
			     &ioa::alarm_automaton::set);
      ioa::make_bind_helper (this,
			     fragment_clock,
			     &ioa::alarm_automaton::alarm,
			     &m_self,
			     &mftp_automaton::fragment_timer_interrupt);

    
      ioa::automaton_manager<ioa::alarm_automaton>* request_clock = new ioa::automaton_manager<ioa::alarm_automaton> (this, ioa::make_generator<ioa::alarm_automaton> ());
      ioa::make_bind_helper (this,
			     &m_self,
			     &mftp_automaton::set_request_timer,
			     request_clock,
			     &ioa::alarm_automaton::set);
      ioa::make_bind_helper (this,
			     request_clock,
			     &ioa::alarm_automaton::alarm,
			     &m_self,
			     &mftp_automaton::request_timer_interrupt);
    
      ioa::automaton_manager<ioa::alarm_automaton>* announcement_clock = new ioa::automaton_manager<ioa::alarm_automaton> (this, ioa::make_generator<ioa::alarm_automaton> ());
      ioa::make_bind_helper (this,
			     &m_self,
			     &mftp_automaton::set_announcement_timer,
			     announcement_clock,
			     &ioa::alarm_automaton::set);
      ioa::make_bind_helper (this,
			     announcement_clock,
			     &ioa::alarm_automaton::alarm,
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

    bool send_precondition () const {
      return !m_sendq.empty () && m_send_state == SEND_READY;
    }

    message send_effect () {
      std::auto_ptr<message> m (m_sendq.front ());
      m_sendq.pop ();
      convert_to_network (m.get ());
      m_send_state = SEND_COMPLETE_WAIT;
      return *m;
    }

  public:
    V_UP_OUTPUT (mftp_automaton, send, message);

  private:
    void send_complete_effect () {
      m_send_state = SEND_READY;
    }

  public:
    UV_UP_INPUT (mftp_automaton, send_complete);

  private:
    void receive_effect (const message& mess) {

      message m;
      convert_to_host(mess, m);

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
	std::cout << "Receiving request" << std::endl;
	//Requests must be for our file.
	if (m.req.fid == m_fileid){
	  for (uint32_t sp = 0; sp < m.req.span_count; sp++){
	    // Request must be in range.
	    if (m.req.spans[sp].start % FRAGMENT_SIZE == 0 &&
		m.req.spans[sp].stop % FRAGMENT_SIZE == 0 &&
		m.req.spans[sp].start < m.req.spans[sp].stop &&
		m.req.spans[sp].stop <= m_mfileid.get_final_length ()) {
	      // We have it.
	      for (uint32_t offset = m.req.spans[sp].start;
		   offset < m.req.spans[sp].stop;
		   offset += FRAGMENT_SIZE) {
		uint32_t idx = offset / FRAGMENT_SIZE;
		if (m_file.have (offset) && !m_their_req[idx]) {
		  m_their_req[idx] = true;
		  ++m_their_req_count;
		}
	      }
	    }
	  }
	}
      }
      break;
      default:
	// Unkown message type.
	break;
      }
    }
    
  public:

    V_UP_INPUT (mftp_automaton, receive, message);

  private:
    bool set_fragment_timer_precondition () const {
      return m_fragment_timer_state == SET_READY && m_their_req_count != 0 && ioa::bind_count (&mftp_automaton::set_fragment_timer) != 0;
    }

    ioa::time set_fragment_timer_effect () {
      m_fragment_timer_state = INTERRUPT_WAIT;
      return m_fragment_interval;
    }

    V_UP_OUTPUT (mftp_automaton, set_fragment_timer, ioa::time);

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
    }

    UV_UP_INPUT (mftp_automaton, fragment_timer_interrupt);

    bool set_request_timer_precondition () const {
      return m_request_timer_state == SET_READY && !m_file.complete () && ioa::bind_count (&mftp_automaton::set_request_timer) != 0;
    }

    ioa::time set_request_timer_effect () {
      m_request_timer_state = INTERRUPT_WAIT;
      return m_request_interval;
    }

    V_UP_OUTPUT (mftp_automaton, set_request_timer, ioa::time);

    void request_timer_interrupt_effect () {
      //std::cout << "creating request message to send" << std::endl;
      if (m_sendq.empty () && !m_file.complete ()) {
	span_t spans[64];
	spans[0] = m_file.get_next_range();
	uint32_t sp_count = 1;
	bool looking = true;
	while (looking){
	  span_t range = m_file.get_next_range ();
	  if (range.start == spans[0].start || sp_count == 64){ //when we've come back to the range we started on, or we have as many ranges as we can hold
	    looking = false;
	  }
	  else {
	    spans[sp_count] = range;
	    ++sp_count;
	  }
	}
	m_sendq.push (new message (request_type (), m_fileid, sp_count, spans));
      }

      m_request_timer_state = SET_READY;
    }

    UV_UP_INPUT (mftp_automaton, request_timer_interrupt);

    bool set_announcement_timer_precondition () const {
      return m_announcement_timer_state == SET_READY && ioa::bind_count (&mftp_automaton::set_announcement_timer) != 0;
    }

    ioa::time set_announcement_timer_effect () {
      m_announcement_timer_state = INTERRUPT_WAIT;
      return m_announcement_interval;
    }

    V_UP_OUTPUT (mftp_automaton, set_announcement_timer, ioa::time);

    void announcement_timer_interrupt_effect () {
      if (!m_file.empty () && m_sendq.empty()) {
	m_sendq.push (get_fragment (m_file.get_random_index ()));
      }
      m_announcement_timer_state = SET_READY;
    }

    UV_UP_INPUT (mftp_automaton, announcement_timer_interrupt);


    void convert_to_network (message* m) {
      std::cout << "converting to network byte order" << std::endl;
      switch (m->header.message_type) {
      case FRAGMENT:
        m->frag.fid.length = htonl (m->frag.fid.length);
        m->frag.fid.type = htonl (m->frag.fid.type);
        m->frag.offset = htonl (m->frag.offset);
	break;
      case REQUEST:
        m->req.fid.length = htonl (m->req.fid.length);
        m->req.fid.type = htonl (m->req.fid.type);
	for (uint32_t i = 0; i < m->req.span_count; ++i){
	  m->req.spans[i].start = htonl (m->req.spans[i].start);
	  m->req.spans[i].stop = htonl (m->req.spans[i].stop);
	}
	m->req.span_count = htonl (m-> req.span_count);
	break;
      }
      m->header.message_type = htonl (m->header.message_type);
    }

    void convert_to_host (const message& mess, message& m) {
      std::cout << "converting to host byte order" << std::endl;
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
        m.req.span_count = ntohl (mess.req.span_count);
	for (uint32_t i = 0; i<m.req.span_count; ++i){
	  m.req.spans[i].start = ntohl (mess.req.spans[i].start);
	  m.req.spans[i].stop = ntohl (mess.req.spans[i].stop);
	}
	break;
      default:
	//Received unknown message type.
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

  };

}

#endif
