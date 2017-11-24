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
#include <unistd.h>
using namespace std;
using namespace folly;
void sleeptimes()
{
  // int times = 10;
  // while(times -- )
  // {
  //   std::cout << "times :" << times << std::endl;
  //   usleep(1000000);
  // }
}
int main(int argc, char** argv) 
{
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
    int mutiple = 0;
    if(argc >= 4)
    {
      max_level = atoi(argv[2]);
      skip = atoi(argv[3]);
      if(argc == 5)
      {
        mutiple = atoi(argv[4]);
      }
    }
    kn::db::core::DataBase base("test");
    release_skillist("/home/kid/benckmark/kn_db/data/",base, max_level, skip);
    namedot::set_search_skiplist(base, mutiple);
    ndt::set_search_skiplist(base, mutiple);
    nts::set_search_skiplist(base, mutiple);
    sleeptimes();
    runBenchmarks();
    sleeptimes();
    return 0;
  }
  if(std::string(argv[1]) == std::string("order"))
  {
    namedot::set_search_bench_single("/home/kid/benckmark/kn_db/data/orders/000002");
  }
  else if(std::string(argv[1]) == std::string("transaction"))
  {
    int mutiple = 0;
    if(argc == 3)
    {
      mutiple = atoi(argv[2]);
    }
    ndt::set_search_bench_single("/home/kid/benckmark/kn_db/data/transactions/000002", mutiple);
  }
  else if(std::string(argv[1]) == std::string("timeshare"))
  {
    nts::set_search_bench_single("/home/kid/benckmark/kn_db/data/one_min/000002");
  }
  sleeptimes();
  runBenchmarks();
  sleeptimes();
  return 0;
}
