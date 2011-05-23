#ifndef __automaton_helper_hpp__
#define __automaton_helper_hpp__

namespace ioa {

  namespace automaton_dfa {

    typedef enum {
      START,
      CREATE_SENT,
      CREATE_RECV1,
      CREATE_RECV2,
      DESTROY_SENT,
      STOP,
      ERROR,
      STATE_COUNT,
    } state_type;

    typedef enum {
      // User symbols.
      CREATE,
      DESTROY,
      // System symbols.
      AUTOMATON_CREATED,
      INSTANCE_EXISTS,
      AUTOMATON_DESTROYED,
      TARGET_AUTOMATON_DNE,
      DESTROYER_NOT_CREATOR,
      SYMBOL_COUNT,
    } symbol_type;

    static const state_type transition[STATE_COUNT][SYMBOL_COUNT] =
      {
	/* CREATE                      DESTROY                      AUTOMATON_CREATED            INSTANCE_EXISTS       AUTOMATON_DESTROYED   TARGET_AUTOMATON_DNE  DESTROYER_NOT_CREATOR */
	/* START */        { automaton_dfa::CREATE_SENT, automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
	/* CREATE_SENT */  { automaton_dfa::ERROR,       automaton_dfa::CREATE_RECV2, automaton_dfa::CREATE_RECV1, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
	/* CREATE_RECV1 */ { automaton_dfa::ERROR,       automaton_dfa::DESTROY_SENT, automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
	/* CREATE_RECV2 */ { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::DESTROY_SENT, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
	/* DESTROY_SENT */ { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::STOP,  automaton_dfa::ERROR, automaton_dfa::ERROR  },
	/* STOP */         { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
	/* ERROR */        { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR }
      };

  };

  template <class T, class G>
  class automaton_helper
  {
  public:
    typedef G generator;
    typedef typename G::result_type instance;

  private:
    automaton_dfa::state_type m_state;
    const T* m_t;
    G m_generator;
    ioa::automaton_handle<instance> m_handle;

  public:

    automaton_helper (const T* t,
		      G generator) :
      m_state (automaton_dfa::START),
      m_t (t),
      m_generator (generator)
    {

    }

    void create () {
      switch (m_state) {
      case automaton_dfa::START:
	ioa::scheduler.create (m_t, m_generator, *this);
	break;
      default:
	break;
      }
      m_state = automaton_dfa::transition[m_state][automaton_dfa::CREATE];
    }

    void destroy () {
      switch (m_state) {
      case automaton_dfa::CREATE_RECV1:
	ioa::scheduler.destroy (m_t, m_handle, *this);
	break;
      default:
	break;
      }
      m_state = automaton_dfa::transition[m_state][automaton_dfa::DESTROY];
    }
  
    void automaton_created (const ioa::automaton_handle<instance>& automaton) {
      switch (m_state) {
      case automaton_dfa::CREATE_SENT:
	m_handle = automaton;
	break;
      case automaton_dfa::CREATE_RECV2:
	m_handle = automaton;
	ioa::scheduler.destroy (m_t, m_handle, *this);
	break;
      default:
	break;
      }
      m_state = automaton_dfa::transition[m_state][automaton_dfa::AUTOMATON_CREATED];
    }
  
    void instance_exists (const instance* /* */) {
      m_state = automaton_dfa::transition[m_state][automaton_dfa::INSTANCE_EXISTS];
    }
  
    void automaton_destroyed () {
      m_state = automaton_dfa::transition[m_state][automaton_dfa::AUTOMATON_DESTROYED];
    }

    void target_automaton_dne () {
      m_state = automaton_dfa::transition[m_state][automaton_dfa::TARGET_AUTOMATON_DNE];
    }

    void destroyer_not_creator () {
      m_state = automaton_dfa::transition[m_state][automaton_dfa::DESTROYER_NOT_CREATOR];
    }
  
    automaton_dfa::state_type get_state () const {
      return m_state;
    }
  };

}

#endif
