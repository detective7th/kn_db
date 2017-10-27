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
#include "db_core/data_base.h"
#include "db_service/data_def.h"
namespace namedot 
{
using namespace folly;
using namespace kn::db::core;
using namespace kn::db::service;
std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
template<typename T>bool fun_vector_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(Order);
    if(size <=0 || (file_data.size() % sizeof(Order) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        Order* dot = (Order*)(file_data.start() + i*sizeof(Order));
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
template<>bool fun_vector_insert<SkipList>(const folly::StringPiece& file_data, SkipList& con)
{
    int size = file_data.size()/sizeof(Order);
    if(size <=0 || (file_data.size() % sizeof(Order) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        Order* dot = (Order*)(file_data.start() + i*sizeof(Order));
        auto node = std::make_shared<DataNode>((void*)dot, sizeof(Order), ((dot->trade_time_/100000000) << 32)|(dot->order_no_)); 
        con.Insert(node);
        //std::cout << "size :" << size << "  i:" << i <<"  dot->order_no_:"<< dot->order_no_ <<std::endl;
    }
    std::cout << "test over" << std::endl;
    return true;
}

template<typename T>bool fun_map_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(Order);
    if(size <=0 || (file_data.size() % sizeof(Order) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        Order* dot = (Order*)(file_data.start() + i*sizeof(Order));
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
            rand_bench_com<std::map<int64_t, Order*>>(iters ,file_data,fun_map_insert<std::map<int64_t, Order*>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "order_unordered_map_insert",
        [=](int iters) {
            rand_bench_com<std::unordered_map<int64_t, Order*>>(iters ,file_data,fun_map_insert<std::unordered_map<int64_t, Order*>>);
            return iters;
        });
}
// void set_search_bench_single(const std::string& path)
// {
//     folly::StringPiece file_data;
//     if(!file_mapping)
//     {
//         file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
//     }
//     if (file_mapping)
//     {
//         file_data = file_mapping->data();
//         file_data = file_data.subpiece(sizeof(uint32_t));
//     }
//     else
//     {
//         std::cout << "Lock Settle Memory Map Failed" <<std::endl;
//         return;
//     }
//     int size = file_data.size()/sizeof(Order);
//     if(size <=0 || (file_data.size()% sizeof(Order) != 0 ))
//     {
//         std::cout << "file content error;" <<std::endl;
//         return;
//     }
//     std::cout <<"file: "<<__FILE__<< "single size : " << size << std::endl;
//     std::vector<int64_t> test;
//     std::list<int64_t> testlist;
//     std::map<int64_t, Order*> test_map;
//     std::unordered_map<int64_t, Order*> test_hash_map;
//     uint64_t no = 0;
//     for(int i = 0; i < size; i++)
//     {
//         //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(Order), sizeof(Order)));
//         Order* dot = (Order*)(file_data.start() +sizeof(uint32_t)+ i*sizeof(Order));
//         no = ((dot->trade_time_/100000000) << 32)|(dot->order_no_);
//         test.emplace_back(no);
//         testlist.emplace_back(no);
//         test_map.emplace(no, dot);
//         test_hash_map.emplace(no, dot);
//     }
//     std::vector<int64_t> search_key = rand_count();
//     addBenchmark(
//         __FILE__,
//         "order_vector_search",
//         [=](int iters) {
//             rand_search_bench_com<std::vector<int64_t>, int64_t>(iters ,test, search_key, fun_vector_search<std::vector<int64_t>>);
//             return iters;
//         });
//     addBenchmark(
//             __FILE__,
//             "order_list_search",
//             [=](int iters) {
//                 rand_search_bench_com<std::list<int64_t>, int64_t>(iters ,testlist, search_key, fun_vector_search<std::list<int64_t>>);
//                 return iters;
//             });    
//     addBenchmark(
//         __FILE__,
//         "order_map_search",
//         [=](int iters) {
//             rand_search_bench_com<std::map<int64_t, Order*>, int64_t>(iters ,test_map, search_key, fun_map_search<std::map<int64_t, Order*>>);
//             return iters;
//         });    
//     addBenchmark(
//         __FILE__,
//         "order_unordered_map_search",
//         [=](int iters) {
//             rand_search_bench_com<std::unordered_map<int64_t, Order*>, int64_t>(iters ,test_hash_map, search_key, fun_map_search<std::unordered_map<int64_t, Order*>>);
//             return iters;
//         });    
//     } 
void set_search_bench_single()
{
    std::string path = "/media/psf/Home/Documents/kn_db/kn_db/data/orders/000002";
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
    int size = file_data.size()/sizeof(Order);
    if(size <=0 || (file_data.size()% sizeof(Order) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return;
    }
    std::cout <<"file: "<<__FILE__<< "single size : " << size << std::endl;
    std::vector<int64_t> test;
    std::list<int64_t> testlist;
    std::map<int64_t, std::shared_ptr<DataNode>> test_map;
    std::unordered_map<int64_t, std::shared_ptr<DataNode>> test_hash_map;
    uint64_t no = 0;
    std::vector<int64_t> search_key;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(Order), sizeof(Order)));
        Order* dot = (Order*)(file_data.start() + i*sizeof(Order));
        no = kn::db::service::CombineHiLow<kn::db::core::KeyType, uint32_t>
        (static_cast<uint32_t>(dot->trade_time_/1000000000)
         , static_cast<uint32_t>(dot->order_no_));
         search_key.emplace_back(no);
        auto ptr = std::make_shared<DataNode>((void*)dot,sizeof(Order),no);
        test.emplace_back(no);
        testlist.emplace_back(no);
        test_map.emplace(no, ptr);
        test_hash_map.emplace(no, ptr);
    }
    std::reverse(search_key.begin(), search_key.end());
    std::cout<< "search key size:" << search_key.size() << std::endl;
    // addBenchmark(
    //     __FILE__,
    //     "order_vector_search",
    //     [=](int iters) {
    //         rand_search_bench_com<std::vector<int64_t>, int64_t>(iters ,test, search_key, fun_vector_search<std::vector<int64_t>>);
    //         return iters;
    //     });
    // addBenchmark(
    //         __FILE__,
    //         "order_list_search",
    //         [=](int iters) {
    //             rand_search_bench_com<std::list<int64_t>, int64_t>(iters ,testlist, search_key, fun_vector_search<std::list<int64_t>>);
    //             return iters;
    //         });    
    addBenchmark(
        __FILE__,
        "order_map_search",
        [=](int iters) {
            rand_search_bench_com<std::map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_map, search_key, fun_map_search<std::map<int64_t,std::shared_ptr<DataNode>>>);
            return iters;
        });    
    addBenchmark(
        __FILE__,
        "order_unordered_map_search",
        [=](int iters) {
            rand_search_bench_com<std::unordered_map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_hash_map, search_key, fun_map_search<std::unordered_map<int64_t, std::shared_ptr<DataNode>>>);
            return iters;
        });    
    } 
    std::vector<int64_t> rand_search_key()
    {
        std::string path = "/media/psf/Home/Documents/kn_db/kn_db/data/orders/000002";
        std::shared_ptr<folly::MemoryMapping> tmp_mapping = nullptr;
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());

