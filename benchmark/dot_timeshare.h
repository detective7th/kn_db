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
#include "db_core/data_base.h"
#include "db_service/data_def.h"
#include "skiplist.hpp"
#include "btree.h"
#include "art/radix_map.h"
#include "stx/btree_map.h"
namespace nts 
{
const int kSecurityCodeLength = 6;
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
    int size = file_data.size()/sizeof(TimeShare);
    if(size <=0 || (file_data.size() % sizeof(TimeShare) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        TimeShare* dot = (TimeShare*)(file_data.start() + i*sizeof(TimeShare));
        con.emplace_back(kcode_id_ + std::to_string(dot->time_));
    }
    return true;
}
template<typename T>bool fun_map_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(TimeShare);
    if(size <=0 || (file_data.size() % sizeof(TimeShare) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        TimeShare* dot = (TimeShare*)(file_data.start() + i*sizeof(TimeShare));
        con.emplace(kcode_id_ + std::to_string(dot->time_), dot);
    }
    return true;
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
        file_data = file_data.subpiece(sizeof(uint32_t));
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
        "timesharing_vector_insert",
        [=](int iters) {
            rand_bench_com<std::vector<std::string>>(iters ,file_data, fun_vector_insert<std::vector<std::string>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "timesharing_list_insert",
        [=](int iters) {
            rand_bench_com<std::list<std::string>>(iters ,file_data, fun_vector_insert<std::list<std::string>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "timesharing_map_insert",
        [=](int iters) {
            rand_bench_com<std::map<std::string, TimeShare*>>(iters ,file_data, fun_map_insert<std::map<std::string, TimeShare*>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "timesharing_unordered_map_insert",
        [=](int iters) {
            rand_bench_com<std::unordered_map<std::string, TimeShare*>>(iters ,file_data, fun_map_insert<std::unordered_map<std::string, TimeShare*>>);
            return iters;
        });
}
void set_search_bench_single(std::string path, int mutiple = 0)
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
    int size = file_data.size()/sizeof(TimeShare);
    if(size <=0 || (file_data.size() % sizeof(TimeShare) != 0 ))
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
    uint64_t no = 0;
    std::vector<int64_t> search_key;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(TimeShare), sizeof(TimeShare)));
        TimeShare* dot = (TimeShare*)(file_data.start() + i*sizeof(TimeShare));
        no = dot->time_;
        auto ptr = std::make_shared<DataNode>((void*)dot,sizeof(TimeShare),no);
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
    if(!mutiple)
    {
        search_key = rand_count_in_vec(search_key);
        //std::reverse(search_key.begin(), search_key.end());
        std::cout<< "search key size:" << search_key.size() << std::endl;
        const std::string test_name("timeshare_test");  
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
    else
    {
        auto search_range = rand_rang_count_in_vec(search_key, mutiple);
        std::cout << "timeshare_test ,total size : "<< search_range.size() <<"| range "<< mutiple * kMinLimtCount << std::endl;
        const std::string test_name("timeshare_range_test");
        addBenchmark(
            test_name.c_str(),
            "map",
            [=](int iters) {
                rand_search_bench_com<std::map<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_map, search_range, fun_map_range_search<std::map<int64_t, std::shared_ptr<DataNode>>>);
                return iters;
            });    
        addBenchmark(
            test_name.c_str(),
            "unordered_map",
            [=](int iters) {
                rand_search_bench_com<std::unordered_map<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_hash_map, search_range, fun_map_range_search<std::unordered_map<int64_t, std::shared_ptr<DataNode>>>);
                return iters;
            });
            addBenchmark(
                test_name.c_str(),
                "vector_binary",
                [=](int iters) {
                    rand_search_bench_com<std::vector<int64_t>, std::pair<int64_t,int64_t>>(iters ,test, search_range, fun_vector_binary_range_search);
                    return iters;
                });
            addBenchmark(
                test_name.c_str(),
                "tradition_skiplist",
                [=](int iters) {
                    rand_search_bench_com<guoxiao::skiplist::SkipList<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_skip_list, search_range, fun_map_range_search<guoxiao::skiplist::SkipList<int64_t,std::shared_ptr<DataNode>>>);
                    return iters;
                });
            addBenchmark(
                test_name.c_str(),
                "btree",
                [=](int iters) {
                    rand_search_bench_com<trees::BTree<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_btree, search_range, fun_map_range_search<trees::BTree<int64_t,std::shared_ptr<DataNode>>>);
                    return iters;
                });
            addBenchmark(
                test_name.c_str(),
                "bplustree",
                [=](int iters) {
                    rand_search_bench_com<stx::btree_map<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_bplustree, search_range, fun_map_range_search<stx::btree_map<int64_t,std::shared_ptr<DataNode>>>);
                    return iters;
                });
            addBenchmark(
                test_name.c_str(),
                "art tree",
                [=](int iters) {
                    rand_search_bench_com< art::radix_map<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_art, search_range, fun_map_range_search< art::radix_map<int64_t,std::shared_ptr<DataNode>>>);
                    return iters;
                });
    } 
     
}
std::vector<int64_t> rand_search_key()
{
    std::string path = "/home/kid/benckmark/kn_db/data/one_min/000002";
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
    uint64_t no = 0;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(Order), sizeof(Order)));
        TimeShare* dot = (TimeShare*)(file_data.start() + i*sizeof(TimeShare));
        no = dot->time_;
        con.emplace_back(no);
    }
    return con;
}
void rand_bench_skiplist_search(int iters, Table* table,const std::vector<int64_t>& search_key)
{
    folly::BenchmarkSuspender braces;
   
    braces.dismissing([&] {
        while (iters--) {
            for(const auto& iter : search_key)
            {
                table->Find(iter);
                // if(!res)
                // {
                //     std::cout <<"nunllptr,key:"<< iter;
                // }  
            }
            //folly::doNotOptimizeAway(base); 
        }
    });
}
void set_search_skiplist(kn::db::core::DataBase& base, int mutiple = 0)
{
    auto table = base.GetSet("one_min")->GetTable("000002").get();    
    auto search_key = rand_search_key();
    
    if(mutiple == 0)
    {
        search_key = rand_count_in_vec(search_key);
        std::cout << "timeshare_test ,total size : "<< search_key.size() << std::endl;
        std::cout<< "size:" << search_key.size() << std::endl;
        addBenchmark(
            "timeshare_test",
            "skiplist",
            [=](int iters) {
                rand_bench_skiplist_search(iters , table, search_key);
                return iters;
            });
    }
    else
    {
        // auto search_range = rand_rang_count_in_vec(search_key, mutiple);
        // std::cout << "timeshare_test ,total size : "<< search_key.size() <<"|search size:" << search_range.size() <<"| range "<< mutiple * kMinLimtCount << std::endl;
        // addBenchmark(
        //     "timeshare_test",
        //     "skiplist_range",
        //     [=](int iters) {
        //         rand_bench_skiplist_range_search(iters , table, search_range);
        //         return iters;
        //     });
    }
}
    

}// namespace tv

#endif