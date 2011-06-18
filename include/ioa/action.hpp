#ifndef __action_hpp__
#define __action_hpp__

#include <ioa/aid.hpp>

namespace ioa {

  /* Represents the absence of a type. */
  class null_type { };

  /*
    Action categories.

    Input, outputs, and internal actions come directly from the I/O automata formalism.
    System outputs allow an automaton to request a configuration action (create, bind, unbind, destroy).
    System inputs allow an automaton to receive the result of a configuration actions.
   */
  struct input_category { };
  struct output_category { };
  struct internal_category { };
  struct system_input_category { };
  struct system_output_category { };

  /* Indicates if an input or output has an associated value. */
  struct unvalued { };
  struct valued { };

  /* Indicates if an input, output, or internal action has an associated parameter.
     Parameters are designed to solve the problem of fan-in as an input can only be composed with one output.
     We can compose an input with multiple outputs by declaring a parameter for each output.
     This idea can be extended to outputs and internal actions to capture the notion of a session where the parameter indicates that all interactions with another automaton are identified using a parameter.
     Actions can be automatically parameterized by the ID of the automaton to which they are bound.
  */
  struct unparameterized { };
  struct parameterized { };
  struct auto_parameterized { };

  /* These are helpers that define action traits. */

  struct no_value {
    typedef unvalued value_status;
    typedef null_type value_type;
  };

  template <typename T>
  struct value {
    typedef valued value_status;
    typedef T value_type;
  };

  struct no_parameter {
    typedef unparameterized parameter_status;
    typedef null_type parameter_type;
  };

  template <typename T>
  struct parameter {
    typedef parameterized parameter_status;
    typedef T parameter_type;
  };

  struct auto_parameter {
    typedef auto_parameterized parameter_status;
    typedef aid_t parameter_type;
  };

  struct input {
    typedef input_category action_category;
  };

  struct output {
    typedef output_category action_category;
  };

  struct internal : public no_value {
    typedef internal_category action_category;
  };

  struct system_input : public no_parameter {
    typedef system_input_category action_category;
  };

  struct system_output : public no_parameter {
    typedef system_output_category action_category;
  };

}

#endif
