#include "db_core/skiplist.h"

int main()
{
    kn::db::core::SkipList list(2, 2);
    std::cout << list << std::endl;
    void* data = malloc(32);

    for (int i = 0; i != 32; ++i)
    {
        auto data_node = std::make_shared<kn::db::core::DataNode>(data, i, i);
        list.Insert(data_node);
    }
    std::cout << list << std::endl;

    for (int i = 0; i != 32; ++i)
    {
        auto find_node = list.Find(i);
        std::cout << "data=" << find_node->data() << "|len=" << find_node->data_len() << std::endl;
    }

    return 0;
}
