#include <cstdlib>
#include <iostream>

#include <ioa.hpp>
#include <ioa/simple_scheduler.hpp>

// class file_automaton :
//   public ioa::dispatching_automaton
// {
// private:
//   int m_count;

//   UP_INTERNAL (file_automaton, increment) {
//     ++m_count;
//     std::cout << m_count << std::endl;
//     if (m_count < 10) {
//       ioa::scheduler.schedule (this, &file_automaton::increment);
//     }
//   }

// public:
//   file_automaton ()
//     : m_count (0),
//       ACTION (file_automaton, increment)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &file_automaton::increment);
//   }
// };

class vfs_automaton :
  public ioa::dispatching_automaton
{
private:

  UV_EVENT (vfs_automaton, open) {
    std::cout << __func__ << std::endl;
  }

public:
  vfs_automaton () :
    ACTION (vfs_automaton, open)
  { }

  void init () { }
};

class file_reader :
  public ioa::dispatching_automaton
{
private:
  typedef ioa::automaton_helper<file_reader, vfs_automaton> vfs_helper_type;
  vfs_helper_type* vfs_helper;

public:
  file_reader ()
  { }

  void init () {
    vfs_helper = new vfs_helper_type (this, ioa::make_generator<vfs_automaton> ());
  }
};

int
main () {
  ioa::scheduler::run (ioa::make_generator<file_reader> ());
  return 0; 
}
