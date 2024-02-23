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

#include "SatelliteDistribution.h"

#include "Basics/debugging.h"

#include <random>

using namespace arangodb;

SatelliteDistribution::SatelliteDistribution() {
  _shardToServerMapping.reserve(1);
}

auto SatelliteDistribution::checkDistributionPossible(
    std::vector<ServerID>& availableServers) -> Result {
  return {};
}

auto SatelliteDistribution::planShardsOnServers(
    std::vector<ServerID> availableServers,
    std::unordered_set<ServerID>& serversPlanned) -> Result {
  // Caller needs to ensure we have something to place shards on
  TRI_ASSERT(!availableServers.empty());
  _shardToServerMapping.clear();
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(availableServers.begin(), availableServers.end(), g);
  for (auto const& s : availableServers) {
    // Satellites use all!
    serversPlanned.emplace(s);
  }
  // We simply place ourselfs on all servers, a random one shall be the leader
  _shardToServerMapping.emplace_back(
      ResponsibleServerList{std::move(availableServers)});
  // This can never fail.
  return TRI_ERROR_NO_ERROR;
}
