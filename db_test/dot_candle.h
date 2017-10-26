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
std::string kcode_id_;
template<typename T>bool fun_vector_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(DotCandle);
    if(size <=0 || (file_data.size() % sizeof(DotCandle) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        DotCandle* dot = (DotCandle*)(file_data.start() + i*sizeof(DotCandle));
        con.emplace_back(kcode_id_ + std::to_string(dot->open_time_));
    }
    return true;
}
template<typename T> bool fun_vector_search(const T& con, const std::string& search_key)
{
    auto res = std::find(con.begin(), con.end(), search_key);
    if(res != con.end())
    {
        //std::cout <<" found";
    }
}
template<typename T>bool fun_map_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(DotCandle);
    if(size <=0 || (file_data.size() % sizeof(DotCandle) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        DotCandle* dot = (DotCandle*)(file_data.start() + i*sizeof(DotCandle));
        con.emplace(kcode_id_ + std::to_string(dot->open_time_), dot);
    }
    return true;
}
template<typename T> bool fun_map_search(const T& con, const std::string& search_key)
{
    auto res = con.find(search_key);
    if(res != con.end())
    {
        //std::cout <<" found";
    }
}
void set_rand_bench_single(const std::string& path,const std::string& id)
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
            rand_bench_com<std::map<std::string, DotCandle*>>(iters ,file_data, fun_map_insert<std::map<std::string, DotCandle*>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "candle_unordered_map_insert",
        [=](int iters) {
            rand_bench_com<std::unordered_map<std::string, DotCandle*>>(iters ,file_data, fun_map_insert<std::unordered_map<std::string, DotCandle*>>);
            return iters;
        });
}
void set_search_bench_single(const std::string& path, const std::string& id)
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
    int size = file_data.size()/sizeof(DotCandle);
    if(size <=0 || (file_data.size() % sizeof(DotCandle) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return;
    }
    std::cout <<"file: "<<__FILE__<< "single size : " << size << std::endl;
    std::vector<std::string> test;
    std::list<std::string> testlist;
    std::map<std::string, DotCandle*> test_map;
    std::unordered_map<std::string, DotCandle*> test_hash_map;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(DotCandle), sizeof(DotCandle)));
        DotCandle* dot = (DotCandle*)(file_data.start() + i*sizeof(DotCandle));
        //test.emplace(id + std::to_string(dot->open_time_), dot);
        test.emplace_back(kcode_id_ + std::to_string(dot->open_time_));
        test_map.emplace(kcode_id_ + std::to_string(dot->open_time_), dot);
        test_hash_map.emplace(kcode_id_ + std::to_string(dot->open_time_),dot);
    }
    std::vector<std::string> search_key = rand_date_17bit(id);
    addBenchmark(
        __FILE__,
        "candle_vector_search",
        [=](int iters) {
            rand_search_bench_com<std::vector<std::string>, std::string>(iters ,test, search_key, fun_vector_search<std::vector<std::string>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "candle_list_search",
        [=](int iters) {
        rand_search_bench_com<std::list<std::string>, std::string>(iters ,testlist, search_key, fun_vector_search<std::list<std::string>>);
        return iters;
        });
    addBenchmark(
        __FILE__,
        "candle_map_search",
        [=](int iters) {
        rand_search_bench_com<std::map<std::string, DotCandle*>, std::string>(iters ,test_map, search_key, fun_map_search<std::map<std::string, DotCandle*>>);
        return iters;
        });
    addBenchmark(
        __FILE__,
        "candle_unordered_map_search",
        [=](int iters) {
        rand_search_bench_com<std::map<std::string, DotCandle*>, std::string>(iters ,test_map, search_key, fun_map_search<std::map<std::string, DotCandle*>>);
        return iters;
        });
}
    
}// namespace tv

#endif