#ifndef __action_hpp__
#define __action_hpp__

#include <set>
#include "automaton.hpp"

namespace ioa {

  class abstract_action {
  public:
    virtual abstract_automaton* get_automaton() const = 0;
    virtual bool is_action(const abstract_automaton*,
			   const void*) const = 0;
  };
  
  template<class T>
  class abstract_output_action : public abstract_action {
  public:
    virtual T operator()() = 0;
  };

  template<class Instance, class Member, class T>
  class output_action : public abstract_output_action<T> {
  private:
    automaton<Instance>* m_automaton;
    Member Instance::*m_member;
  public:
    output_action(automaton<Instance>* automaton,
		  Member Instance::*member)
      : m_automaton(automaton),
	m_member(member) { }

    T operator()() {
      Instance* instance = m_automaton->get_typed_instance();
      Member& ref = (*instance).*m_member;
      return ref();
    }

    bool is_action(const abstract_automaton* aa,
		   const void* output) const {
      Instance* instance = m_automaton->get_typed_instance();
      const Member& ref = (*instance).*m_member;
      return m_automaton == aa && &ref == output;
    }

    abstract_automaton* get_automaton() const {
      return m_automaton;
    }
  };

  template<class T>
  class abstract_input_action : public abstract_action {
  public:
    virtual void operator()(const T) = 0;
    virtual bool is_action_owner(const abstract_automaton*,
				 const void*,
				 const abstract_automaton*) const = 0;
    virtual void decompose() = 0;
  };

  template<class Instance, class Member, class T, class Callback>
  class input_action : public abstract_input_action<T> {
  private:
    automaton<Instance>* m_automaton;
    Member Instance::*m_member;
    const abstract_automaton* m_owner;
    Callback m_callback;

  public:
    input_action(automaton<Instance>* automaton,
  		 Member Instance::*member,
		 const abstract_automaton* owner,
		 Callback& callback)
      : m_automaton(automaton),
  	m_member(member),
	m_owner(owner),
	m_callback(callback) { }
    void operator() (const T t) {
      Instance* instance = m_automaton->get_typed_instance();
      Member& ref = (*instance).*m_member;
      ref(t);
    }

    bool is_action(const abstract_automaton* aa,
		   const void* input) const {
      Instance* instance = m_automaton->get_typed_instance();
      Member& ref = (*instance).*m_member;
      return m_automaton == aa && &ref == input;
    }

    bool is_action_owner(const abstract_automaton* aa,
			 const void* input,
			 const abstract_automaton* owner) const {
      return is_action(aa, input) && m_owner == owner;
    }

    abstract_automaton* get_automaton() const {
      return m_automaton;
    }

    void decompose() {
      m_callback.decompose();
    }
  };

  template <class T>
  struct abstract_input_action_compare {
    bool operator()(const abstract_input_action<T>* x,
		    const abstract_input_action<T>* y) {
      return x->get_automaton() < y->get_automaton();
    }
  };

  class abstract_macro_action {
  public:
    virtual void operator()() const = 0;
    virtual bool is_output(const abstract_automaton* aa,
			   const void* output) const = 0;
    virtual bool is_input(const abstract_automaton* aa) const = 0;
    virtual bool is_input(const abstract_automaton* aa,
			  const void* input) const = 0;
    virtual bool is_input(const abstract_automaton* aa,
			  const void* input,
			  const abstract_automaton* owner) const = 0;
    virtual bool empty() const = 0;
    virtual void decompose(const abstract_automaton* aa,
			   const void* input,
			   const abstract_automaton* owner) = 0;
 };

  template<class T>
  class macro_action : public abstract_macro_action {
  private:
    abstract_output_action<T>* m_output_action;
    typedef std::set<abstract_input_action<T>*, abstract_input_action_compare<T> > set_type;
    typedef typename set_type::const_iterator const_iterator;
    set_type m_input_actions;
  public:
    macro_action(abstract_output_action<T>* output_action)
      : m_output_action(output_action) { }
    ~macro_action() {
      for(const_iterator pos = m_input_actions.begin();
	  pos != m_input_actions.end();
	  ++pos) {
	(*pos)->decompose();
	delete (*pos);
      }
      delete m_output_action;
    }

    bool is_output(const abstract_automaton* aa,
		   const void* output) const {
      return m_output_action->is_action (aa, output);
    }

    bool is_input(const abstract_automaton* aa) const {
      for(const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
	if((*pos)->get_automaton() == aa) {
	  return true;
	}
      }
      return false;
    }

    bool is_input(const abstract_automaton* aa,
		  const void* input) const {
      for(const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
	if((*pos)->is_action(aa, input)) {
	  return true;
	}
      }
      return false;
    }

    bool is_input(const abstract_automaton* aa,
		  const void* input,
		  const abstract_automaton* owner) const {
      for(const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
	if((*pos)->is_action_owner(aa, input, owner)) {
	  return true;
	}
      }
      return false;
    }

    void add_input(abstract_input_action<T>* input_action) {
      m_input_actions.insert(input_action);
    }

    bool empty() const {
      return m_input_actions.empty();
    }

    void decompose(const abstract_automaton* aa,
		   const void* input,
		   const abstract_automaton* owner) {
      const_iterator pos;
      for(pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
	if((*pos)->is_action_owner(aa, input, owner)) {
	  break;
	}
      }

      if(pos != m_input_actions.end()) {
	(*pos)->decompose();
	delete (*pos);
	m_input_actions.erase(pos);
      }
    }


    void operator()() const {
      bool locked_output = false;
      abstract_automaton* ao = m_output_action->get_automaton();

      // Acquire locks (in order).
      for(const_iterator pos = m_input_actions.begin();
	  pos != m_input_actions.end();
	  ++pos) {
	abstract_automaton* ai = (*pos)->get_automaton();
	if(!locked_output && ao < ai) {
	  ao->lock();
	  locked_output = true;
	}
	ai->lock();
      }

      // Execute.
      T t = (*m_output_action)();
      for(const_iterator pos = m_input_actions.begin();
	  pos != m_input_actions.end();
	  ++pos) {
	(*(*pos))(t);
      }

      // Release locks.
      locked_output = false;
      for(const_iterator pos = m_input_actions.begin();
	  pos != m_input_actions.end();
	  ++pos) {
	abstract_automaton* ai = (*pos)->get_automaton();
	if(!locked_output && ao < ai) {
	  ao->unlock();
	  locked_output = true;
	}
	ai->unlock();
      }
    }
  };

}

#endif
