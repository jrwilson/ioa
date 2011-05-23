#ifndef __parameter_handle_hpp__
#define __parameter_handle_hpp__

namespace ioa {

  // Forward declaration.
  class automaton_record_interface;

  typedef int pid_t;

  template <class T>
  class parameter_handle
  {
  private:
    pid_t m_pid;

  public:
    parameter_handle () :
      m_pid (-1)
    { }

  private:
    // Only the system can call this constructor.
    parameter_handle (pid_t p) :
      m_pid (p)
    { }

    friend class automaton_record_interface;

  public:

    pid_t pid () const {
      return m_pid;
    }

  };

}

#endif
