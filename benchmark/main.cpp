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
#include <gflags/gflags.h>
using namespace std;
using namespace folly;

DEFINE_string(type, "sl", "test type");
DEFINE_int32(max_level, 8, "max level in kn_db");
DEFINE_int32(skip, 4, "max skip in kn_db");
DEFINE_string(path, "/media/psf/Home/Documents/kn_db/kn_db/data", "data path");
DEFINE_int32(multiple, 0, "rang test");
void sleeptimes()
{
  // int times = 10;
  // while(times -- )
  // {
  //   std::cout << "times :" << times << std::endl;
  //   usleep(1000000);
  // }
}
void release_path(const std::string& path, std::map<std::string,std::string>& all_paths)
{
  std::string set_name;
  for (boost::filesystem::recursive_directory_iterator it(path);
        it != boost::filesystem::recursive_directory_iterator(); ++it)
  {
    if (boost::filesystem::is_directory(*it))
    {
        set_name = it->path().leaf().string().c_str();
        continue;
    }
    all_paths.emplace(set_name, it->path().string());
  }
}
int main(int argc, char** argv) 
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);

 std::map<std::string,std::string> all_paths;
  release_path(FLAGS_path, all_paths);
  const auto res = all_paths.find(FLAGS_type);
  std::string tmp_path;
  if(res != all_paths.end())
  {
    tmp_path = res->second;
  }
  if(FLAGS_type == std::string("sl"))
  {
    kn::db::core::DataBase base("test");
    release_skillist(FLAGS_path,base, FLAGS_max_level, FLAGS_skip);
    for(const auto& iter : all_paths)
    {
      if ("orders" == iter.first)
      {
        namedot::set_search_skiplist(iter.second, base, FLAGS_multiple);
      }
      else if ("transactions" == iter.first)
      {
        ndt::set_search_skiplist(iter.second, base, FLAGS_multiple);
      }
      else if ("one_min" == iter.first)
      {
        nts::set_search_skiplist(iter.second, base, FLAGS_multiple);
      }
    }
    sleeptimes();
    runBenchmarks();
    sleeptimes();
    return 0;
  }
  else if(FLAGS_type  == std::string("orders"))
  {
    namedot::set_search_bench_single(tmp_path, FLAGS_multiple);
  }
  else if(FLAGS_type == std::string("transactions"))
  {
    ndt::set_search_bench_single(tmp_path, FLAGS_multiple);
  }
  else if(FLAGS_type == std::string("one_min"))
  {
    nts::set_search_bench_single(tmp_path, FLAGS_multiple);
  }
  sleeptimes();
  runBenchmarks();
  sleeptimes();
  return 0;
}
