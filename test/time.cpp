#include "minunit.h"

#include <ioa/time.hpp>
#include <iostream>

static const char*
default_ctor ()
{
  std::cout << __func__ << std::endl;

  ioa::time t;

  mu_assert (t.get_sec () == 0);
  mu_assert (t.get_usec () == 0);

  return 0;
}

static const char*
ctor ()
{
  std::cout << __func__ << std::endl;

  ioa::time t1 (1, 500000);
  mu_assert (t1.get_sec () == 1);
  mu_assert (t1.get_usec () == 500000);

  ioa::time t2 (-1, 500000);
  mu_assert (t2.get_sec () == 0);
  mu_assert (t2.get_usec () == -500000);

  ioa::time t3 (1, -500000);
  mu_assert (t3.get_sec () == 0);
  mu_assert (t3.get_usec () == 500000);

  ioa::time t4 (-1, -500000);
  mu_assert (t4.get_sec () == -1);
  mu_assert (t4.get_usec () == -500000);

  ioa::time t5 (1, 5000000);
  mu_assert (t5.get_sec () == 6);
  mu_assert (t5.get_usec () == 0);

  ioa::time t6 (-1, 5000000);
  mu_assert (t6.get_sec () == 4);
  mu_assert (t6.get_usec () == 0);

  ioa::time t7 (1, -5000000);
  mu_assert (t7.get_sec () == -4);
  mu_assert (t7.get_usec () == 0);

  ioa::time t8 (-1, -5000000);
  mu_assert (t8.get_sec () == -6);
  mu_assert (t8.get_usec () == 0);

  return 0;
}

static const char*
copy_ctor ()
{
  std::cout << __func__ << std::endl;

  ioa::time t1 (1, 500000);
  ioa::time t2 (t1);
  mu_assert (t2.get_sec () == 1);
  mu_assert (t2.get_usec () == 500000);

  return 0;
}

static const char*
copy_struct ()
{
  std::cout << __func__ << std::endl;

  struct timeval t1;
  t1.tv_sec = 1;
  t1.tv_usec = 500000;
  ioa::time t2 (t1);
  mu_assert (t2.get_sec () == 1);
  mu_assert (t2.get_usec () == 500000);

  return 0;
}

static const char*
op_assign ()
{
  std::cout << __func__ << std::endl;

  ioa::time t1 (1, 500000);
  ioa::time t2 (2, 300000);
  t2 = t1;
  mu_assert (t2.get_sec () == 1);
  mu_assert (t2.get_usec () == 500000);

  return 0;
}

static const char*
op_equal ()
{
  std::cout << __func__ << std::endl;

  ioa::time t1 (1, 500000);
  ioa::time t2 (2, 300000);
  ioa::time t3 (1, 500000);
  mu_assert (t1 == t3);
  mu_assert (!(t1 == t2));

  return 0;
}

static const char*
op_plus_equal ()
{
  std::cout << __func__ << std::endl;

  ioa::time t1 (1, 500500);
  ioa::time t2 (-2, -300300);

  ioa::time t3 (t1);
  t3 += t1;
  mu_assert (t3.get_sec () == 3);
  mu_assert (t3.get_usec () == 1000);

  ioa::time t4 (t1);
  t4 += t2;

  mu_assert (t4.get_sec () == 0);
  mu_assert (t4.get_usec () == -799800);

  ioa::time t5 (t2);
  t5 += t1;
  mu_assert (t5.get_sec () == 0);
  mu_assert (t5.get_usec () == -799800);

  ioa::time t6 (t2);
  t6 += t2;
  mu_assert (t6.get_sec () == -4);
  mu_assert (t6.get_usec () == -600600);

  return 0;
}

static const char*
op_minus ()
{
  std::cout << __func__ << std::endl;

  ioa::time t1 (5, 789999);
  ioa::time t2 (1, 500000);
  ioa::time t3 (-1, -500000);
  ioa::time t4 (-3, -800000);

  ioa::time t;

  t = t1 - t1;
  mu_assert (t.get_sec () == 0);
  mu_assert (t.get_usec () == 0);

  t = t1 - t2;
  mu_assert (t.get_sec () == 4);
  mu_assert (t.get_usec () == 289999);

  t = t1 - t3;
  mu_assert (t.get_sec () == 7);
  mu_assert (t.get_usec () == 289999);

  t = t1 - t4;
  mu_assert (t.get_sec () == 9);
  mu_assert (t.get_usec () == 589999);

  t = t2 - t1;
  mu_assert (t.get_sec () == -4);
  mu_assert (t.get_usec () == -289999);

  t = t2 - t2;
  mu_assert (t.get_sec () == 0);
  mu_assert (t.get_usec () == 0);

  t = t2 - t3;
  mu_assert (t.get_sec () == 3);
  mu_assert (t.get_usec () == 0);

  t = t2 - t4;
  mu_assert (t.get_sec () == 5);
  mu_assert (t.get_usec () == 300000);

  t = t3 - t1;
  mu_assert (t.get_sec () == -7);
  mu_assert (t.get_usec () == -289999);

  t = t3 - t2;
  mu_assert (t.get_sec () == -3);
  mu_assert (t.get_usec () == 0);

  t = t3 - t3;
  mu_assert (t.get_sec () == 0);
  mu_assert (t.get_usec () == 0);

  t = t3 - t4;
  mu_assert (t.get_sec () == 2);
  mu_assert (t.get_usec () == 300000);

  t = t4 - t1;
  mu_assert (t.get_sec () == -9);
  mu_assert (t.get_usec () == -589999);

  t = t4 - t2;
  mu_assert (t.get_sec () == -5);
  mu_assert (t.get_usec () == -300000);

  t = t4 - t3;
  mu_assert (t.get_sec () == -2);
  mu_assert (t.get_usec () == -300000);

  t = t4 - t4;
  mu_assert (t.get_sec () == 0);
  mu_assert (t.get_usec () == 0);

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (default_ctor);
  mu_run_test (ctor);
  mu_run_test (copy_ctor);
  mu_run_test (copy_struct);
  mu_run_test (op_assign);
  mu_run_test (op_equal);
  mu_run_test (op_plus_equal);
  mu_run_test (op_minus);

  return 0;
}
