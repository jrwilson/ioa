#ifndef __automata_hh__
#define __automata_hh__

#include <ueioa.h>

#include "runq.h"
#include "ioq.hh"
#include "receipts.h"
#include "buffers.hh"

class automata {
 public:
  automata ();
  ~automata ();

  void create_automaton (Receipts&, Runq&, const descriptor_t*, const void*);
  
  void system_input_exec (Receipts&, Runq&, buffers&, aid_t);
  void system_output_exec (Receipts&, Runq&, ioq&, buffers&, aid_t);
  void alarm_input_exec (buffers&, aid_t);
  void read_input_exec (buffers&, aid_t);
  void write_input_exec (buffers&, aid_t);
  void free_input_exec (buffers&, aid_t, aid_t, input_t, bid_t);
  void output_exec (buffers&, aid_t, output_t, void*);
  void internal_exec (aid_t, internal_t, void*);

  aid_t get_current_aid ();
  bool system_output_exists (aid_t);
  bool alarm_input_exists (aid_t);
  bool read_input_exists (aid_t);
  bool write_input_exists (aid_t);
  bool free_input_exists (aid_t, input_t);
  bool output_exists (aid_t, output_t, void*);
  bool internal_exists (aid_t, internal_t, void*);

 private:
  struct automaton_entry {
    aid_t aid;
    aid_t parent;
    void* state;
    pthread_mutex_t* lock;
    input_t system_input;
    output_t system_output;
    input_t alarm_input;
    input_t read_input;
    input_t write_input;
  };
  
  class automaton_entry_aid_equal {
  private:
    const aid_t m_aid;
  public:
    automaton_entry_aid_equal (const aid_t aid) :
      m_aid (aid) { }
    bool operator () (const automaton_entry& x) const { return m_aid == x.aid; }
  };

  class automaton_entry_parent_equal {
  private:
    const aid_t m_aid;
  public:
    automaton_entry_parent_equal (const aid_t aid) :
      m_aid (aid) { }
    bool operator () (const automaton_entry& x) const { return m_aid == x.parent; }
  };

  struct input_entry {
    aid_t aid;
    input_t input;
    input_entry (aid_t _aid, input_t _input) :
      aid (_aid),
      input (_input) { }
    bool operator< (const input_entry& x) const {
      if (aid != x.aid) {
	return aid < x.aid;
      }
      return input < x.input;
    }
  };
  
  struct output_entry {
    aid_t aid;
    output_t output;
    output_entry (aid_t _aid, output_t _output) :
      aid (_aid),
      output (_output) { }
    bool operator< (const output_entry& x) const {
      if (aid != x.aid) {
	return aid < x.aid;
      }
      return output < x.output;
    }
  };
  
  struct internal_entry {
    aid_t aid;
    internal_t internal;
    internal_entry (aid_t _aid, internal_t _internal) :
      aid (_aid),
      internal (_internal) { }
    bool operator< (const internal_entry& x) const {
      if (aid != x.aid) {
	return aid < x.aid;
      }
      return internal < x.internal;
    }
  };
  
  struct param_entry {
    aid_t aid;
    void* param;
    param_entry (aid_t _aid, void* _param) :
      aid (_aid),
      param (_param) { }
    bool operator< (const param_entry& x) const {
      if (aid != x.aid) {
	return aid < x.aid;
      }
      return param < x.param;
    }
  };
  
  struct composition_entry {
    aid_t aid;
    aid_t out_aid;
    output_t output;
    void* out_param;
    aid_t in_aid;
    input_t input;
    void* in_param;
    bool operator< (const composition_entry& x) const {
      if (out_aid != x.out_aid) {
	return out_aid < x.out_aid;
      }
      if (output != x.output) {
	return output < x.output;
      }
      if (out_param != x.out_param) {
	return out_param < x.out_param;
      }
      if (in_aid != x.in_aid) {
	return in_aid < x.in_aid;
      }
      if (input != x.input) {
	return input < x.input;
      }
      return in_param < x.in_param;
    }
  };

  class composition_entry_in_aid_input_in_param_equal {
  private:
    const aid_t m_in_aid;
    const input_t m_input;
    const void* m_in_param;
  public:
    composition_entry_in_aid_input_in_param_equal (const aid_t in_aid, const input_t input, const void* in_param) :
      m_in_aid (in_aid),
      m_input (input),
      m_in_param (in_param) { }
    bool operator () (const composition_entry& x) const
    {
      return
  	m_in_aid == x.in_aid &&
  	m_input == x.input &&
  	m_in_param == x.in_param;
    }
  };

  class composition_entry_out_aid_output_out_param_in_aid_equal {
  private:
    const aid_t m_out_aid;
    const output_t m_output;
    const void* m_out_param;
    const aid_t m_in_aid;
  public:
    composition_entry_out_aid_output_out_param_in_aid_equal (const aid_t out_aid, const output_t output, const void* out_param, const aid_t in_aid) :
      m_out_aid (out_aid),
      m_output (output),
      m_out_param (out_param),
      m_in_aid (in_aid) { }
    bool operator () (const composition_entry& x) const
    {
      return
  	m_out_aid == x.out_aid &&
  	m_output == x.output &&
  	m_out_param == x.out_param &&
  	m_in_aid == x.in_aid;
    }
  };

