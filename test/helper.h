#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE __BASE_FILE__

#include <stdint.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "utils.h"

using namespace std;

vector<int32_t> gen_sequence(int32_t len)
{
  vector<int32_t> result(len);
  iota(result.begin(), result.end(), 0);
  return result;
}

vector<int32_t> gen_random_sequence(int32_t len)
{
  vector<int32_t> result(gen_sequence(len));
  fisher_yates_shuffle(result.data(), len);
  return result;
}
