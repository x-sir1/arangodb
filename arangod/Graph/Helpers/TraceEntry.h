////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2020-2020 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
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
/// @author Heiko Kernbach
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_GRAPH_HELPERS_TRACE_ENTRY_H
#define ARANGOD_GRAPH_HELPERS_TRACE_ENTRY_H 1

#include <numeric>
#include <ostream>

namespace arangodb {
namespace graph {

class TraceEntry {
 public:
  TraceEntry();
  ~TraceEntry();

  void addTiming(double timeTaken);

  friend auto operator<<(std::ostream& out, TraceEntry const& entry) -> std::ostream&;

 private:
  double _min{std::numeric_limits<double>::max()};
  double _max{0};
  double _total{0};
  uint64_t _count{0};
};

}  // namespace graph
}  // namespace arangodb

#endif