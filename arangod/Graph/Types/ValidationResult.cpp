////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2024 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Business Source License 1.1 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     https://github.com/arangodb/arangodb/blob/devel/LICENSE
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Michael Hackstein
////////////////////////////////////////////////////////////////////////////////

#include "ValidationResult.h"

#include <iostream>

using namespace arangodb;
using namespace arangodb::graph;

// PRUNE and Filter are both pruned.
bool ValidationResult::isPruned() const noexcept {
  return _type == Type::PRUNE || _type == Type::FILTER_AND_PRUNE;
}

bool ValidationResult::isFiltered() const noexcept {
  return _type == Type::FILTER || _type == Type::FILTER_AND_PRUNE;
}

bool ValidationResult::isUnknown() const noexcept {
  return _type == Type::UNKNOWN;
}

void ValidationResult::combine(Type t) noexcept {
  switch (t) {
    case Type::TAKE:
      break;
    case Type::PRUNE:
      if (isFiltered()) {
        _type = Type::FILTER_AND_PRUNE;
      } else {
        _type = Type::PRUNE;
      }
      break;
    case Type::FILTER:
      if (isPruned()) {
        _type = Type::FILTER_AND_PRUNE;
      } else {
        _type = Type::FILTER;
      }
      break;
    case Type::FILTER_AND_PRUNE:
      _type = Type::FILTER_AND_PRUNE;
      break;
    case Type::UNKNOWN:
      break;
  }
}

std::ostream& arangodb::graph::operator<<(std::ostream& stream,
                                          ValidationResult const& res) {
  switch (res._type) {
    case ValidationResult::Type::TAKE:
      stream << "take";
      break;
    case ValidationResult::Type::PRUNE:
      stream << "prune";
      break;
    case ValidationResult::Type::FILTER:
      stream << "filter";
      break;
    case ValidationResult::Type::FILTER_AND_PRUNE:
      stream << "filter and prune";
      break;
    case ValidationResult::Type::UNKNOWN:
      stream << "unknown";
      break;
  }
  return stream;
}
