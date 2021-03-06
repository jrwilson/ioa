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

#include "minunit.h"

#include "automaton2.hpp"
#include <ioa/ioa.hpp>
#include <iostream>
#include <ioa/simple_scheduler.hpp>

static bool goal_reached;

class uv_up_uv_up_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_up_uv_up_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_up_output, input, &automaton2::uv_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_up_uv_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_up_uv_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_up_uv_p_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_up_uv_p_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_up_output, input, &automaton2::uv_p_input, 37);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_up_uv_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_up_uv_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_up_uv_ap_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_up_uv_ap_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_up_output, input, &automaton2::uv_ap_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_up_uv_ap ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_up_uv_ap_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_p_uv_up_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_p_uv_up_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_p_output, 37, input, &automaton2::uv_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_p_uv_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_p_uv_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_p_uv_p_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_p_uv_p_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_p_output, 37, input, &automaton2::uv_p_input, 85);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_p_uv_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_p_uv_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_p_uv_ap_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_p_uv_ap_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_p_output, 37, input, &automaton2::uv_ap_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_p_uv_ap ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_p_uv_ap_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_ap_uv_up_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_ap_uv_up_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_ap_output, input, &automaton2::uv_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_ap_uv_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_ap_uv_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_ap_uv_p_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_ap_uv_p_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_ap_output, input, &automaton2::uv_p_input, 38);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_ap_uv_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_ap_uv_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_ap_uv_ap_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  uv_ap_uv_ap_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::uv_ap_output, input, &automaton2::uv_ap_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_ap_uv_ap ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<uv_ap_uv_ap_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_up_v_up_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_up_v_up_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_up_output, input, &automaton2::v_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_up_v_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_up_v_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_up_v_p_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_up_v_p_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_up_output, input, &automaton2::v_p_input, 37);
    add_observable (m_helper);
  };
};

static const char*
bind_v_up_v_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_up_v_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_up_v_ap_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_up_v_ap_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_up_output, input, &automaton2::v_ap_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_up_v_ap ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_up_v_ap_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_p_v_up_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_p_v_up_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_p_output, 37, input, &automaton2::v_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_p_v_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_p_v_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_p_v_p_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_p_v_p_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_p_output, 37, input, &automaton2::v_p_input, 85);
    add_observable (m_helper);
  };
};

static const char*
bind_v_p_v_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_p_v_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_p_v_ap_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_p_v_ap_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_p_output, 37, input, &automaton2::v_ap_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_p_v_ap ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_p_v_ap_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_ap_v_up_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_ap_v_up_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_ap_output, input, &automaton2::v_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_ap_v_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_ap_v_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_ap_v_p_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_ap_v_p_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_ap_output, input, &automaton2::v_p_input, 85);
    add_observable (m_helper);
  };
};

static const char*
bind_v_ap_v_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_ap_v_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_ap_v_ap_automaton :
  public ioa::automaton,
  public ioa::observer
{
private:
  ioa::binding_manager_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->get_state () == ioa::binding_manager_interface::BOUND) {
      goal_reached = true;
    }
  }

public:  
  v_ap_v_ap_automaton () {
    ioa::automaton_manager<automaton2>* output = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    ioa::automaton_manager<automaton2>* input = new ioa::automaton_manager<automaton2> (this, ioa::make_allocator<automaton2> ());
    m_helper = ioa::make_binding_manager (this, output, &automaton2::v_ap_output, input, &automaton2::v_ap_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_ap_v_ap ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<v_ap_v_ap_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

const char*
all_tests ()
{
  mu_run_test (bind_uv_up_uv_up);
  mu_run_test (bind_uv_up_uv_p);
  mu_run_test (bind_uv_up_uv_ap);
  mu_run_test (bind_uv_p_uv_up);
  mu_run_test (bind_uv_p_uv_p);
  mu_run_test (bind_uv_p_uv_ap);
  mu_run_test (bind_uv_ap_uv_up);
  mu_run_test (bind_uv_ap_uv_p);
  mu_run_test (bind_uv_ap_uv_ap);
  mu_run_test (bind_v_up_v_up);
  mu_run_test (bind_v_up_v_p);
  mu_run_test (bind_v_up_v_ap);
  mu_run_test (bind_v_p_v_up);
  mu_run_test (bind_v_p_v_p);
  mu_run_test (bind_v_p_v_ap);
  mu_run_test (bind_v_ap_v_up);
  mu_run_test (bind_v_ap_v_p);
  mu_run_test (bind_v_ap_v_ap);

  return 0;
}
 
