#ifndef _TEST_RAND_COMMON_H_
#define _TEST_RAND_COMMON_H_
#include <vector>
#include <time.h>
#include "folly/FBString.h"
#include"boost/format.hpp"
#include <folly/Benchmark.h>
#include <folly/String.h>
#include <folly/FixedString.h>
#include <string>
const int kSearchCount = 1;
enum OrderType 
{
    OT_UNKNOWN = 0,
    OT_MARKET = 1,
    OT_LIMIT = 2,
    OT_BEST = 3
};

enum Direction 
{
    UNKNOW = 0,
    BID = 1,
    ASK = 2
};
enum TradeType 
{
    TT_UNKNOWN = 0,
    TT_TRADED = 1,
    TT_CANCELED = 2,
};
std::vector<int64_t> rand_count()
{
    std::vector<int64_t> vec;
    srand(time(0));
    int base = 20171001;
    for(int i = 0; i < kSearchCount; i ++)
    {
        
        vec.emplace_back(((int64_t)rand()%30 + base) << 32| ((int64_t)rand() % 10000000));
    }
    return vec;
}
std::vector<std::string> rand_date_17bit(const std::string& front)
{
    std::vector<std::string> vec;
    srand(time(0));
    boost::format fmt("2017%02d%02d%02d%02d00000");
    for(int i = 0; i < kSearchCount; i ++)
    {
        fmt % (rand()%12 +1) % (rand()%30 + 1) %(rand()%24) % (rand()%60);
        vec.emplace_back(front + fmt.str());
    }
    return vec;
}
template<typename T> void rand_bench_com(int iters, const folly::StringPiece& file_data, const std::function<bool(const folly::StringPiece& file_data, T& con)>& fun_insert)
{
    folly::BenchmarkSuspender braces;
    braces.dismissing([&] {
        while (iters--) {
          T test;
          if(!fun_insert(file_data, test))
          {
            return;
          }
          folly::doNotOptimizeAway(test);
        }
    });
}
template<typename T, typename P>void rand_search_bench_com(int iters,const T& dicts , const std::vector<P>& search_key, const std::function<bool(const T& dicts , const P& key)>& fun_search)
{
    folly::BenchmarkSuspender braces;
    braces.dismissing([&] {
        while (iters--) {
            for(const auto& key : search_key)
            {
              fun_search(dicts, key);
              folly::doNotOptimizeAway(dicts);   
            }
        }
    });
}
#endif