        folly::StringPiece file_data;
        file_data = file_mapping->data();
        file_data = file_data.subpiece(sizeof(uint32_t));
        
        int size = file_data.size()/sizeof(Order);
        std::vector<int64_t> con;
        if(size <=0 || (file_data.size()% sizeof(Order) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return con;
        }
        
        for(int i = 0; i < size; i++)
        {
            //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(Order), sizeof(Order)));
            Order* dot = (Order*)(file_data.start() +sizeof(uint32_t)+ i*sizeof(Order));
            con.emplace_back(kn::db::service::CombineHiLow<kn::db::core::KeyType, uint32_t>
                (static_cast<uint32_t>(dot->trade_time_/1000000000)
                 , static_cast<uint32_t>(dot->order_no_)));
        }
        return con;
    }
    void rand_bench_skiplist_search(int iters, kn::db::core::DataBase& base, Table* table,const std::vector<int64_t>& search_key)
    {
        folly::BenchmarkSuspender braces;
       
        braces.dismissing([&] {
            while (iters--) {
                for(const auto& iter : search_key)
                {
                    table->Find(iter);  
                }
                //folly::doNotOptimizeAway(base); 
            }
        });
    }
    void set_search_skiplist(kn::db::core::DataBase& base)
    {
        auto search_key = rand_search_key();
        std::reverse(search_key.begin(), search_key.end());
        std::cout<< "size:" << search_key.size() << std::endl;
        auto table = base.GetSet("orders")->GetTable("000002").get();
        addBenchmark(
            __FILE__,
            "order_skiplist_search",
            [=,&base](int iters) {
                rand_bench_skiplist_search(iters ,base, table, search_key);
                return iters;
            });
    }
}// namespace tv

#endif