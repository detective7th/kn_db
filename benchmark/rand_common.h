#ifndef _TEST_RAND_COMMON_H_
#define _TEST_RAND_COMMON_H_
#include <vector>
#include <time.h>
#include"boost/format.hpp"
const int kSearchCount = 1000000;
enum OrderType 
{
    OT_UNKNOWN = 0,
    OT_MARKET = 1,
    OT_LIMIT = 2,
    OT_BEST = 3
};

enum Direction 
{
    UNKNOW = 0,
    BID = 1,
    ASK = 2
};
std::vector<int64_t> rand_count()
{
    std::vector<int64_t> vec;
    srand(time(0));
    for(int i = 0; i < kSearchCount; i ++)
    {
        vec.emplace_back(rand() % 10000000);
    }
    return vec;
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
#endif