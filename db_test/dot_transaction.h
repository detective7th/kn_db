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
    Direction direction_{Direction::UNKNOW};
    double price_ {0};
    int64_t ask_order_no_ {0};
    int64_t bid_order_no_ {0};
    uint64_t trade_time_{0};
    TradeType trade_type_{TradeType::TT_UNKNOWN};
    int64_t transaction_no_ {0};
    int64_t transaction_seq_ {0};
    uint32_t channel_no_ {0};
};
std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
template<typename T>bool fun_vector_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(DotTransaction);
    if(size <=0 || (file_data.size() % sizeof(DotTransaction) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        DotTransaction* dot = (DotTransaction*)(file_data.start() + i*sizeof(DotTransaction));
        con.emplace_back(((dot->trade_time_/100000000) << 32)|(dot->transaction_no_));
    }
    return true;
}
template<typename T> bool fun_vector_search(const T& con, const int64_t& search_key)
{
    auto res = std::find(con.begin(), con.end(), search_key);
    if(res != con.end())
    {
        //std::cout <<" found";
    }
}
template<typename T>bool fun_map_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(DotTransaction);
    if(size <=0 || (file_data.size() % sizeof(DotTransaction) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        DotTransaction* dot = (DotTransaction*)(file_data.start() + i*sizeof(DotTransaction));
        con.emplace(((dot->trade_time_/100000000) << 32)|(dot->transaction_no_), dot);
    }
    return true;
}
template<typename T> bool fun_map_search(const T& con, const int64_t& search_key)
{
    auto res = con.find(search_key);
    if(res != con.end())
    {
        //std::cout <<" found";
    }
}

void set_rand_bench_single(const std::string& path)
{
    folly::StringPiece file_data;
    if(!file_mapping)
    {
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
    }
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
        "transaction_insert",
        [=](int iters) {
            rand_bench_com<std::vector<int64_t>>(iters ,file_data,fun_vector_insert<std::vector<int64_t>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "transaction_list_insert",
        [=](int iters) {
            rand_bench_com<std::list<int64_t>>(iters ,file_data,fun_vector_insert<std::list<int64_t>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "transaction_map_insert",
        [=](int iters) {
            rand_bench_com<std::map<int64_t, DotTransaction*>>(iters ,file_data,fun_map_insert<std::map<int64_t, DotTransaction*>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "transaction_unordered_map_insert",
        [=](int iters) {
            rand_bench_com<std::unordered_map<int64_t, DotTransaction*>>(iters ,file_data,fun_map_insert<std::unordered_map<int64_t, DotTransaction*>>);
            return iters;
        });
}
void set_search_bench_single(const std::string& path)
{
    folly::StringPiece file_data;
    if(!file_mapping)
    {
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
    }
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
    std::cout <<"file: "<<__FILE__<< "single size : " << size << std::endl;
    std::vector<int64_t> test;
    std::list<int64_t> testlist;
    std::map<int64_t, DotTransaction*> test_map;
    std::unordered_map<int64_t, DotTransaction*> test_hash_map;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(DotTransaction), sizeof(DotTransaction)));
        DotTransaction* dot = (DotTransaction*)(file_data.start() + i*sizeof(DotTransaction));
        test.emplace_back( dot->transaction_no_);
        testlist.emplace_back( dot->transaction_no_);
        test_map.emplace( dot->transaction_no_, dot);
        test_hash_map.emplace( dot->transaction_no_, dot);
    }
    std::vector<int64_t> search_key = rand_count();
    addBenchmark(
        __FILE__,
        "transaction_vector_search",
        [=](int iters) {
            rand_search_bench_com<std::vector<int64_t>, int64_t>(iters ,test, search_key, fun_vector_search<std::vector<int64_t>>);
            return iters;
        });
    addBenchmark(
            __FILE__,
            "transaction_list_search",
            [=](int iters) {
                rand_search_bench_com<std::list<int64_t>, int64_t>(iters ,testlist, search_key, fun_vector_search<std::list<int64_t>>);
                return iters;
            });    
    addBenchmark(
        __FILE__,
        "transaction_map_search",
        [=](int iters) {
            rand_search_bench_com<std::map<int64_t, DotTransaction*>, int64_t>(iters ,test_map, search_key, fun_map_search<std::map<int64_t, DotTransaction*>>);
            return iters;
        });    
    addBenchmark(
        __FILE__,
        "transaction_unordered_map_search",
        [=](int iters) {
            rand_search_bench_com<std::unordered_map<int64_t, DotTransaction*>, int64_t>(iters ,test_hash_map, search_key, fun_map_search<std::unordered_map<int64_t, DotTransaction*>>);
            return iters;
        });    
    } 
}// namespace ndt
#endif