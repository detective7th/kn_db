/*
 * Copyright (c) 2017, kid Novalis
 * All rights reserved.
 *
 * Email: kid_1412_94@hotmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _KN_DB_CACHE_SENSITIVE_LINE_SKIPLIST_H_
#define _KN_DB_CACHE_SENSITIVE_LINE_SKIPLIST_H_

#pragma once

constexpr auto MAX_SKIP = 5;
constexpr auto TOP_LANE_BLOCK = 16;
constexpr auto SIMD_SEGMENTS = 8;

#include <stdint.h>
#include <memory>
#include <limits>
#include <iostream>
#include <algorithm>
#include <assert.h>

namespace kn
{
namespace db
{
namespace core
{

using KeyType = uint32_t;

class DataNode
{
    friend class Lane;
    friend class SkipList;
    friend class ProxyNode;

public:
    DataNode() = default;
    DataNode(void* data, size_t data_len, uint32_t key)
            :data_(data)
            ,data_len_(data_len)
            ,key_(key) {}

    virtual ~DataNode()
    {
        //free(data_);
        data_len_ = 0;
        key_ = 0;
        next_ = nullptr;
    }

    friend inline std::ostream& operator << (std::ostream& os, DataNode& data)
    {
        if (data.data_) os << "[" << data.key_ << "]";
        return os;
    }

    auto data_len() { return data_len_; }
    auto data() { return data_; }

protected:
    void* data_ {nullptr};
    size_t data_len_{0};
    uint32_t key_ {0};
    struct std::shared_ptr<DataNode> next_ {nullptr};
};

class ProxyNode
{
    friend class ProxyLane;

public:
    ProxyNode(std::shared_ptr<DataNode>& node, uint8_t skip)
            //:keys_(new KeyType[skip], std::default_delete<KeyType[]>())
            :skip_(skip)
            ,nodes_(new std::shared_ptr<DataNode>[skip_])
    {
        //std::fill_n(keys_.get(), skip, std::numeric_limits<KeyType>::max());
        std::fill_n(nodes_.get(), skip, nullptr);
        //keys_[0] = node->key_;
        nodes_[0] = node;
    }

    bool Add(std::shared_ptr<DataNode>& node)
    {
        decltype(skip_) pos_add = 0;
        for (;pos_add != skip_; ++pos_add)
        {
            if (!nodes_[pos_add]) break;
            if (nodes_[pos_add]->key_ == node->key_)
            {
                //std::cerr << "proxy lane key collision" << std::endl;
                //return false;
            }
        }
        if (skip_ == pos_add)
        {
            std::cerr << "proxy lane full" << std::endl;
            return false;
        }

        nodes_[pos_add] = node;

        return true;
    }

    friend inline std::ostream& operator << (std::ostream& os, ProxyNode& node)
    {
        for (decltype(node.skip_) i = 0; i != node.skip_; ++i)
        {
            if (node.nodes_[i]) os << *(node.nodes_[i]); else os << "[n]";
        }
        //os << "|";
        return os;
    }

    std::shared_ptr<DataNode> Search(KeyType key)
    {
        for (uint8_t i = 0; i != skip_; ++i)
        {
            if (nodes_[i]->key_ == key) return nodes_[i];
        }
        return nullptr;
    }

protected:
    //std::unique_ptr<KeyType[]> keys_ {nullptr};
    uint8_t skip_{0};
    std::unique_ptr<std::shared_ptr<DataNode>[]> nodes_ {nullptr};
    bool updated_{false};
};

class ProxyLane
{
public:
    ProxyLane(uint32_t slot_num, uint8_t skip)
            :slot_num_(slot_num)
            ,skip_(skip)
            ,nodes_(new std::unique_ptr<ProxyNode>[slot_num_]())
    {
        std::fill_n(nodes_.get(), slot_num, nullptr);
    }

    void Set(uint32_t pos, std::shared_ptr<DataNode>& data)
    {
        nodes_[pos].reset(new ProxyNode(data, skip_));
    }

    bool Add(uint32_t start, std::shared_ptr<DataNode>& data)
    {
        std::unique_ptr<ProxyNode>& proxy_node = nodes_[start];
        assert(proxy_node);
        if (proxy_node) return proxy_node->Add(data);
        return false;
    }

    friend inline std::ostream& operator << (std::ostream& os, ProxyLane& proxy_lane)
    {
        os << "proxy|slot_num_=" << proxy_lane.slot_num_
           << "|skip=" << uint32_t(proxy_lane.skip_ )
           << std::endl;

        for (decltype(proxy_lane.slot_num_) i = 0; i != proxy_lane.slot_num_; ++i)
        {
            if (proxy_lane.nodes_[i]) os << *(proxy_lane.nodes_[i]);
            else os << "na";
            os << "|";
        }
        return os;
    }

    auto Search(uint32_t pos, KeyType key)
    {
        return nodes_[pos]->Search(key);
    }

protected:
    uint32_t slot_num_{0};
    uint8_t skip_{0};
    std::unique_ptr<std::unique_ptr<ProxyNode>[]> nodes_{nullptr};
};

class Lane
{
    friend class Lanes;
    friend class SkipList;

public:
    Lane() = default;
    explicit Lane(uint32_t slot_num, uint32_t start, uint8_t level, uint8_t skip)
            :slot_num_(slot_num)
            ,start_(start)
            ,level_(level)
            ,skip_(skip)
            ,keys_(new KeyType[slot_num_](), std::default_delete<KeyType[]>())
    {
        InitProxyPointers();
        std::fill_n(keys_.get(), slot_num_, std::numeric_limits<KeyType>::max());
    }

    void Init(uint32_t slot_num, uint32_t start, uint8_t level, uint8_t skip)
    {
        keys_ = nullptr;
        slot_num_ = slot_num;
        elements_ = 0;
        start_ = start;
        level_ = level;
        skip_ = skip;
        InitProxyPointers();
        keys_ = std::unique_ptr<KeyType[]>(new KeyType[slot_num_]()
                                           , std::default_delete<KeyType[]>());
        std::fill_n(keys_.get(), slot_num_, std::numeric_limits<KeyType>::max());
    }

    friend inline std::ostream& operator << (std::ostream& os, Lane& lane)
    {
        os << "lane(" << uint32_t(lane.level_) << ")" << "|slot_num=" << lane.slot_num_
           << "|start_idx=" << lane.start_ << "|elements_num=" << lane.elements_ << std::endl;

        for (decltype(lane.slot_num_) i = 0; i != lane.slot_num_; ++i)
        {
            if (std::numeric_limits<KeyType>::max() == lane.keys_[i]) os << "na"; else os << lane.keys_[i];
            os << "|";
        }
        if (lane.proxy_) os << std::endl << *(lane.proxy_);
        return os;
    }

    auto InsertElement(std::shared_ptr<DataNode>& new_node)
    {
        auto cur_pos = /*start_ +*/ elements_;
        auto pos_limit = /*cur_pos + */slot_num_ - 1;

        //if (cur_pos > pos_limit) cur_pos = pos_limit;

        while (new_node->key_ > keys_[cur_pos] && cur_pos < pos_limit) ++cur_pos;

        if (std::numeric_limits<KeyType>::max() == keys_[cur_pos])
        {
            keys_[cur_pos] = new_node->key_;
            if (0 == level_) proxy_->Set(cur_pos /*- start_*/, new_node);
            ++elements_;
        }
        else return std::numeric_limits<KeyType>::max();

        return cur_pos + start_;
    }

    bool InsertIntoProxy(std::shared_ptr<DataNode>& new_node)
    {
        if (proxy_) return proxy_->Add(elements_ - 1, new_node);
        return false;
    }

    auto BinarySearch(uint32_t key)
    {
        uint32_t cur_pos = 0;
        uint32_t first = 0;
        uint32_t last = elements_ - 1;
        while (first < last)
        {
            auto middle = (first + last) / 2;
            if (keys_[middle] < key)
            {
                first = middle + 1;
                cur_pos = middle;
            }
            else if (keys_[middle] == key)
            {
                cur_pos = middle;
                break;
            }
            else
            {
                last = middle;
            }
        }
        if (first > last) cur_pos = last;

        return cur_pos;
    }

    std::shared_ptr<DataNode> SearchProxyLane(uint32_t pos, uint32_t key)
    {
        if (proxy_) return proxy_->Search(pos, key);
        return nullptr;
    }

