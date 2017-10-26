#ifndef _TEST_DOT_TRANSACTION_H_
#define _TEST_DOT_TRANSACTION_H_

#include "rand_common.h"
#include <folly/Benchmark.h>
#include <folly/MemoryMapping.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <time.h>
namespace ndt
{

    using namespace folly;
    struct DotTransaction
    {
        double trade_vol_ {0};
        double turnover_ {0};
        uint64_t timestamp_ {0};
        uint32_t sequence_ {0};
        md::hxzq::Direction direction_{md::hxzq::Direction::UNKNOW};
        double price_ {0};
        int64_t ask_order_no_ {0};
        int64_t bid_order_no_ {0};
        uint64_t trade_time_{0};
        md::hxzq::TradeType trade_type_{md::hxzq::TradeType::TT_UNKNOWN};
        int64_t transaction_no_ {0};
        int64_t transaction_seq_ {0};
        uint32_t channel_no_ {0};
    };
    std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
    void rand_bench(int iters, const folly::StringPiece& file_data)
    {
        int size = file_data.size()/sizeof(DotTransaction);
        if(size <=0 || (file_data.size() % sizeof(DotTransaction) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        BenchmarkSuspender braces;
        braces.dismissing([&] {
            while (iters--) {
              std::unordered_map<int64_t,DotTransaction*> test;
              for(int i = 0; i < size; i++)
              {
                DotTransaction* dot = (DotTransaction*)(file_data.start() + i*sizeof(DotTransaction));
                test.emplace(dot->transaction_no_, dot);
              }
              doNotOptimizeAway(test);
            }
        });
    }
    void set_rand_bench(const std::string& path)
    {
        srand(time(0));
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
        addBenchmark(
            __FILE__,
            "rand_bench_trans_insert",
            [=](int iters) {
              rand_bench(iters ,file_data);
              return iters;
            });
    }
    void rand_search_bench(int iters,const std::unordered_map<int64_t,DotTransaction*>& dicts , const std::vector<int64_t>& search_key)
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
    void set_search_bench(const std::string& path)
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
        int size = file_data.size()/sizeof(DotTransaction);
        if(size <=0 || (file_data.size() % sizeof(DotTransaction) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        std::unordered_map<int64_t,DotTransaction*> test;
        for(int i = 0; i < size; i++)
        {
          //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(DotTransaction), sizeof(DotTransaction)));
          DotTransaction* dot = (DotTransaction*)(file_data.start() + i*sizeof(DotTransaction));
          test.emplace(dot->transaction_no_, dot);
        }
        std::cout << "size:" << size << std::endl;
        std::vector<int64_t> search_key = rand_count();
        addBenchmark(
            __FILE__,
            "rand_search_bench_transaction",
            [=](int iters) {
                rand_search_bench(iters ,test, search_key);
              return iters;
            });
    }
    //--------------------single key insert ------------------
    template<typename T> void rand_bench_single(int iters, const folly::StringPiece& file_data)
    {
        int size = file_data.size()/sizeof(DotTransaction);
        if(size <=0 || (file_data.size() % sizeof(DotTransaction) != 0 ))
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
                DotTransaction* dot = (DotTransaction*)(file_data.start() + i*sizeof(DotTransaction));
                test.emplace_back(dot->transaction_no_);
              }
              doNotOptimizeAway(test);
            }
        });
    }
    template<typename T> void set_rand_bench_single(const std::string& path)
    {
        srand(time(0));
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
        addBenchmark(
            __FILE__,
            "rand_bench_trans_insert",
            [=](int iters) {
                rand_bench_single<T>(iters ,file_data);
              return iters;
            });
    }
    template<typename T>void rand_search_bench_single(int iters,T& dicts , const std::vector<int64_t>& search_key)
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
    template<typename T> void set_search_bench_single(const std::string& path)
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
        int size = file_data.size()/sizeof(DotTransaction);
        if(size <=0 || (file_data.size() % sizeof(DotTransaction) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return;
        }
        T test;
        for(int i = 0; i < size; i++)
        {
          //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(DotTransaction), sizeof(DotTransaction)));
          DotTransaction* dot = (DotTransaction*)(file_data.start() + i*sizeof(DotTransaction));
          test.emplace(dot->transaction_no_, dot);
        }
        std::vector<int64_t> search_key = rand_count();
        addBenchmark(
            __FILE__,
            "rand_search_bench_transaction_single",
            [=](int iters) {
                rand_search_bench_single<T>(iters ,test, search_key);
              return iters;
            });
    }
}// namespace ndt
#endif