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
/// @author Max Neunhoeffer
////////////////////////////////////////////////////////////////////////////////

#include "CollectOptions.h"
#include "Basics/Exceptions.h"

#include <velocypack/Builder.h>
#include <velocypack/Slice.h>

using namespace arangodb::aql;

/// @brief constructor
CollectOptions::CollectOptions(VPackSlice slice)
    : method(CollectMethod::UNDEFINED) {
  VPackSlice v = slice.get("collectOptions");
  if (v.isObject()) {
    v = v.get("method");
    if (v.isString()) {
      method = methodFromString(v.stringView());
    }
  }
}

/// @brief whether or not the method can be used
bool CollectOptions::canUseMethod(CollectMethod m) const {
  return (this->method == m || this->method == CollectMethod::UNDEFINED);
}

/// @brief whether or not the method should be used (i.e. is preferred)
bool CollectOptions::shouldUseMethod(CollectMethod m) const {
  return (this->method == m);
}

/// @brief convert the options to VelocyPack
void CollectOptions::toVelocyPack(VPackBuilder& builder) const {
  VPackObjectBuilder guard(&builder);
  builder.add("method", VPackValue(methodToString(method)));
}

/// @brief get the aggregation method from a string
CollectOptions::CollectMethod CollectOptions::methodFromString(
    std::string_view method) {
  if (method == "hash") {
    return CollectMethod::HASH;
  }
  if (method == "sorted") {
    return CollectMethod::SORTED;
  }
  if (method == "distinct") {
    return CollectMethod::DISTINCT;
  }
  if (method == "count") {
    return CollectMethod::COUNT;
  }

  return CollectMethod::UNDEFINED;
}

/// @brief stringify the aggregation method
std::string_view CollectOptions::methodToString(
    CollectOptions::CollectMethod method) {
  if (method == CollectMethod::HASH) {
    return "hash";
  }
  if (method == CollectMethod::SORTED) {
    return "sorted";
  }
  if (method == CollectMethod::DISTINCT) {
    return "distinct";
  }
  if (method == CollectMethod::COUNT) {
    return "count";
  }

  THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL,
                                 "cannot stringify unknown aggregation method");
}

CollectOptions::CollectOptions() : method(CollectMethod::UNDEFINED) {}
