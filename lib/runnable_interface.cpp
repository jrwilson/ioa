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

#include <ioa/runnable_interface.hpp>
#include "unique_lock.hpp"
#include "shared_lock.hpp"

namespace ioa {

  shared_mutex runnable_interface::m_mutex;
  size_t runnable_interface::m_count = 0;

  runnable_interface::runnable_interface ()
  {
    unique_lock lock (m_mutex);
    ++m_count;
  }

  runnable_interface::~runnable_interface () {
    unique_lock lock (m_mutex);
    --m_count;
  }

  size_t runnable_interface::count () {
    shared_lock lock (m_mutex);
    return m_count;
  }

}
