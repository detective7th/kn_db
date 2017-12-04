#include <sstream>
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

    std::cout << "memory(MB):" << table->total_memory() / 1024 / 1024 << std::endl;
    auto res = table->Find(85900655892627062);
    std::cout << res << std::endl;

    //for (auto key : orders_keys)
    //{
    //    auto data_node = table->Find(key);
    //    assert(data_node);
    //}

    //auto nodes = table->Find(orders_keys[10], orders_keys[300]);
    //assert(nodes.start_);
    //assert(nodes.end_);
    return 0;
}
