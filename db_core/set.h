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

#ifndef _KN_DB_CORE_SET_H_
#define _KN_DB_CORE_SET_H_

#pragma once

#include <folly/experimental/StringKeyedUnorderedMap.h>
#include <folly/FBString.h>
#include "table.h"

namespace kn
{
namespace db
{
namespace core
{

class Set
{
public:
    Set(const folly::fbstring& name)
            :name_(name) {}

    bool AddTable(std::shared_ptr<Table>& table)
    {
        return tables_.insert({table->name(), table}).second;
    }

    folly::StringPiece name() { return {name_.begin(), name_.end()}; }

    std::shared_ptr<DataNode> Find(folly::StringPiece table_name, KeyType key)
    {
        auto it = tables_.find(table_name);
        if (it != tables_.end())
        {
            return it->second->Find(key);
        }
        return nullptr;
    }

protected:
    folly::StringKeyedUnorderedMap<std::shared_ptr<Table>> tables_;
    folly::fbstring name_;
};
} //core
} //db
} //kn
#endif //_KN_DB_CORE_DATA_BASE_H_
