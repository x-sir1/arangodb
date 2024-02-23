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
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Basics/Result.h"
#include "Indexes/Index.h"
#include "Indexes/IndexIterator.h"
#include "RocksDBEngine/RocksDBIndex.h"
#include "VocBase/Identifiers/IndexId.h"
#include "VocBase/voc-types.h"
#include "VocBase/vocbase.h"

#include <set>
#include <string>

namespace arangodb {
class LocalDocumentId;

struct FulltextQueryToken {
  /// @brief fulltext query match options
  /// substring not implemented, maybe later
  enum MatchType { COMPLETE, PREFIX, SUBSTRING };
  /// @brief fulltext query logical operators
  enum Operation { AND, OR, EXCLUDE };

  FulltextQueryToken(std::string const& v, MatchType t, Operation o)
      : value(v), matchType(t), operation(o) {}

  std::string value;
  MatchType matchType;
  Operation operation;
};
/// A query consists of a list of tokens evaluated left to right:
/// An AND operation causes the entire result set on the left to
/// be intersected with every result containing the token.
/// Similarly an OR triggers union
typedef std::vector<FulltextQueryToken> FulltextQuery;

class RocksDBFulltextIndex final : public RocksDBIndex {
 public:
  RocksDBFulltextIndex() = delete;

  RocksDBFulltextIndex(IndexId iid, LogicalCollection& collection,
                       arangodb::velocypack::Slice info);

  ~RocksDBFulltextIndex() = default;

  IndexType type() const override { return Index::TRI_IDX_TYPE_FULLTEXT_INDEX; }

  char const* typeName() const override { return "fulltext"; }

  bool canBeDropped() const override { return true; }

  bool isSorted() const override { return false; }

  bool hasSelectivityEstimate() const override { return false; }

  void toVelocyPack(
      velocypack::Builder&,
      std::underlying_type<Index::Serialize>::type) const override;

  std::vector<std::vector<arangodb::basics::AttributeName>> const&
  coveredFields() const override {
    // index does not cover the index attribute!
    return Index::emptyCoveredFields;
  }

  bool matchesDefinition(VPackSlice const&) const override;

  bool isSame(std::string const& field, int minWordLength) const {
    std::string fieldString;
    TRI_AttributeNamesToString(fields()[0], fieldString);
    return (_minWordLength == minWordLength && fieldString == field);
  }

  std::unique_ptr<IndexIterator> iteratorForCondition(
      ResourceMonitor& monitor, transaction::Methods* trx,
      aql::AstNode const* node, aql::Variable const* reference,
      IndexIteratorOptions const& opts, ReadOwnWrites readOwnWrites,
      int) override;

  arangodb::Result parseQueryString(std::string const&, FulltextQuery&);
  Result executeQuery(transaction::Methods* trx, FulltextQuery const& query,
                      std::set<LocalDocumentId>& resultSet);

 protected:
  /// insert index elements into the specified write batch.
  Result insert(transaction::Methods& trx, RocksDBMethods* methods,
                LocalDocumentId documentId, velocypack::Slice doc,
                OperationOptions const& /*options*/,
                bool /*performChecks*/) override;

  /// remove index elements and put it in the specified write batch.
  Result remove(transaction::Methods& trx, RocksDBMethods* methods,
                LocalDocumentId documentId, velocypack::Slice doc,
                OperationOptions const& /*options*/) override;

 private:
  std::set<std::string> wordlist(arangodb::velocypack::Slice const&);

  /// @brief the indexed attribute (path)
  std::vector<std::string> _attr;

  /// @brief minimum word length
  int _minWordLength;

  arangodb::Result applyQueryToken(transaction::Methods* trx,
                                   FulltextQueryToken const&,
                                   std::set<LocalDocumentId>& resultSet);
};

}  // namespace arangodb
