#ifndef __receipts_h__
#define __receipts_h__

#include <pthread.h>
#include <ueioa.hh>
#include <list>

class receipts {
 public:
  struct receipt {
    aid_t to;
    receipt_t receipt;
  };
  
  receipts ();
  ~receipts ();

  void push_bad_order (aid_t);
  
  void push_self_created (aid_t, aid_t);
  void push_child_created (aid_t, aid_t);
  void push_bad_descriptor (aid_t);
  
  void push_declared (aid_t);
  
  void push_output_dne (aid_t);
  void push_input_dne (aid_t);
  void push_output_unavailable (aid_t);
  void push_input_unavailable (aid_t);
  void push_composed (aid_t);
  void push_input_composed (aid_t, input_t, void*);
  void push_output_composed (aid_t, output_t, void*);
  
  void push_not_composed (aid_t);
  void push_decomposed (aid_t, aid_t, input_t, void*);
  void push_input_decomposed (aid_t, input_t, void*);
  void push_output_decomposed (aid_t, output_t, void*);
  
  void push_rescinded (aid_t);
  
  void push_automaton_dne (aid_t);
  void push_not_owner (aid_t);
  void push_child_destroyed (aid_t, aid_t);
  
  int pop (aid_t, receipt_t*);
  bool empty (aid_t);
  
  void purge_aid (aid_t);

 private:

  class receipt_entry_to_equal {
  private:
    const aid_t m_to;
  public:
    receipt_entry_to_equal (const aid_t to) :
      m_to (to) { }
    bool operator() (const receipt& receipt_entry) { return m_to == receipt_entry.to; }
  };
  
  pthread_rwlock_t m_lock;
  typedef std::list<receipt> receipt_list;
  receipt_list m_receipts;

  void push (const receipt& receipt);
};

#endif /* __receipts_h__ */
