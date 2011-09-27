/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __generator_interface_hpp__
#define __generator_interface_hpp__

namespace ioa {

  /*
    A generator produces an automaton when invoked.
    The result should be delete'able.
    Generators are owned by exactly one object at a time and will only be invoked once.  
  */

  class automaton;

  class generator_interface
  {
  public:
    virtual ~generator_interface () { }
    virtual automaton* operator() () = 0;
  };

  template <class I>
  class typed_generator_interface :
    public generator_interface
  {
  public:
    virtual ~typed_generator_interface () { }
    virtual I* operator() () = 0;
  };

}

#endif