protected:
    void InitProxyPointers()
    {
        if (0 == level_) proxy_ = std::make_unique<ProxyLane>(slot_num_, skip_);
    }

protected:
    uint32_t slot_num_{0};
    uint32_t elements_{0};
    uint32_t start_{0}; // abs start idx
    uint8_t level_{0};
    uint8_t skip_{0};
    std::unique_ptr<ProxyLane> proxy_{nullptr};
    std::unique_ptr<KeyType[]> keys_{nullptr};
};

class Lanes
{
    friend class SkipList;

public:
    explicit Lanes(uint8_t max_level, uint8_t skip)
            :max_level_(max_level >= 2 ? max_level : 2)
            ,skip_(skip > 1 ? skip : 2)
            ,lanes_(new Lane[max_level_], std::default_delete<Lane[]>())
    {
        total_slots_size_ = TOP_LANE_BLOCK;
        lanes_[max_level_ - 1].Init(total_slots_size_, 0, max_level_ - 1, skip_);
        for (int8_t level = max_level_ - 2; 0 <= level; --level)
        {
            auto& next_lane = lanes_[level + 1];
            lanes_[level].Init(next_lane.slot_num_ * skip_, next_lane.start_ + next_lane.slot_num_, level
                               , skip_);
            total_slots_size_ += lanes_[level].slot_num_;
        }
    }

