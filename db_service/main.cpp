#include <sstream>
#include <numeric>
#include <boost/filesystem.hpp>
#include <gflags/gflags.h>
#include <folly/experimental/StringKeyedUnorderedMap.h>
#include "data_def.h"
#include "db_core/data_base.h"

DEFINE_string(data_path, "./data", "data path");

int main(int argc, char* argv[])
{
    std::stringstream version_str;

#ifdef VERSION
    version_str << VERSION;
#endif
    //__m256i avx_sreg2 = _mm256_set_epi64x(5, 0, 0, 0); //_mm256_setzero_si256();
    //__m256i avx_creg2 = _mm256_set_epi64x(4, 3, 2, 1);
    //__m256i res = _mm256_cmpgt_epi64(avx_sreg2, avx_creg2);
    //uint32_t bitmask = _mm256_movemask_epi8(res);

    //return 0;

    //kn::db::core::SkipList sl(2, 2);

    //size_t datas[32] = {0};
    //std::iota(std::begin(datas), std::end(datas), 0);
    //for (size_t i = 0; i != sizeof(datas)/sizeof(size_t); ++i)
    //{
    //    auto tmp = new kn::db::core::DataNode(datas + i, sizeof(size_t), i);
    //    sl.Insert(tmp);
    //}

    //std::cout << sl << std::endl;

    //auto ret = sl.Find(2, 15);
    //std::cout << ret << std::endl;

    //return 0;

    gflags::SetVersionString(version_str.str());
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    boost::filesystem::path data_path(FLAGS_data_path);
    if (!boost::filesystem::exists(data_path))
    {
        std::cerr << "wrong data path" << std::endl;
        return -1;
    }
    if (!boost::filesystem::is_directory(data_path))
    {
        std::cerr << "wrong data path" << std::endl;
        return -2;
    }

    folly::fbstring set_name;
    kn::db::core::DataBase base{"test"};
    std::vector<uint64_t> orders_keys;
    for (boost::filesystem::recursive_directory_iterator it(data_path);
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
                        (static_cast<uint32_t>(o->trade_time_/1000000000), static_cast<uint32_t>(o->order_no_));
                    };

                    folly::MemoryMapping::Options o;
                    o.setWritable(false);
                    o.setGrow(false);
                    o.setShared(true);
                    auto file_mapping
                            = std::make_unique<folly::MemoryMapping>(it->path().native().c_str(), 0, -1, o);
                    folly::StringPiece data = file_mapping->data();
                    auto row_len = *((uint32_t*)data.begin());
                    data.advance(sizeof(uint32_t));
                    auto row_num = data.size()/row_len;

                    for (size_t i = 0; i != row_num; ++i)
                    {
                        kn::db::service::Order* o = (kn::db::service::Order*)data.begin();
                        orders_keys.push_back(kn::db::service::CombineHiLow<kn::db::core::KeyType, uint32_t>
                                              (static_cast<uint32_t>(o->trade_time_/1000000000)
                                               , static_cast<uint32_t>(o->order_no_)));
                        data.advance(row_len);
                    }
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
                else continue;

                auto table = std::make_shared<kn::db::core::Table>(it->path()
                                                                   , it->path().leaf().string().c_str()
                                                                   , hash_func);

                table->InitSkipList(8, 6);

                assert(base.AddTable({set_name.begin(), set_name.end()}, table));

                set_name = "";
            }
        }
    }

    auto table = base.GetSet("transactions")->GetTable("000002");

    return 0;
}
