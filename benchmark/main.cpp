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
  if(argc > 1 && std::string(argv[1]) == std::string("sl"))
  {
    uint32_t max_level = 8;
    uint32_t skip = 4;
    if(argc == 4)
    {
      max_level = atoi(argv[2]);
      skip = atoi(argv[3]);
    }
    kn::db::core::DataBase base("test");
    release_skillist("/media/psf/Home/Documents/kn_db/kn_db/data",base, max_level, skip);
    namedot::set_search_skiplist(base);
    runBenchmarks();
    return 0;
  }
  else
  {
    namedot::set_search_bench_single("/media/psf/Home/Documents/kn_db/kn_db/data/orders/000002");
    runBenchmarks();
    return 0;
  }
  
  //namedot::set_search_bench_single();
  //template search test
  //namedot::set_rand_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_orders_last");
  //namedot::set_search_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_orders_last");

  // ndt::set_rand_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_trans_last");
  // ndt::set_search_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_trans_last");

  // ndc::set_rand_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_5min_candle_last", "000002");
  // ndc::set_search_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_5min_candle_last", "000002");

  // nts::set_rand_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_time_share_last", "000002");
  // nts::set_search_bench_single("/media/psf/Home/Documents/newlibinstall/test_file/1020_time_share_last", "000002");

  //ndt::set_rand_bench_single<std::vector<int64_t>>("/media/psf/Home/Documents/newlibinstall/test_file/trans.plan");
  //ndc::set_rand_bench_single<std::vector<std::string>>("/media/psf/Home/Documents/newlibinstall/test_file/5mincan.plan", "000002");
  //nts::set_rand_bench_single<std::vector<std::string>>("/media/psf/Home/Documents/newlibinstall/test_file/timeshare.plan", "000002");
  //runBenchmarks();
  // std::vector<int> vec;
  // struct timespec time_start={0, 0},time_end={0, 0};
  // std::map<int, std::string> seq_add;
  // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time_start);
  
  
  // for (size_t i = 0; i < 1000000; ++i) {
  //   seq_add.emplace(i,std::to_string(i));
  // }
  // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time_end);
  // std::cout << "sec:" << time_end.tv_sec-time_start.tv_sec <<" ns:" << time_end.tv_nsec-time_start.tv_nsec << std::endl;
   return 0;
}
