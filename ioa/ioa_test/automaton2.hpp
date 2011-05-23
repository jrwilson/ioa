#ifndef __automaton2_hpp__
#define __automaton2_hpp__

class automaton2
{
public:
  void init () { }

private:
  bool output_ () {
    return false;
  }

  void input_ () {
  }

public:
  ioa::void_output_wrapper<automaton2, &automaton2::output_> output;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input2;

  automaton2 () :
    output (*this),
    input (*this),
    input2 (*this)
  { }
};

struct automaton2_generator
{
  typedef automaton2 result_type;

  automaton2* operator() () {
    return new automaton2 ();
  }
};

template <class T>
class instance_holder
{
private:
  T* m_instance;

public:

  typedef T result_type;

  instance_holder (T* instance) :
    m_instance (instance)
  { }

  T* operator() () {
    return m_instance;
  }
  
};

#endif
