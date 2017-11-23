#include <random>
#include <vector>
#include <map>
#include <string>
#include <list>
#include <iostream>
#include<time.h>
#include "rand_common.h"
#include "dot_order.h"
#include "dot_transaction.h"
#include "dot_candle.h"
#include "dot_timeshare.h"
#include "db_core/data_base.h"
using namespace std;
using namespace folly;

int main(int argc, char** argv) {
 
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if(argc <= 1)
  {
    std::cout << "please input params, test_type [sl maxlevel skip] ";
    return 0;
  }
  if(std::string(argv[1]) == std::string("sl"))
  {
    uint32_t max_level = 8;
    uint32_t skip = 4;
    if(argc == 4)
    {
      max_level = atoi(argv[2]);
      skip = atoi(argv[3]);
    }
    kn::db::core::DataBase base("test");
    release_skillist("/home/hzs/SSE/kn_db/data/",base, max_level, skip);
    namedot::set_search_skiplist(base);
    nts::set_search_skiplist(base);
    //segmentation fault
    ndt::set_search_skiplist(base);
    runBenchmarks();
    return 0;
  }
  if(std::string(argv[1]) == std::string("order"))
  {
    namedot::set_search_bench_single("/home/hzs/SSE/kn_db/data/orders/000002");
  }
  else if(std::string(argv[1]) == std::string("transaction"))
  {
    ndt::set_search_bench_single("/home/hzs/SSE/kn_db/data/transactions/000002");
  }
  else if(std::string(argv[1]) == std::string("timeshare"))
  {
    nts::set_search_bench_single("/home/hzs/SSE/kn_db/data/one_min/000002");
  }
  runBenchmarks();
  return 0;
}
