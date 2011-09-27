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

#ifndef __thread_hpp__
#define __thread_hpp__

#include <pthread.h>
#include <memory>
#include <cassert>

namespace ioa {

  class thread_arg_interface
  {
  public:
    virtual ~thread_arg_interface () { }
    virtual void operator() () = 0;
  };

  template <class I>
  class thread_arg :
    public thread_arg_interface
  {
  private:
    I& m_ref;
    void (I::*m_member_function) ();
    
  public:
    thread_arg (I& ref,
		void (I::*member_function) ()) :
      m_ref (ref),
      m_member_function (member_function)
    { }

    void operator() () {
      (m_ref.*m_member_function) ();
    }
  };

  void* thread_func (void* a);
    
  class thread
  {
  private:
    pthread_t m_thread;
    pthread_attr_t m_attr;
    std::auto_ptr<thread_arg_interface> m_thread_arg;
    
    // No copying.
    thread (const thread&) { }

  public:
    template <class I>
    thread (I& ref,
	    void (I::*func) ()) :
      m_thread_arg (new thread_arg<I> (ref, func))
    {
      int r;
      
      r = pthread_attr_init (&m_attr);
      assert (r == 0);
      r = pthread_create (&m_thread, &m_attr, thread_func, m_thread_arg.get ());
      assert (r == 0);
    }

    ~thread ();
    void join ();
    pthread_t get_id () const;
  };
  
}

#endif