    friend inline std::ostream& operator << (std::ostream& os, Lanes& lanes)
    {
        os << "lanes|max_level_=" << uint32_t(lanes.max_level_) << "|skip=" << uint32_t(lanes.skip_)
           << "|total_elements=" << lanes.total_elements_
           << "|total_slots_size=" << uint32_t(lanes.total_slots_size_) << std::endl;

        for (int8_t i = lanes.max_level_ - 1; 0 <= i; --i) os << lanes.lanes_[i] << std::endl;

        return os;
    }

    auto InsertElement(std::shared_ptr<DataNode>& new_node)
    {
        bool lane_instered {false};
        bool node_inserted {true};

        for (decltype(max_level_) level = 0; level != max_level_; ++level)
        {
            if ((0 == (total_elements_ % static_cast<uint32_t>(pow(skip_, level + 1)))) && node_inserted)
            {
                node_inserted = lanes_[level].InsertElement(new_node);
            }
            else break;

            lane_instered = true;
        }

        if (!lane_instered) if (!lanes_[0].InsertIntoProxy(new_node)) return false;

        ++total_elements_;

        if (0 == (total_elements_ % (TOP_LANE_BLOCK * static_cast<uint32_t>(pow(skip_, max_level_)))))
        {
            ResizeLanes();
        }

        return true;
    }

    auto Find(KeyType key)
    {
        //cur_pos is abs idx
        auto cur_pos = lanes_[max_level_ - 1].BinarySearch(key);
        uint32_t r_pos = 0;
        for (auto level = max_level_ - 1; 0 <= level; --level)
        {
            auto& lane = lanes_[level];
            r_pos = cur_pos - lane.start_;
            while (r_pos < lane.elements_ - 1 && key > lane.keys_[r_pos] && key >= lane.keys_[r_pos + 1])
            {
                ++r_pos;
            }
            if (0 == level) break;
            cur_pos = lanes_[level - 1].start_ + r_pos * lanes_[level - 1].skip_;
        }

        return lanes_[0].SearchProxyLane(r_pos, key);
    }

protected:
    void ResizeLanes()
    {
        //auto new_size = lanes_[max_level_ - 1].slot_num_ + TOP_LANE_BLOCK;
    }

protected:
    uint8_t max_level_{5};
    uint8_t skip_{2};
    size_t total_slots_size_{0};
    size_t total_elements_{0};
    std::unique_ptr<Lane[]> lanes_{nullptr};
};

class SkipList
{

public:
    SkipList(uint8_t max_level, uint8_t skip)
            :head_(std::make_shared<DataNode>())
            ,lanes_(std::make_unique<Lanes>(max_level, skip))
    {
        tail_= head_;
    }

    friend inline std::ostream& operator << (std::ostream& os, SkipList& list)
    {
        os << "skiplist|head=" << list.head_ << "|tail=" << list.tail_ << std::endl;
        os << *(list.lanes_);
        return os;
    }

    bool Insert(std::shared_ptr<DataNode>& new_node)
    {
        if (lanes_->InsertElement(new_node))
        {
            tail_ = new_node;
            return true;
        }
        return false;
    }

    auto Find(uint32_t key)
    {
        return lanes_->Find(key);
    }

protected:
    std::shared_ptr<DataNode> head_{nullptr};
    std::shared_ptr<DataNode> tail_{nullptr};
    std::unique_ptr<Lanes> lanes_{nullptr};
};

struct RangeSearchResult
{
    DataNode* start_ {nullptr};
    DataNode* end_ {nullptr};
    uint32_t count_ {0};
};

} //core
} //db
} //kn

#endif // _KN_DB_CACHE_SENSITIVE_LINE_SKIPLIST_H_