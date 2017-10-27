#ifndef _TEST_DOT_ORDER_H_
#define _TEST_DOT_ORDER_H_

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
#include <algorithm>
#include "folly/FBString.h"
#include "db_core/skiplist.h"
namespace namedot 
{
using namespace folly;
struct DotOrder
{
    double trade_vol_ {0};
    //double turnover_ {0};
    //uint64_t timestamp_ {0};
    //uint32_t sequence_ {0};
    Direction direction_{Direction::UNKNOW};
    double price_ {0};
    uint64_t trade_time_{0};
    OrderType order_type_{OrderType::OT_UNKNOWN};
    int64_t order_no_{0};
};
std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
template<typename T>bool fun_vector_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(DotOrder);
    if(size <=0 || (file_data.size() % sizeof(DotOrder) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        DotOrder* dot = (DotOrder*)(file_data.start() + i*sizeof(DotOrder));
        con.emplace_back(((dot->trade_time_/100000000) << 32)|(dot->order_no_));
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
    return true;
}
template<typename T>bool fun_map_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(DotOrder);
    if(size <=0 || (file_data.size() % sizeof(DotOrder) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        DotOrder* dot = (DotOrder*)(file_data.start() + i*sizeof(DotOrder));
        con.emplace(((dot->trade_time_/100000000) << 32)|(dot->order_no_), dot);
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
    return true;
}
bool fun_skiplist_insert(const folly::StringPiece& file_data, kn::db::core::SkipList& con)
{
    int size = file_data.size()/sizeof(DotOrder);
    if(size <=0 || (file_data.size() % sizeof(DotOrder) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        DotOrder* dot = (DotOrder*)(file_data.start() + i*sizeof(DotOrder));
        auto data_node = std::make_shared<kn::db::core::DataNode>((void*)dot, sizeof(DotOrder), ((dot->trade_time_/100000000) << 32)|(dot->order_no_));
        //con.Insert(data_node);
    }
    return true;
}
// bool fun_skiplist_search(kn::db::core::SkipList& con, const uint32_t& search_key)
// {
//     auto res = con.Find(search_key);
//     if(res)
//     {
//         //std::cout <<" found";
//     }
//     return true;
// }
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
        file_data = file_data.subpiece(sizeof(uint32_t));
    }
    else
    {
        std::cout << "Lock Settle Memory Map Failed" <<std::endl;
        return;
    }
    addBenchmark(
        __FILE__,
        "order_insert",
        [=](int iters) {
            rand_bench_com<std::vector<int64_t>>(iters ,file_data,fun_vector_insert<std::vector<int64_t>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "order_list_insert",
        [=](int iters) {
            rand_bench_com<std::list<int64_t>>(iters ,file_data,fun_vector_insert<std::list<int64_t>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "order_map_insert",
        [=](int iters) {
            rand_bench_com<std::map<int64_t, DotOrder*>>(iters ,file_data,fun_map_insert<std::map<int64_t, DotOrder*>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "order_unordered_map_insert",
        [=](int iters) {
            rand_bench_com<std::unordered_map<int64_t, DotOrder*>>(iters ,file_data,fun_map_insert<std::unordered_map<int64_t, DotOrder*>>);
            return iters;
        });
    // addBenchmark(
    //     __FILE__,
    //     "order_skiplist_insert",
    //     [=](int iters) {
    //         rand_bench_com< kn::db::core::SkipList>(iters ,file_data,fun_vector_insert<kn::db::core::SkipList>);
    //         return iters;
    //     });
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
        file_data = file_data.subpiece(sizeof(uint32_t));
    }
    else
    {
        std::cout << "Lock Settle Memory Map Failed" <<std::endl;
        return;
    }
    int size = file_data.size()/sizeof(DotOrder);
    if(size <=0 || (file_data.size()% sizeof(DotOrder) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return;
    }
    std::cout <<"file: "<<__FILE__<< "single size : " << size << std::endl;
    std::vector<int64_t> test;
    std::list<int64_t> testlist;
    std::map<int64_t, DotOrder*> test_map;
    std::unordered_map<int64_t, DotOrder*> test_hash_map;
    uint64_t no = 0;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(DotOrder), sizeof(DotOrder)));
        DotOrder* dot = (DotOrder*)(file_data.start() +sizeof(uint32_t)+ i*sizeof(DotOrder));
        no = ((dot->trade_time_/100000000) << 32)|(dot->order_no_);
        test.emplace_back(no);
        testlist.emplace_back(no);
        test_map.emplace(no, dot);
        test_hash_map.emplace(no, dot);
    }
    std::vector<int64_t> search_key = rand_count();
    addBenchmark(
        __FILE__,
        "order_vector_search",
        [=](int iters) {
            rand_search_bench_com<std::vector<int64_t>, int64_t>(iters ,test, search_key, fun_vector_search<std::vector<int64_t>>);
            return iters;
        });
    addBenchmark(
            __FILE__,
            "order_list_search",
            [=](int iters) {
                rand_search_bench_com<std::list<int64_t>, int64_t>(iters ,testlist, search_key, fun_vector_search<std::list<int64_t>>);
                return iters;
            });    
    addBenchmark(
        __FILE__,
        "order_map_search",
        [=](int iters) {
            rand_search_bench_com<std::map<int64_t, DotOrder*>, int64_t>(iters ,test_map, search_key, fun_map_search<std::map<int64_t, DotOrder*>>);
            return iters;
        });    
    addBenchmark(
        __FILE__,
        "order_unordered_map_search",
        [=](int iters) {
            rand_search_bench_com<std::unordered_map<int64_t, DotOrder*>, int64_t>(iters ,test_hash_map, search_key, fun_map_search<std::unordered_map<int64_t, DotOrder*>>);
            return iters;
        });    
    }   
}// namespace tv

#endif