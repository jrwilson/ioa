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

#ifndef __aid_hpp__
#define __aid_hpp__

#include <ioa/environment.hpp>

#include <stdint.h>

namespace ioa {
  
  // Automaton ID.
#ifdef ENVIRONMENT64
  typedef int64_t aid_t;
#endif
#ifdef ENVIRONMENT32
  typedef int32_t aid_t;
#endif
  
}

#endif

