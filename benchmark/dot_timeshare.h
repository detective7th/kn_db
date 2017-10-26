#ifndef _TEST_DOT_SHAREING_H_
#define _TEST_DOT_SHAREING_H_
#include "rand_common.h"
#include <folly/Benchmark.h>
#include <folly/MemoryMapping.h>
#include <folly/String.h>
#include <folly/FixedString.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "folly/FBString.h"
namespace nts 
{
    const int kSecurityCodeLength = 6;
    using namespace folly;
    struct TimeSharingDot
    {
        int64_t time_{0};
        double last_{0.0};
        double avg_{0.0};
        double vol_{0.0};
        double total_vol_{0.0};
        double turnover_{0.0};
    };
    struct spookyhask 
    {
        size_t operator()(const std::string &key) const
        {
            return folly::hash::SpookyHashV2::Hash32(key.c_str(), key.size(), 0);
        }
    };
    std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
    void rand_bench(int iters, const folly::StringPiece& file_data, const std::string front)
    {
        int size = file_data.size()/sizeof(TimeSharingDot);
        if(size <=0 || (file_data.size() % sizeof(TimeSharingDot) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        BenchmarkSuspender braces;
        braces.dismissing([&] {
            while (iters--) {
              std::unordered_map<std::string,TimeSharingDot*, spookyhask> test;
              for(int i = 0; i < size; i++)
              {
                //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(TimeSharingDot), sizeof(TimeSharingDot)));
                TimeSharingDot* dot = (TimeSharingDot*)(file_data.start() + i*sizeof(TimeSharingDot));
                test.emplace(front + std::to_string(dot->time_), dot);
              }
              doNotOptimizeAway(test);
            }
        });
    }
    void set_rand_bench(const std::string& path, const std::string& id)
    {
        folly::StringPiece file_data;
        //std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
        if (file_mapping)
        {
            file_data = file_mapping->data();
        }
        else
        {
            std::cout << "Lock Settle Memory Map Failed" <<std::endl;
            return;
        }
        addBenchmark(
            __FILE__,
            "rand_bench_timesharing_insert",
            [=](int iters) {
              rand_bench(iters ,file_data, id);
              return iters;
            });
    }
    void rand_search_bench(int iters,const std::unordered_map<std::string,TimeSharingDot*, spookyhask>& dicts , const std::vector<std::string>& search_key)
    {
        if(dicts.empty())
        {
            std::cout << "dict is empty" << std::endl;
        }
        BenchmarkSuspender braces;
        braces.dismissing([&] {
            while (iters--) {
                for(const auto& key : search_key)
                {
                  const auto res = dicts.find(key);
                  //doNotOptimizeAway(test);   
                }
            }
        });
    }
    void set_search_bench(const std::string& path, const std::string& id)
    {
        folly::StringPiece file_data;
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
        if (file_mapping)
        {
            file_data = file_mapping->data();
        }
        else
        {
            std::cout << "Lock Settle Memory Map Failed" <<std::endl;
            return;
        }
        int size = file_data.size()/sizeof(TimeSharingDot);
        if(size <=0 || (file_data.size() % sizeof(TimeSharingDot) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        std::unordered_map<std::string,TimeSharingDot*, spookyhask> test;
        for(int i = 0; i < size; i++)
        {
          //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(TimeSharingDot), sizeof(TimeSharingDot)));
          TimeSharingDot* dot = (TimeSharingDot*)(file_data.start() + i*sizeof(TimeSharingDot));
          test.emplace(id + std::to_string(dot->time_), dot);
        }
        std::cout << "size:" << size << std::endl;
        std::vector<std::string>  search_key = rand_date_17bit(id);;
        addBenchmark(
            __FILE__,
            "rand_search_bench_timeshare",
            [=](int iters) {
                rand_search_bench(iters ,test, search_key);
              return iters;
            });
    }
    //------------------single key test-------------------------
    template<typename T> void rand_bench_single(int iters, const folly::StringPiece& file_data, const std::string front)
    {
        int size = file_data.size()/sizeof(TimeSharingDot);
        if(size <=0 || (file_data.size() % sizeof(TimeSharingDot) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        BenchmarkSuspender braces;
        braces.dismissing([&] {
            while (iters--) {
              T test;
              for(int i = 0; i < size; i++)
              {
                //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(TimeSharingDot), sizeof(TimeSharingDot)));
                TimeSharingDot* dot = (TimeSharingDot*)(file_data.start() + i*sizeof(TimeSharingDot));
                test.emplace_back(front + std::to_string(dot->time_));
              }
              doNotOptimizeAway(test);
            }
        });
    }
    template<typename T> void set_rand_bench_single(const std::string& path, const std::string& id)
    {
        folly::StringPiece file_data;
        //std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
        if (file_mapping)
        {
            file_data = file_mapping->data();
        }
        else
        {
            std::cout << "Lock Settle Memory Map Failed" <<std::endl;
            return;
        }
        addBenchmark(
            __FILE__,
            "rand_bench_timesharing_insert_template",
            [=](int iters) {
                rand_bench_single<T>(iters ,file_data, id);
              return iters;
            });
    }
    template<typename T>void rand_search_bench_single(int iters,const T& container , const std::vector<std::string>& search_key)
    {
        BenchmarkSuspender braces;
        braces.dismissing([&] {
            while (iters--) {
                for(const auto& key : search_key)
                {
                  //const auto res = container.find(key);
                  //doNotOptimizeAway(test);   
                }
            }
        });
    }
    template<typename T> void set_search_bench_single(const std::string& path, const std::string& id)
    {
        folly::StringPiece file_data;
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
        if (file_mapping)
        {
            file_data = file_mapping->data();
        }
        else
        {
            std::cout << "Lock Settle Memory Map Failed" <<std::endl;
            return;
        }
        int size = file_data.size()/sizeof(TimeSharingDot);
        if(size <=0 || (file_data.size() % sizeof(TimeSharingDot) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        T test;
        for(int i = 0; i < size; i++)
        {
          //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(TimeSharingDot), sizeof(TimeSharingDot)));
          TimeSharingDot* dot = (TimeSharingDot*)(file_data.start() + i*sizeof(TimeSharingDot));
          test.emplace_back(id + std::to_string(dot->time_), dot);
        }
        std::vector<std::string>  search_key = rand_date_17bit(id);;
        addBenchmark(
            __FILE__,
            "rand_search_bench_single_timeshare",
            [=](int iters) {
                rand_search_bench_single<T>(iters ,test, search_key);
              return iters;
            });
    }
    
}// namespace tv

#endif