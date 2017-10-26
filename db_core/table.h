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

#ifndef _KN_DB_CORE_TABLE_H_
#define _KN_DB_CORE_TABLE_H_

#pragma once

#include <functional>
#include <boost/filesystem.hpp>
#include <folly/MemoryMapping.h>
#include <folly/FBString.h>
#include "skiplist.h"

namespace kn
{
namespace db
{
namespace core
{

class Table
{
public:
    explicit Table(const boost::filesystem::path& path, const char* name
                   , std::function<KeyType(void*, Table*)> hash_func)
            :name_(name)
            ,path_(path)
            ,hash_func_(hash_func)
    {}

    void InitSkipList(uint32_t max_level = 8, uint32_t skip = 4)
    {
        folly::MemoryMapping::Options o;
        o.setWritable(false);
        o.setGrow(false);
        o.setShared(true);
        file_mapping_ = std::make_unique<folly::MemoryMapping>(path_.native().c_str(), 0, -1, o);

        folly::StringPiece data = file_mapping_->data();
        row_len_ = *((uint32_t*)data.begin());
        data.advance(sizeof(uint32_t));
        auto row_num = data.size()/row_len_;

        skip_list_ = std::make_unique<SkipList>(max_level, skip);
        for (size_t i = 0; i != row_num; ++i)
        {
            auto node = std::make_shared<DataNode>((void*)data.begin(), row_len_, hash_func_((void*)data.begin()
                                                                                             , this));
            skip_list_->Insert(node);
            data.advance(row_len_);
        }
    }
    folly::StringPiece name() { return {name_.begin(), name_.end()}; }

    auto Find(KeyType key)
    {
        return skip_list_->Find(key);
    }

protected:
    folly::fbstring name_;
    boost::filesystem::path path_;
    std::function<KeyType(void*, Table*)> hash_func_;
    std::unique_ptr<SkipList> skip_list_{nullptr};
    uint32_t row_len_{0};
    std::unique_ptr<folly::MemoryMapping> file_mapping_{nullptr};
};

} // core
} // db
} // core


#endif //_KN_DB_CACHE_SENSITIVE_LINE_SKIPLIST_H_
