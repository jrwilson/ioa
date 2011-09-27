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

#ifndef __output_exec_runnable_hpp__
#define __output_exec_runnable_hpp__

namespace ioa {

  class output_exec_runnable :
    public action_runnable_interface
  {
  private:
    std::auto_ptr<output_executor_interface> m_exec;
    
  public:
    output_exec_runnable (const output_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() (model_interface& model) {
      model.execute (*m_exec);
    }

    const action_executor_interface& get_action () const {
      return *m_exec;
    }

  };

}

#endif
