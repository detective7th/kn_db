#ifndef _TEST_DOT_CANDLE_H_
#define _TEST_DOT_CANDLE_H_

#include "rand_common.h"
#include <folly/Benchmark.h>
#include <folly/MemoryMapping.h>
#include <folly/String.h>
#include <folly/FixedString.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "folly/FBString.h"
#include "db_service/data_def.h"
#include "db_core/data_base.h"
#include "db_service/data_def.h"
#include "skiplist.hpp"
#include "btree.h"
#include "art/radix_map.h"
#include "stx/btree_map.h"
namespace ndc 
{
using namespace folly;
using namespace kn::db::core;
using namespace kn::db::service;
struct spookyhask 
{
    size_t operator()(const std::string &key) const
    {
        return folly::hash::SpookyHashV2::Hash32(key.c_str(), key.size(), 0);
    }
};
std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
std::string kcode_id_;
template<typename T>bool fun_vector_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(Candle);
    if(size <=0 || (file_data.size() % sizeof(Candle) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        Candle* dot = (Candle*)(file_data.start() + i*sizeof(Candle));
        con.emplace_back(kcode_id_ + std::to_string(dot->open_time_));
    }
    return true;
}

template<typename T>bool fun_map_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(Candle);
    if(size <=0 || (file_data.size() % sizeof(Candle) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        Candle* dot = (Candle*)(file_data.start() + i*sizeof(Candle));
        con.emplace(kcode_id_ + std::to_string(dot->open_time_), dot);
    }
    return true;
}

void set_rand_bench_single(const std::string& path,const std::string& id)
{
    folly::StringPiece file_data;
    if(!file_mapping)
    {
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());
        kcode_id_ = "000002";
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
    //std::string front("000002");
    // std::size_t pos = path.find_last_of('.');
    // if (pos != std::string::npos)
    // {
    //     std::size_t posline = path.find_last_of('/');
    //     if (pos != std::string::npos)
    //     {
    //         if(pos - posline > 1)
    //         {
    //             front = path.substr(posline+1, pos - posline);
    //         }
    //     }
    // }
    addBenchmark(
        __FILE__,
        "candle_vector_insert",
        [=](int iters) {
            rand_bench_com<std::vector<std::string>>(iters ,file_data, fun_vector_insert<std::vector<std::string>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "candle_list_insert",
        [=](int iters) {
            rand_bench_com<std::list<std::string>>(iters ,file_data, fun_vector_insert<std::list<std::string>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "candle_map_insert",
        [=](int iters) {
            rand_bench_com<std::map<std::string, Candle*>>(iters ,file_data, fun_map_insert<std::map<std::string, Candle*>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "candle_unordered_map_insert",
        [=](int iters) {
            rand_bench_com<std::unordered_map<std::string, Candle*>>(iters ,file_data, fun_map_insert<std::unordered_map<std::string, Candle*>>);
            return iters;
        });
}
void set_search_bench_single(std::string path)
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
    int size = file_data.size()/sizeof(Candle);
    if(size <=0 || (file_data.size() % sizeof(Candle) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return;
    }
    std::cout <<"file: "<<__FILE__<< "single size : " << size << std::endl;
    std::vector<int64_t> test;
    std::list<int64_t> testlist;
    std::map<int64_t, std::shared_ptr<DataNode>> test_map;
    std::unordered_map<int64_t, std::shared_ptr<DataNode>> test_hash_map;
    guoxiao::skiplist::SkipList<int64_t, std::shared_ptr<DataNode>> test_skip_list;
    static trees::BTree<int64_t, std::shared_ptr<DataNode>> test_btree(64);
    static art::radix_map<int64_t, std::shared_ptr<DataNode>> test_art;
    stx::btree_map<int64_t, std::shared_ptr<DataNode>> test_bplustree;
   // std::unordered_map<uint64_t, std::shared_ptr<DataNode>, spookyhask> spooky_hash_map;
    uint64_t no = 0;
    std::vector<int64_t> search_key;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(Candle), sizeof(Candle)));
        Candle* dot = (Candle*)(file_data.start() + i*sizeof(Candle));
        auto ptr = std::make_shared<DataNode>((void*)dot,sizeof(Order),no);
        no = dot->open_time_;
        search_key.emplace_back(no);
        test.emplace_back(no);
        testlist.emplace_back(no);
        test_map.emplace(no, ptr);
        test_hash_map.emplace(no, ptr);
        test_skip_list.emplace(no, ptr);
        test_btree.insert(no, ptr);
        test_bplustree.insert(no, ptr);
        test_art.emplace(no, ptr);
    }
    search_key = rand_count_in_vec(search_key);
    std::cout<< "search key size:" << search_key.size() << std::endl;
    const std::string test_name("candle_test");  
    addBenchmark(
        test_name.c_str(),
        "map",
        [=](int iters) {
            rand_search_bench_com<std::map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_map, search_key, fun_map_search<std::map<int64_t,std::shared_ptr<DataNode>>, int64_t>);
            return iters;
        });    
    addBenchmark(
        test_name.c_str(),
        "unordered_map",
        [=](int iters) {
            rand_search_bench_com<std::unordered_map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_hash_map, search_key, fun_map_search<std::unordered_map<int64_t, std::shared_ptr<DataNode>>, int64_t>);
            return iters;
        });
    addBenchmark(
        test_name.c_str(),
        "vector_binary",
        [=](int iters) {
            rand_search_bench_com<std::vector<int64_t>, int64_t>(iters ,test, search_key, fun_vector_binary_search);
            return iters;
        });
    addBenchmark(
        test_name.c_str(),
        "tradition_skiplist",
        [=](int iters) {
            rand_search_bench_com<guoxiao::skiplist::SkipList<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_skip_list, search_key, fun_map_search<guoxiao::skiplist::SkipList<int64_t,std::shared_ptr<DataNode>>, int64_t>);
            return iters;
        });
    addBenchmark(
        test_name.c_str(),
        "btree",
        [=](int iters) {
            rand_search_bench_com<trees::BTree<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_btree, search_key, fun_map_search<trees::BTree<int64_t,std::shared_ptr<DataNode>>, int64_t>);
            return iters;
        });
    addBenchmark(
        test_name.c_str(),
        "bplustree",
        [=](int iters) {
            rand_search_bench_com<stx::btree_map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_bplustree, search_key, fun_map_search<stx::btree_map<int64_t,std::shared_ptr<DataNode>>, int64_t>);
            return iters;
        });
    addBenchmark(
        test_name.c_str(),
        "art tree",
        [=](int iters) {
            rand_search_bench_com< art::radix_map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_art, search_key, fun_map_search< art::radix_map<int64_t,std::shared_ptr<DataNode>>, int64_t>);
            return iters;
        });
}
    
}// namespace tv

#endif