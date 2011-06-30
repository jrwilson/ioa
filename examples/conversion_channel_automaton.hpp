#ifndef __conversion_channel_automaton_hpp__
#define __conversion_channel_automaton_hpp__

#include "mftp.hpp"
#include <ioa/ioa.hpp>

#include <queue>

class conversion_channel_automaton :
  public ioa::automaton
{
private:
  std::queue<ioa::const_shared_ptr<mftp::message> > q;
  
public:
  conversion_channel_automaton ()
  {
    schedule ();
  }

private:
  void schedule () const {
    if (pass_message_precondition ()) {
      ioa::schedule (&conversion_channel_automaton::pass_message);
    }
  }

  void receive_buffer_effect (const ioa::udp_receiver_automaton::receive_val& rv) {
    if (rv.buffer.get () != 0 && rv.buffer->size () == sizeof (mftp::message)) {
      std::auto_ptr<mftp::message> m (new mftp::message);
      memcpy (m.get (), rv.buffer->data (), rv.buffer->size ());
      if (convert_to_host (m.get ())) {
	q.push (ioa::const_shared_ptr<mftp::message> (m.release ()));
      }
    }
  }

public:
  V_UP_INPUT (conversion_channel_automaton, receive_buffer, ioa::udp_receiver_automaton::receive_val);

private:
  bool pass_message_precondition () const {
    return !q.empty ()  && ioa::binding_count (&conversion_channel_automaton::pass_message) != 0;
  }

  ioa::const_shared_ptr<mftp::message> pass_message_effect () {
    ioa::const_shared_ptr<mftp::message> m = q.front ();
    q.pop ();
    return m;
  }

public:
  V_UP_OUTPUT (conversion_channel_automaton, pass_message, ioa::const_shared_ptr<mftp::message>);

private:
  bool convert_to_host (mftp::message* m) {
    m->header.message_type = ntohl (m->header.message_type);
    switch (m->header.message_type) {
    case mftp::FRAGMENT:
      {
	m->frag.fid.length = ntohl (m->frag.fid.length);
	m->frag.fid.type = ntohl (m->frag.fid.type);
	m->frag.offset = ntohl (m->frag.offset);
	mftp::mfileid mid (m->frag.fid);
	return ((m->frag.offset % mftp::FRAGMENT_SIZE) == 0) &&
	  (m->frag.offset < mid.get_final_length ());
      }
      break;
    case mftp::REQUEST:
      {
	m->req.fid.length = ntohl (m->req.fid.length);
	m->req.fid.type = ntohl (m->req.fid.type);
	m->req.span_count = ntohl (m->req.span_count);
	if (m->req.span_count == 0 || m->req.span_count > mftp::SPANS_SIZE) {
	  return false;
	}
	mftp::mfileid mid (m->req.fid);
	for (uint32_t i = 0; i < m->req.span_count; ++i){
	  m->req.spans[i].start = ntohl (m->req.spans[i].start);
	  m->req.spans[i].stop = ntohl (m->req.spans[i].stop);
	  if (!(m->req.spans[i].start < m->req.spans[i].stop &&
		m->req.spans[i].stop <= mid.get_final_length () &&
		m->req.spans[i].start % mftp::FRAGMENT_SIZE == 0 &&
		m->req.spans[i].stop % mftp::FRAGMENT_SIZE == 0)) {
	    return false;
	  }
	}
	return true;
      }
      break;
    default:
      return false;
      break;
    }

    return false;
  }
};

#endif
