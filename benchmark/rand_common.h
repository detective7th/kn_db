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
#include <algorithm>
#include "db_core/data_base.h"
#include "db_service/data_def.h"
#include "db_core/data_base.h"
const int kSearchCount = 1000000;
const int kMinLimtCount = 10;
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
std::vector<int64_t> rand_count_in_vec(const std::vector<int64_t>& data)
{
    std::vector<int64_t> vec;
    if(data.empty())
    {
        std::cout<< "data source is empty"<< std::endl;
        return vec;
    }
    srand(time(0));
    int value = 0;
    for(int i = 0; i < kSearchCount; i ++)
    {
        value = rand() % data.size();
        vec.emplace_back(data.at(value));
    }
    return vec;
}
std::vector<std::pair<int64_t,int64_t>> rand_rang_count_in_vec(const std::vector<int64_t>& data, int mutiple = 1)
{
    std::vector<std::pair<int64_t,int64_t>> vec;
    if(data.empty())
    {
        std::cout<< "data source is empty"<< std::endl;
        return vec;
    }
    srand(time(0));
    int first = 0, second = mutiple;
    int max = mutiple;
    for(int i = 0; i < max; i ++)
    {
        //first = rand() % data.size();
        //second = rand() % (kMinLimtCount * mutiple) + 1;
        first = 1;
        vec.emplace_back(std::pair<int64_t,int64_t>(data.at(first), data.at(first) + second));
    }
    return vec;
}
std::map<uint32_t,uint32_t> rand_map_count()
{
    srand(time(0));
    std::map<uint32_t,uint32_t> con;
    boost::format fmt("20%2d%2d%2d");
    for(int i = 0; i < kSearchCount; i ++)
    {
        fmt%(rand()%17)%(rand()%12 + 1)%(rand()%30+1);
        con.emplace(atoi(fmt.str().c_str()), rand()% 10000000 + 1);
    }
    return con;
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
            //   if(!fun_search(dicts, key))
            //   {
            //     //std::cout << "Not fount key:" << key << "\t";
            //   }
              folly::doNotOptimizeAway(dicts);   
            }
        }
    });
}
void release_skillist(const std::string path, kn::db::core::DataBase& base, uint32_t max_level = 8, uint32_t skip = 4)
{
    folly::fbstring set_name;
    for (boost::filesystem::recursive_directory_iterator it(path);
         it != boost::filesystem::recursive_directory_iterator(); ++it)
    {
        if (boost::filesystem::is_directory(*it))
        {
            set_name = it->path().leaf().string().c_str();
            auto set = std::make_shared<kn::db::core::Set>(set_name.c_str());
            assert(base.AddSet(set));
        }
        else
        {
            if (!set_name.empty())
            {
                std::function<kn::db::core::KeyType (void*, kn::db::core::Table*)> hash_func;
                if ("orders" == set_name)
                {
                    hash_func = [=](void* data, kn::db::core::Table* table){
                        auto o = (kn::db::service::Order*)data;
                        return kn::db::service::CombineHiLow<kn::db::core::KeyType, uint32_t>
                        (static_cast<uint32_t>(o->trade_time_/1000000000)
                         , static_cast<uint32_t>(o->order_no_));
                    };
                }
                else if ("transactions" == set_name)
                {
                    hash_func = [=](void* data, kn::db::core::Table* table){
                        auto o = (kn::db::service::Transaction*)data;
                        return kn::db::service::CombineHiLow<kn::db::core::KeyType, uint32_t>
                        (static_cast<uint32_t>(o->trade_time_/1000000000)
                         , static_cast<uint32_t>(o->transaction_no_));
                    };
                }
                else if ("one_min" == set_name)
                {
                    hash_func = [=](void* data, kn::db::core::Table* table){
                        auto o = (kn::db::service::TimeShare*)data;
                        return o->time_;
                    };
                }
                else continue;

                auto table = std::make_shared<kn::db::core::Table>(it->path()
                                                                   , it->path().leaf().string().c_str()
                                                                   , hash_func);

                table->InitSkipList(max_level, skip);

                assert(base.AddTable({set_name.begin(), set_name.end()}, table));

                set_name = "";
            }
        }
    }
}
bool fun_vector_binary_search(const std::vector<int64_t>& con, const int64_t& search_key)
{
    return std::binary_search (con.begin(), con.end(), search_key);
}
template<typename T> bool fun_map_search(const T& con, const int64_t& search_key)
{
    auto res = con.find(search_key);
    // if(res == con.end())
    // {
    //     std::cout <<"nunllptr,key:"<< search_key;
    // }
    return true;
}
template<typename T, typename K> bool fun_vector_search(const T& con, const K& search_key)
{
    auto res = std::find(con.begin(), con.end(), search_key);
    // if(res != con.end())
    // {
    //     std::cout <<"nunllptr,key:"<< search_key;
    // }
    return true;
}
template<typename T,typename K> bool fun_map_search(const T& con, const K& search_key)
{
    auto res = con.find(search_key);
    if(res != con.end())
    {
        //std::cout <<" found";
    }
    return true;
}
void rand_bench_skiplist_range_search(int iters, kn::db::core::Table* table,const std::vector<std::pair<int64_t, int64_t>>& search_key)
{
    folly::BenchmarkSuspender braces;
     braces.dismissing([&] {
         while (iters--) {
             for(const auto& iter : search_key)
             {
                 auto res = table->Find(iter.first, iter.second);
                 if(!res.start_)
                 {
                     std::cout <<"start nunllptr";
                 }
                if(!res.end_)
                {
                std::cout <<"end nunllptr";
                } 
             }
             //folly::doNotOptimizeAway(base); 
         }
     });
}
template<typename T> bool fun_map_range_search(const T& con,const std::pair<int64_t, int64_t>& key)
{
    for(int64_t i = key.first; i < key.second; i++)
    {
        auto res = con.find(i);
        if(res != con.end())
        {
            //std::cout <<" found";
        }
    }
    return true;
}
bool fun_vector_binary_range_search(const std::vector<int64_t>& con, const std::pair<int64_t, int64_t>& key)
{
    for(int64_t i = key.first; i < key.second; i++)
    {
        std::binary_search (con.begin(), con.end(), i);
    }
    return true;
}
#endif