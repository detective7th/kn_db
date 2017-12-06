#include <random>
#include <vector>
#include <map>
#include <string>
#include <list>
#include <iostream>
#include <time.h>
#include <gflags/gflags.h>
#include "rand_common.h"
#include "dot_order.h"
#include "dot_transaction.h"
#include "dot_candle.h"
#include "dot_timeshare.h"
#include "db_core/data_base.h"

using namespace std;
using namespace folly;

DEFINE_string(data_path, "./data", "data path");
DEFINE_string(test_type, "sl", "test type");
DEFINE_uint32(max_level, 8, "sl max level");
DEFINE_uint32(skip, 4, "sl skip");

int main(int argc, char** argv)
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if(FLAGS_test_type == "sl")
    {
          kn::db::core::DataBase base("test");
          release_skillist(FLAGS_data_path, base, FLAGS_max_level, FLAGS_skip);
          namedot::set_search_skiplist(base, FLAGS_data_path);
          nts::set_search_skiplist(base, FLAGS_data_path);
          //segmentation fault
          ndt::set_search_skiplist(base, FLAGS_data_path);
          runBenchmarks();
          return 0;
    }
    else
    {
        if(FLAGS_test_type == std::string("order"))
        {
            namedot::set_search_bench_single(FLAGS_data_path + "/orders/000002");
        }
        else if(FLAGS_test_type == std::string("transaction"))
        {
            ndt::set_search_bench_single(FLAGS_data_path + "/transactions/000002");
        }
        else if(FLAGS_test_type == std::string("timeshare"))
        {
            nts::set_search_bench_single(FLAGS_data_path + "/one_min/000002");
        }
    }

    runBenchmarks();

    return 0;
}
