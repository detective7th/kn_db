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

// weituo baodan
struct Order
{
    double trade_vol_{0}; // weituo shuliang
    int direction_{0}; // maimai fangxiang
    double price_{0}; // weituo jiage
    uint64_t trade_time_{0}; // weituo shijian
    int order_type_{0}; // weituo leixing
    int64_t order_no_{0}; // weituo danhao
};

// chengjiao baodan
struct Transaction
{
    double trade_vol_{0}; // chengjiao shuliang
    int direction_{0}; // chengjiao fangxiang
    double price_{0}; // chengjiao jiage
    int64_t ask_order_no_{0}; // maifang weituo danhao
    int64_t bid_order_no_{0}; // maifang weituo danhao
    uint64_t trade_time_{0}; // chengjiao shijian
    int trade_type_{0}; // chengjiao leixing
    int64_t transaction_no_{0}; // chengjiao danhao
};

// k xian tu
struct Candle
{
    double  high_; // zui gao jia
    double  low_; // zui di jia
    double  open_; // kai pan jia
    double  close_; // shou pan jia
    int64_t open_time_; // kaishi shijian
    int64_t close_time_; // jiesu shijian
    double  vol_; // chengjiao liang

};

// fen shi tu
struct TimeShare
{
    int64_t time_{0}; // kaishi shijian
    double last_{0.0}; // zui xin jia
    double vol_{0.0}; // chengjiao liang
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
