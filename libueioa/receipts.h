#ifndef __receipts_h__
#define __receipts_h__

#include <pthread.h>
#include <ueioa.h>

typedef struct {
  aid_t to;
  receipt_t receipt;
} receipt_entry_t;

class Receipts {
 public:
  Receipts ();
  ~Receipts ();

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
  
  pthread_rwlock_t m_lock;
  typedef std::list<receipt_entry_t> ReceiptList;
  ReceiptList m_receipts;

  void push (receipt_entry_t& receipt);
};

#endif /* __receipts_h__ */
