#ifndef _TEST_DOT_CANDLE_H_
#define _TEST_DOT_CANDLE_H_

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
#include "market_def.pb.h"
namespace ndc 
{
    using namespace folly;
    struct DotCandle
    {
        double  high_;
        double  low_;
        double  open_;
        double  close_;
        int64_t open_time_;
        int64_t close_time_;
        double  vol_;
        double  total_vol_;
        double  turnover_;
    };
    
    std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
    void rand_bench(int iters, const folly::StringPiece& file_data,const std::string front)
    {
        int size = file_data.size()/sizeof(DotCandle);
        if(size <=0 || (file_data.size() % sizeof(DotCandle) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        BenchmarkSuspender braces;
        braces.dismissing([&] {
            while (iters--) {
              std::unordered_map<std::string,DotCandle*> test;
              for(int i = 0; i < size; i++)
              {
                DotCandle* dot = (DotCandle*)(file_data.start() + i*sizeof(DotCandle));
                test.emplace(front + std::to_string(dot->open_time_), dot);
              }
              doNotOptimizeAway(test);
            }
        });
    }
    void set_rand_bench(const std::string& path,const std::string& id)
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
        std::string front;
        std::size_t pos = path.find_last_of('.');
        if (pos != std::string::npos)
        {
            std::size_t posline = path.find_last_of('/');
            if (pos != std::string::npos)
            {
                if(pos - posline > 1)
                {
                    front = path.substr(posline+1, pos - posline);
                }
            }
        }
        front = id +front;
        addBenchmark(
            __FILE__,
            "rand_bench_candle_insert",
            [=](int iters) {
              rand_bench(iters ,file_data, front);
              return iters;
            });
    }
    void rand_search_bench(int iters,const std::unordered_map<std::string,DotCandle*>& dicts , const std::vector<std::string>& search_key)
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
        int size = file_data.size()/sizeof(DotCandle);
        if(size <=0 || (file_data.size() % sizeof(DotCandle) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        std::unordered_map<std::string,DotCandle*> test;
        for(int i = 0; i < size; i++)
        {
          //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(DotCandle), sizeof(DotCandle)));
          DotCandle* dot = (DotCandle*)(file_data.start() + i*sizeof(DotCandle));
          test.emplace(id + std::to_string(dot->open_time_), dot);
        }
        std::cout << "size:" << size << std::endl;
        std::vector<std::string> search_key = rand_date_17bit(id);
        addBenchmark(
            __FILE__,
            "rand_search_bench_candle",
            [=](int iters) {
                rand_search_bench(iters ,test, search_key);
              return iters;
            });
    }
    //-------------------------------single key test -----------------------------
    template<typename T> void rand_bench_single(int iters, const folly::StringPiece& file_data,const std::string front)
    {
        int size = file_data.size()/sizeof(DotCandle);
        if(size <=0 || (file_data.size() % sizeof(DotCandle) != 0 ))
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
                DotCandle* dot = (DotCandle*)(file_data.start() + i*sizeof(DotCandle));
                test.emplace_back(front + std::to_string(dot->open_time_));
              }
              doNotOptimizeAway(test);
            }
        });
    }
    template<typename T> void set_rand_bench_single(const std::string& path,const std::string& id)
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
        std::string front;
        std::size_t pos = path.find_last_of('.');
        if (pos != std::string::npos)
        {
            std::size_t posline = path.find_last_of('/');
            if (pos != std::string::npos)
            {
                if(pos - posline > 1)
                {
                    front = path.substr(posline+1, pos - posline);
                }
            }
        }
        front = id +front;
        addBenchmark(
            __FILE__,
            "rand_bench_candle_insert_single",
            [=](int iters) {
                rand_bench_single<T>(iters ,file_data, front);
              return iters;
            });
    }
    template<typename T> void set_rand_bench_single(int iters,const T& dicts , const std::vector<std::string>& search_key)
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
                  //const auto res = dicts.find(key);
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
        int size = file_data.size()/sizeof(DotCandle);
        if(size <=0 || (file_data.size() % sizeof(DotCandle) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        T test;
        for(int i = 0; i < size; i++)
        {
          //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(DotCandle), sizeof(DotCandle)));
          DotCandle* dot = (DotCandle*)(file_data.start() + i*sizeof(DotCandle));
          test.emplace(id + std::to_string(dot->open_time_), dot);
        }
        std::vector<std::string> search_key = rand_date_17bit(id);
        addBenchmark(
            __FILE__,
            "rand_search_bench_candle_single",
            [=](int iters) {
                set_rand_bench_single<T>(iters ,test, search_key);
              return iters;
            });
    }
}// namespace tv

#endif