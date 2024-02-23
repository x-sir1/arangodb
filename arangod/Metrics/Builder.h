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
/// @author Kaveh Vahedipour
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Metrics/Metric.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace arangodb::metrics {

class Builder {
 public:
  virtual ~Builder() = default;

  [[nodiscard]] virtual std::string_view type() const noexcept = 0;
  [[nodiscard]] virtual std::shared_ptr<Metric> build() const = 0;

  [[nodiscard]] std::string_view name() const noexcept;
  [[nodiscard]] std::string_view labels() const noexcept;

  void addLabel(std::string_view key, std::string_view value);

  void reserveSpaceForLabels(size_t bytes);

 protected:
  std::string_view _name;
  std::string _help;    // TODO(MBkkt) remove
  std::string _labels;  // TODO(MBkkt) const semantic
};

template<typename Derived>
class GenericBuilder : public Builder {
 public:
  Derived&& self() { return static_cast<Derived&&>(std::move(*this)); }

  Derived&& withLabel(std::string_view key, std::string_view value) && {
    Builder::addLabel(key, value);
    return self();
  }
};

}  // namespace arangodb::metrics
