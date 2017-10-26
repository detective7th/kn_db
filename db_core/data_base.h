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

#ifndef _KN_DB_CORE_DATA_BASE_H_
#define _KN_DB_CORE_DATA_BASE_H_

#pragma once

#include <folly/experimental/StringKeyedUnorderedMap.h>
#include <folly/FBString.h>
#include "set.h"

namespace kn
{
namespace db
{
namespace core
{

class DataBase
{
public:
    DataBase(const folly::fbstring& name)
            :name_(name) {}

    auto AddSet(std::shared_ptr<Set>& set)
    {
        return sets_.insert({set->name(), set}).second;
    }

    bool AddTable(const folly::StringPiece& set_name, std::shared_ptr<Table>& table)
    {
        auto it = sets_.find(set_name);
        if (it != sets_.end())
        {
            return it->second->AddTable(table);
        }
        return false;
    }

    std::shared_ptr<DataNode> Find(folly::StringPiece set_name, folly::StringPiece table_name
                                   , KeyType key)
    {
        auto it = sets_.find(set_name);
        if (it != sets_.end())
        {
            return it->second->Find(table_name, key);
        }
        return nullptr;
    }

protected:
    folly::StringKeyedUnorderedMap<std::shared_ptr<Set>> sets_;
    folly::fbstring name_;
};
} //core
} //db
} //kn
#endif //_KN_DB_CORE_DATA_BASE_H_
