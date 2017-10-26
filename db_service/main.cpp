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
                else continue;

                auto table = std::make_shared<kn::db::core::Table>(it->path()
                                                                   , it->path().leaf().string().c_str()
                                                                   , hash_func);

                table->InitSkipList();

                assert(base.AddTable({set_name.begin(), set_name.end()}, table));

                set_name = "";
            }
        }
    }

    //base.Find("orders", "000001", );
    //kn::db::core::SkipList list(2, 2);
    //std::cout << list << std::endl;
    //void* data = malloc(32);

    //for (int i = 0; i != 512; ++i)
    //{
    //    auto data_node = std::make_shared<kn::db::core::DataNode>(data, i, i);
    //    list.Insert(data_node);
    //}
    //std::cout << list << std::endl;

    //for (int i = 0; i != 10; ++i)
    //{
    //    auto find_node = list.Find(i);
    //    if (find_node)
    //    {
    //        std::cout << "data=" << find_node->data() << "|len=" << find_node->data_len() << std::endl;
    //    }
    //}

    //auto nodes = list.Find(4, 12);
    //auto tmp = nodes.start_;
    //while(tmp != nodes.end_)
    //{
    //    std::cout << "data=" << tmp->data() << "|len=" << tmp->data_len() << std::endl;
    //    tmp = tmp->next();
    //}

    //return 0;

    return 0;
}
