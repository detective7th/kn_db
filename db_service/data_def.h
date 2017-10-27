#ifndef _KN_DB_SERVICE_DATA_DEF_H_
#define _KN_DB_SERVICE_DATA_DEF_H_

#include <assert.h>
#include <stdint.h>

namespace kn
{
namespace db
{
namespace service
{

struct Order
{
    double trade_vol_{0};
    int direction_{0};
    double price_{0};
    uint64_t trade_time_{0};
    int order_type_{0};
    int64_t order_no_{0};
};

struct Transaction
{
    double trade_vol_{0};
    int direction_{0};
    double price_{0};
    int64_t ask_order_no_{0};
    int64_t bid_order_no_{0};
    uint64_t trade_time_{0};
    int trade_type_{0};
    int64_t transaction_no_{0};
};
struct Candle
{
    double  high_;
    double  low_;
    double  open_;
    double  close_;
    int64_t open_time_;
    int64_t close_time_;
    double  vol_;

};
struct TimeShare
{
    int64_t time_{0};
    double last_{0.0};
    //double avg_{0.0};
    double vol_{0.0};
    //double total_vol_{0.0};
    //double turnover_{0.0};
};
template<class TypeRet, class TypeIn> TypeRet CombineHiLow(TypeIn hi, TypeIn low)
{
    assert(sizeof(TypeRet) == (sizeof(TypeIn) << 1));
    TypeRet ret = (TypeRet)hi;
    return ((ret << (sizeof(TypeIn) << 3)) ^ low);
}

} // service
} // db
} // kn
#endif // _KN_DB_SERVICE_DATA_DEF_H_
