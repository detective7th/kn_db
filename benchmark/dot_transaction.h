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
#include "db_core/data_base.h"
#include "db_service/data_def.h"
#include "skiplist.hpp"
#include "btree.h"
namespace ndt
{
using namespace folly;
using namespace kn::db::core;
using namespace kn::db::service;
std::shared_ptr<folly::MemoryMapping> file_mapping = nullptr;
template<typename T>bool fun_vector_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(Transaction);
    if(size <=0 || (file_data.size() % sizeof(Transaction) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        Transaction* dot = (Transaction*)(file_data.start() + i*sizeof(Transaction));
        con.emplace_back(((dot->trade_time_/100000000) << 32)|(dot->transaction_no_));
    }
    return true;
}
template<typename T>bool fun_map_insert(const folly::StringPiece& file_data, T& con)
{
    int size = file_data.size()/sizeof(Transaction);
    if(size <=0 || (file_data.size() % sizeof(Transaction) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return false;
    }
    for(int i = 0; i < size; i++)
    {
        Transaction* dot = (Transaction*)(file_data.start() + i*sizeof(Transaction));
        con.emplace(((dot->trade_time_/100000000) << 32)|(dot->transaction_no_), dot);
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
        "insert",
        [=](int iters) {
            rand_bench_com<std::vector<int64_t>>(iters ,file_data,fun_vector_insert<std::vector<int64_t>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "list_insert",
        [=](int iters) {
            rand_bench_com<std::list<int64_t>>(iters ,file_data,fun_vector_insert<std::list<int64_t>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "map_insert",
        [=](int iters) {
            rand_bench_com<std::map<int64_t, Transaction*>>(iters ,file_data,fun_map_insert<std::map<int64_t, Transaction*>>);
            return iters;
        });
    addBenchmark(
        __FILE__,
        "unordered_map_insert",
        [=](int iters) {
            rand_bench_com<std::unordered_map<int64_t, Transaction*>>(iters ,file_data,fun_map_insert<std::unordered_map<int64_t, Transaction*>>);
            return iters;
        });
}
void set_search_bench_single(const std::string& path,int mutiple = 0)
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
    int size = file_data.size()/sizeof(Transaction);
    if(size <=0 || (file_data.size()% sizeof(Transaction) != 0 ))
    {
        std::cout << "file content error;" <<std::endl;
        return;
    }
    std::cout <<"file: "<<__FILE__<< "single size : " << size << std::endl;
    std::vector<int64_t> test;
    std::list<int64_t> testlist;
    std::map<int64_t, std::shared_ptr<DataNode>> test_map;
    std::unordered_map<int64_t, std::shared_ptr<DataNode>> test_hash_map;
    guoxiao::skiplist::SkipList<int64_t, std::shared_ptr<DataNode>> test_skip_list_trans;
    static trees::BTree<int64_t, std::shared_ptr<DataNode>> test_btree_trans(64);
    static art::radix_map<int64_t, std::shared_ptr<DataNode>> test_art_trans;
    stx::btree_map<int64_t, std::shared_ptr<DataNode>> test_bplustree;
    uint64_t no = 0;
    std::vector<int64_t> search_key;
    for(int i = 0; i < size; i++)
    {
        //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(Transaction), sizeof(Transaction)));
        Transaction* dot = (Transaction*)(file_data.start() + i*sizeof(Transaction));
        no = kn::db::service::CombineHiLow<kn::db::core::KeyType, uint32_t>
        (static_cast<uint32_t>(dot->trade_time_/1000000000)
         , static_cast<uint32_t>(dot->transaction_no_));
         search_key.emplace_back(no);
        //  if(85899779711872708ul == no)
        //  {
        //      int test = 1;
        //      test = 0;
        //  }
         auto ptr = std::make_shared<DataNode>((void*)dot,sizeof(Transaction),no);
         test.emplace_back(no);
         testlist.emplace_back(no);
         test_map.emplace(no, ptr);
         test_hash_map.emplace(no, ptr);
         //test_skip_list_trans.emplace(no, ptr);
         test_btree_trans.insert(no, ptr);
         test_bplustree.insert(no, ptr);
         test_art_trans.emplace(no, ptr);
    }
    // addBenchmark(
    //     __FILE__,
    //     "vector",
    //     [=](int iters) {
    //         rand_search_bench_com<std::vector<int64_t>, int64_t>(iters ,test, search_key, fun_vector_search<std::vector<int64_t>,int64_t>);
    //         return iters;
    //     });
    // addBenchmark(
    //         __FILE__,
    //         "list",
    //         [=](int iters) {
    //             rand_search_bench_com<std::list<int64_t>, int64_t>(iters ,testlist, search_key, fun_vector_search<std::list<int64_t>,int64_t>);
    //             return iters;
    //         });
    if(!mutiple)
    {
        search_key = rand_count_in_vec(search_key);
        std::cout<< "search key size:" << search_key.size() << std::endl;  
        const std::string test_name("transaction_test");
        addBenchmark(
            test_name.c_str(),
            "map",
            [=](int iters) {
                rand_search_bench_com<std::map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_map, search_key, fun_map_search<std::map<int64_t, std::shared_ptr<DataNode>>,int64_t>);
                return iters;
            });    
        addBenchmark(
            test_name.c_str(),
            "unordered_map",
            [=](int iters) {
                rand_search_bench_com<std::unordered_map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_hash_map, search_key, fun_map_search<std::unordered_map<int64_t, std::shared_ptr<DataNode>>,int64_t>);
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
                    rand_search_bench_com<guoxiao::skiplist::SkipList<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_skip_list_trans, search_key, fun_map_search<guoxiao::skiplist::SkipList<int64_t,std::shared_ptr<DataNode>>, int64_t>);
                    return iters;
                });
            addBenchmark(
                test_name.c_str(),
                "btree",
                [=](int iters) {
                    rand_search_bench_com<trees::BTree<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_btree_trans, search_key, fun_map_search<trees::BTree<int64_t,std::shared_ptr<DataNode>>, int64_t>);
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
                    rand_search_bench_com< art::radix_map<int64_t, std::shared_ptr<DataNode>>, int64_t>(iters ,test_art_trans, search_key, fun_map_search< art::radix_map<int64_t,std::shared_ptr<DataNode>>, int64_t>);
                    return iters;
                }); 
    }
    else
    {
        auto search_range = rand_rang_count_in_vec(search_key, mutiple);
        std::cout << "tranaction_test ,total size : "<< search_range.size() <<"| range "<< mutiple * kMinLimtCount << std::endl;
        const std::string test_name("transaction_range_test");
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
                    rand_search_bench_com<guoxiao::skiplist::SkipList<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_skip_list_trans, search_range, fun_map_range_search<guoxiao::skiplist::SkipList<int64_t,std::shared_ptr<DataNode>>>);
                    return iters;
                });
            addBenchmark(
                test_name.c_str(),
                "btree",
                [=](int iters) {
                    rand_search_bench_com<trees::BTree<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_btree_trans, search_range, fun_map_range_search<trees::BTree<int64_t,std::shared_ptr<DataNode>>>);
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
                    rand_search_bench_com< art::radix_map<int64_t, std::shared_ptr<DataNode>>, std::pair<int64_t,int64_t>>(iters ,test_art_trans, search_range, fun_map_range_search< art::radix_map<int64_t,std::shared_ptr<DataNode>>>);
                    return iters;
                });
    }   
}
    std::vector<int64_t> rand_search_key(const std::string& path)
    {
        //std::string path = "/home/hzs/SSE/kn_db/data/transactions/000002";
        std::shared_ptr<folly::MemoryMapping> tmp_mapping = nullptr;
        file_mapping = std::make_shared<folly::MemoryMapping>(path.c_str());

        folly::StringPiece file_data;
        file_data = file_mapping->data();
        file_data = file_data.subpiece(sizeof(uint32_t));
        
        int size = file_data.size()/sizeof(Transaction);
        std::vector<int64_t> con;
        if(size <=0 || (file_data.size()% sizeof(Transaction) != 0 ))
        {
            std::cout << "file content error;" <<std::endl;
            return con;
        }
        uint64_t no = 0;
        for(int i = 0; i < size; i++)
        {
            //folly::StringPiece onepiece(file_data.subpiece(i*sizeof(Transaction), sizeof(Transaction)));
            Transaction* dot = (Transaction*)(file_data.start() + i*sizeof(Transaction));
            no = kn::db::service::CombineHiLow<kn::db::core::KeyType, uint32_t>
            (static_cast<uint32_t>(dot->trade_time_/1000000000)
             , static_cast<uint32_t>(dot->transaction_no_));
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
    void set_search_skiplist(const std::string& path, kn::db::core::DataBase& base, int mutiple = 0)
    {
        auto table = base.GetSet("transactions")->GetTable("000002").get();
        auto search_key = rand_search_key(path);
        if(mutiple == 0)
        {
            search_key = rand_count_in_vec(search_key);
            std::cout << "tranaction_test ,total size : "<< search_key.size() << std::endl;
            addBenchmark(
                "transaction_test",
                "kn_db",
                [=](int iters) {
                    rand_bench_skiplist_search(iters , table, search_key);
                    return iters;
                });
        }
        else
        {
            auto search_range = rand_rang_count_in_vec(search_key, mutiple);
            std::cout << "tranaction_test ,total size : "<< search_key.size()<<"|search size:" << search_range.size() <<"| range "<< mutiple * kMinLimtCount << std::endl;
            addBenchmark(
                "transaction_test",
                "kn_db_range",
                [=](int iters) {
                    rand_bench_skiplist_range_search(iters , table, search_range);
                    return iters;
                });
        }
    } 
}// namespace ndt
#endif
