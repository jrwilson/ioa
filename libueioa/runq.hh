#ifndef __runq_h__
#define __runq_h__

#include <pthread.h>
#include <list>
#include <ueioa.h>
#include <cassert>

class runq {
public:
  
  typedef enum {
    SYSTEM_INPUT,
    SYSTEM_OUTPUT,
    ALARM_INPUT,
    READ_INPUT,
    WRITE_INPUT,
    FREE_INPUT,
    OUTPUT,
    INTERNAL,
  } runnable_type_t;
  
  class runnable {
  public:
    runnable_type_t type;
    aid_t aid;
    void* param;
    union {
      struct {
	aid_t caller_aid;
	input_t free_input;
	bid_t bid;
      } free_input;
      struct {
	output_t output;
      } output;
      struct {
	internal_t internal;
      } internal;
    };

    bool operator== (const runnable& runnable) const
    {
      if (type != runnable.type) {
	return false;
      }
      if (aid != runnable.aid) {
	return false;
      }
      if (param != runnable.param) {
	return false;
      }
      
      switch (type) {
      case SYSTEM_INPUT:
      case SYSTEM_OUTPUT:
      case ALARM_INPUT:
      case READ_INPUT:
      case WRITE_INPUT:
	return true;
	break;
      case FREE_INPUT:
	return
	  free_input.caller_aid == runnable.free_input.caller_aid &&
	  free_input.free_input == runnable.free_input.free_input &&
	  free_input.bid == runnable.free_input.bid;
	break;
      case OUTPUT:
	return output.output == runnable.output.output;
	break;
      case INTERNAL:
	return internal.internal == runnable.internal.internal;
	break;
      }
      
      /* Not reached. */
      assert (0);
      return true;
    }
  };
  
  runq ();
  ~runq ();
  
  size_t size ();
  bool empty ();
  
  void insert_system_input (aid_t);
  void insert_system_output (aid_t);
  void insert_alarm_input (aid_t);
  void insert_read_input (aid_t);
  void insert_write_input (aid_t);
  void insert_free_input (aid_t, aid_t, input_t, bid_t);
  void insert_output (aid_t, output_t, void*);
  void insert_internal (aid_t, internal_t, void*);
  
  runnable pop ();
  void purge_aid (aid_t);
  void purge_aid_param (aid_t, void*);
  
 private:
  pthread_cond_t m_cond;
  pthread_mutex_t m_mutex;
  typedef std::list<runnable> runnable_list;
  runnable_list m_runnables;
  
  void push (const runnable& runnable);
};

#endif /* __runq_h__ */
