#include "minunit.h"

#include "../lib/automaton_set.hpp"
#include "automaton1.hpp"

#include <vector>
#include <iostream>

static const char*
create1 ()
{
  std::cout << __func__ << std::endl;

  ioa::automaton_set as;
  automaton1 instance;

  mu_assert (!as.exists (&instance));
  ioa::aid_t aid = as.create (&instance);
  mu_assert (as.exists (&instance));
  mu_assert (as.exists (aid));

  return 0;
}

static const char*
create3 ()
{
  std::cout << __func__ << std::endl;

  ioa::automaton_set as;
  automaton1 parent_instance;
  int key1;
  automaton1 child1_instance;
  int key2;
  automaton1 child2_instance;
  std::map<void*, ioa::aid_t> keys_gold;
  std::map<void*, ioa::aid_t> keys;

  ioa::aid_t parent = as.create (&parent_instance);
  mu_assert (!as.exists (&child1_instance));
  ioa::aid_t child1 = as.create (parent, &key1, &child1_instance);
  mu_assert (as.exists (&child1_instance));
  mu_assert (as.exists (parent, &key1));
  mu_assert (as.exists (child1));
  mu_assert (!as.exists (&child2_instance));
  ioa::aid_t child2 = as.create (parent, &key2, &child2_instance);
  mu_assert (as.exists (&child2_instance));
  mu_assert (as.exists (parent, &key2));
  mu_assert (as.exists (child2));

  keys_gold.insert (std::make_pair (&key1, child1));
  keys_gold.insert (std::make_pair (&key2, child2));
  std::copy (as.keys_begin (parent), as.keys_end (parent), std::inserter (keys, keys.end ()));
  mu_assert (std::equal (keys.begin (), keys.end (), keys_gold.begin ()));

  return 0;
}

static const char*
destroy ()
{
  std::cout << __func__ << std::endl;

  ioa::automaton_set as;
  automaton1 parent_instance;
  int key1;
  automaton1 child1_instance;

  ioa::aid_t parent = as.create (&parent_instance);
  mu_assert (!as.exists (&child1_instance));
  ioa::aid_t child1 = as.create (parent, &key1, &child1_instance);
  mu_assert (as.exists (&child1_instance));
  mu_assert (as.exists (parent, &key1));
  mu_assert (as.exists (child1));

  mu_assert (as.destroy (parent, &key1) == &child1_instance);
  mu_assert (!as.exists (&child1_instance));
  mu_assert (!as.exists (parent, &key1));
  mu_assert (!as.exists (child1));

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (create1);
  mu_run_test (create3);
  mu_run_test (destroy);

  return 0;
}
