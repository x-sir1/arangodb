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
/// @author Dr. Frank Celler
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GeneralServer/RestHandler.h"

#include "Rest/GeneralResponse.h"

namespace arangodb {
namespace transaction {
class Context;
}

namespace velocypack {
class Builder;
struct Options;
class Slice;
}  // namespace velocypack

class RestBaseHandler : public rest::RestHandler {
 public:
  explicit RestBaseHandler(ArangodServer&, GeneralRequest*, GeneralResponse*);

  void handleError(basics::Exception const&) override;

 public:
  // generates a result from VelocyPack
  template<typename Payload>
  void generateResult(rest::ResponseCode, Payload&&);

  // generates a result from VelocyPack
  template<typename Payload>
  void generateResult(rest::ResponseCode, Payload&&,
                      velocypack::Options const*);

  // generates a result from VelocyPack
  template<typename Payload>
  void generateResult(rest::ResponseCode, Payload&&,
                      std::shared_ptr<transaction::Context> context);

  /// convenience function akin to generateError,
  /// renders payload in 'result' field
  /// adds proper `error`, `code` fields
  void generateOk(rest::ResponseCode, velocypack::Slice,
                  VPackOptions const& = VPackOptions::Defaults);

  /// Add `error` and `code` fields into your response
  void generateOk(rest::ResponseCode, velocypack::Builder const&);

  // generates a canceled message
  void generateCanceled();

  /// @brief generates not implemented
  void generateNotImplemented(std::string const& path);

  /// @brief generates forbidden
  void generateForbidden();

 protected:
  /// @brief parses the request body as VelocyPack, generates body
  velocypack::Slice parseVPackBody(bool& success);

  template<typename Payload>
  void writeResult(Payload&&, velocypack::Options const& options);

  /// @brief configure if outgoing responses will have the potential
  /// dirty reads header set:
  void setOutgoingDirtyReadsHeader(bool flag) noexcept {
    _potentialDirtyReads = flag;
  }

  /// @brief Flag, if the outgoing response should have an HTTP header
  /// indicating potential dirty reads:
  bool _potentialDirtyReads;
};
}  // namespace arangodb
