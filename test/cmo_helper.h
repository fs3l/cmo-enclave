#ifndef CMO_HELPER_H
#define CMO_HELPER_H

#include <boost/test/unit_test.hpp>
#include "cmo.h"

void CMO_BOOST_CHECK(CMO_p rt, bool cond)
{
  if (!cond) {
    end_leaky_sec(rt);
    BOOST_CHECK(cond);
    begin_leaky_sec(rt);
  } else
    BOOST_CHECK(cond);
}

#endif
