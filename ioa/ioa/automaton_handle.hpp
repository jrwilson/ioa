#ifndef __automaton_handle_hpp__
#define __automaton_handle_hpp__

namespace ioa {

  // TODO:  Put this somewhere logical.
  class automaton_interface {
  public:
    virtual ~automaton_interface () { }
  };

  // Forware declaration.
  class system;

  typedef int aid_t;

  template <class T>
  class automaton_handle
  {
  private:
    aid_t m_aid;

  public:
    automaton_handle () :
      m_aid (-1)
    { }

  private:
    // Only the system can call this constructor.
    automaton_handle (aid_t a) :
      m_aid (a)
    { }

    friend class system;

  public:

    aid_t aid () const {
      return m_aid;
    }

    // We need the ability to fabricate handles to test bindings.
#ifdef AUTOMATON_HANDLE_SETTER
    void set_aid (aid_t a) {
      m_aid = a;
    }
#endif

    bool operator== (const automaton_handle& handle) const {
      return m_aid == handle.m_aid;
    }

  };

}

#endif