  class free_state_lock {
  public:
    void operator() (const automaton_entry& x) const {
      free (x.state);
      pthread_mutex_destroy (x.lock);
      free (x.lock);
    }
  };
    
  pthread_rwlock_t m_lock;
  pthread_key_t m_current_aid;
  aid_t m_next_aid;

  typedef std::list<automaton_entry> automaton_list;
  automaton_list m_automaton_entries;

  typedef std::set<input_entry> input_list;
  input_list m_free_input_entries;
  input_list m_input_entries;
  typedef std::set<output_entry> output_list;
  output_list m_output_entries;
  typedef std::set<internal_entry> internal_list;
  internal_list m_internal_entries;
  typedef std::set<param_entry> param_list;
  param_list m_param_entries;

  typedef std::set<composition_entry> composition_list;
  composition_list m_composition_entries;

  void set_current_aid (aid_t aid);

  void create (Receipts& receipts, Runq& runq, const aid_t parent, const descriptor_t* descriptor, const void* arg);
  void declare (Receipts& receipts, Runq& runq, const aid_t aid, void* param);
  void compose (Receipts& receipts, Runq& runq, aid_t aid, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param);
  void decompose (Receipts& receipts, Runq& runq, aid_t aid, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param);
  void rescind (Receipts& receipts, Runq& runq, aid_t aid, void* param);
  void destroy_r (Receipts& receipts, Runq& runq, buffers& buffers, aid_t aid);
  void destroy (Receipts& receipts, Runq& runq, buffers& buffers, aid_t aid, aid_t target);

  class output_lock {
  private:
    bool m_output_done;
    const automata& m_automata;
    const aid_t m_out_aid;
    const output_t m_output;
    const void* m_out_param;
    const automaton_list::const_iterator& m_out_pos;
  public:
    output_lock (const automata& automata, const aid_t& out_aid, const output_t& output, const void* out_param, const automaton_list::const_iterator& out_pos) :
      m_output_done (false),
      m_automata (automata),
      m_out_aid (out_aid),
      m_output (output),
      m_out_param (out_param),
      m_out_pos (out_pos) { }
    void operator() (const composition_entry& x) const {
      if (m_out_aid == x.out_aid &&
	  m_output == x.output &&
	  m_out_param == x.out_param) {
	if (!m_output_done && m_out_aid < x.in_aid) {
	  pthread_mutex_lock (m_out_pos->lock);
	}
	automaton_list::const_iterator pos = std::find_if (m_automata.m_automaton_entries.begin (),
							   m_automata.m_automaton_entries.end (),
							   automaton_entry_aid_equal (x.in_aid));
	pthread_mutex_lock (pos->lock);
      }
    }
  };

  class output_execute {
  private:
    automata& m_automata;
    buffers& m_buffers;
    const aid_t m_out_aid;
    const output_t m_output;
    const void* m_out_param;
    const bid_t m_bid;
  public:
    output_execute (automata& automata, buffers& buffers, const aid_t& out_aid, const output_t& output, const void* out_param, const bid_t& bid) :
      m_automata (automata),
      m_buffers (buffers),
      m_out_aid (out_aid),
      m_output (output),
      m_out_param (out_param),
      m_bid (bid) { }
    void operator() (const composition_entry& x) const {
      if (m_out_aid == x.out_aid &&
	  m_output == x.output &&
	  m_out_param == x.out_param) {
	automaton_list::const_iterator pos = std::find_if (m_automata.m_automaton_entries.begin (),
							   m_automata.m_automaton_entries.end (),
							   automaton_entry_aid_equal (x.in_aid));
	m_automata.set_current_aid (x.in_aid);
	m_buffers.change_owner (x.in_aid, m_bid);

	x.input (pos->state, x.in_param, m_bid);
	m_buffers.change_owner (-1, m_bid);
	m_automata.set_current_aid (-1);
      }
    }
  };

  class output_unlock {
  private:
    bool m_output_done;
    const automata& m_automata;
    const aid_t m_out_aid;
    const output_t m_output;
    const void* m_out_param;
    const automaton_list::const_iterator& m_out_pos;
  public:
    output_unlock (const automata& automata, const aid_t& out_aid, const output_t& output, const void* out_param, const automaton_list::const_iterator& out_pos) :
      m_output_done (false),
      m_automata (automata),
      m_out_aid (out_aid),
      m_output (output),
      m_out_param (out_param),
      m_out_pos (out_pos) { }
    void operator() (const composition_entry& x) const {
      if (m_out_aid == x.out_aid &&
	  m_output == x.output &&
	  m_out_param == x.out_param) {
	if (!m_output_done && m_out_aid < x.in_aid) {
	  pthread_mutex_unlock (m_out_pos->lock);
	}
	automaton_list::const_iterator pos = std::find_if (m_automata.m_automaton_entries.begin (),
							   m_automata.m_automaton_entries.end (),
							   automaton_entry_aid_equal (x.in_aid));
	pthread_mutex_unlock (pos->lock);
      }
    }
  };


};

#endif /* __automata_hh__ */
