#include "minunit.h"

#include <ioa/binding_set.hpp>
#include "automaton1.hpp"

#include <iostream>

static const char*
bind ()
{
  std::cout << __func__ << std::endl;

  ioa::binding_set bs;

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> handle1 (1);
  ioa::action_executor<automaton1, automaton1::uv_up_output_action> output_action (instance1, mutex1, handle1, &automaton1::uv_up_output);

  automaton1 instance2;
  ioa::mutex mutex2;
  ioa::automaton_handle<automaton1> handle2 (2);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action1 (instance2, mutex2, handle2, &automaton1::uv_up_input);

  ioa::aid_t owner = 10;
  int key1;

  mu_assert (!bs.exists (owner, &key1));
  mu_assert (!bs.bound (input_action1));
  mu_assert (!bs.bound (output_action, input_action1.get_aid ()));
  mu_assert (bs.begin (output_action) == bs.end (output_action));
  mu_assert (bs.keys (owner).empty ());
  mu_assert (bs.keys (output_action.get_aid ()).empty ());
  mu_assert (bs.keys (input_action1.get_aid ()).empty ());
  bs.bind (owner, &key1, output_action, input_action1);
  mu_assert (bs.exists (owner, &key1));
  mu_assert (bs.bound (input_action1));
  mu_assert (bs.bound (output_action, input_action1.get_aid ()));
  mu_assert (bs.begin (output_action) != bs.end (output_action));
  mu_assert (**bs.begin (output_action) == input_action1);
  mu_assert (++(bs.begin (output_action)) == bs.end (output_action));

  ioa::binding_set::key_set_type keys;
  keys.insert (std::make_pair (owner, &key1));
  mu_assert (bs.keys (owner) == keys);
  mu_assert (bs.keys (output_action.get_aid ()) == keys);
  mu_assert (bs.keys (input_action1.get_aid ()) == keys);

  return 0;
}

static const char*
bind3 ()
{
  std::cout << __func__ << std::endl;

  ioa::binding_set bs;

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> handle1 (1);
  ioa::action_executor<automaton1, automaton1::uv_up_output_action> output_action (instance1, mutex1, handle1, &automaton1::uv_up_output);

  automaton1 instance2;
  ioa::mutex mutex2;
  ioa::automaton_handle<automaton1> handle2 (2);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action1 (instance2, mutex2, handle2, &automaton1::uv_up_input);

  automaton1 instance3;
  ioa::mutex mutex3;
  ioa::automaton_handle<automaton1> handle3 (3);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action2 (instance3, mutex3, handle3, &automaton1::uv_up_input);

  automaton1 instance4;
  ioa::mutex mutex4;
  ioa::automaton_handle<automaton1> handle4 (4);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action3 (instance4, mutex4, handle4, &automaton1::uv_up_input);

  ioa::aid_t owner = 10;
  int key1;
  int key2;
  int key3;

  bs.bind (owner, &key3, output_action, input_action3);
  bs.bind (owner, &key1, output_action, input_action1);
  bs.bind (owner, &key2, output_action, input_action2);

  ioa::binding_set::iterator pos = bs.begin (output_action);
  mu_assert (pos != bs.end (output_action));
  mu_assert (**pos == input_action1);
  ++pos;
  mu_assert (pos != bs.end (output_action));
  mu_assert (**pos == input_action2);
  ++pos;
  mu_assert (pos != bs.end (output_action));
  mu_assert (**pos == input_action3);
  ++pos;
  mu_assert (pos == bs.end (output_action));

  {
    ioa::binding_set::key_set_type keys;
    keys.insert (std::make_pair (owner, &key1));
    keys.insert (std::make_pair (owner, &key2));
    keys.insert (std::make_pair (owner, &key3));
    mu_assert (bs.keys (owner) == keys);
  }
  {
    ioa::binding_set::key_set_type keys;
    keys.insert (std::make_pair (owner, &key1));
    keys.insert (std::make_pair (owner, &key2));
    keys.insert (std::make_pair (owner, &key3));
    mu_assert (bs.keys (output_action.get_aid ()) == keys);
  }
  {
    ioa::binding_set::key_set_type keys;
    keys.insert (std::make_pair (owner, &key1));
    mu_assert (bs.keys (input_action1.get_aid ()) == keys);
  }
  {
    ioa::binding_set::key_set_type keys;
    keys.insert (std::make_pair (owner, &key2));
    mu_assert (bs.keys (input_action2.get_aid ()) == keys);
  }
  {
    ioa::binding_set::key_set_type keys;
    keys.insert (std::make_pair (owner, &key3));
    mu_assert (bs.keys (input_action3.get_aid ()) == keys);
  }

  return 0;
}

static const char*
unbind ()
{
  std::cout << __func__ << std::endl;

  ioa::binding_set bs;

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> handle1 (1);
  ioa::action_executor<automaton1, automaton1::uv_up_output_action> output_action (instance1, mutex1, handle1, &automaton1::uv_up_output);

  automaton1 instance2;
  ioa::mutex mutex2;
  ioa::automaton_handle<automaton1> handle2 (2);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action1 (instance2, mutex2, handle2, &automaton1::uv_up_input);

  ioa::aid_t owner = 10;
  int key1;

  bs.bind (owner, &key1, output_action, input_action1);
  bs.unbind (owner, &key1);
  mu_assert (!bs.exists (owner, &key1));
  mu_assert (!bs.bound (input_action1));
  mu_assert (!bs.bound (output_action, input_action1.get_aid ()));
  mu_assert (bs.begin (output_action) == bs.end (output_action));
  mu_assert (bs.keys (owner).empty ());
  mu_assert (bs.keys (output_action.get_aid ()).empty ());
  mu_assert (bs.keys (input_action1.get_aid ()).empty ());

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (bind);
  mu_run_test (bind3);
  mu_run_test (unbind);

  return 0;
}
