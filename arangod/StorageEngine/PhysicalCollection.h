////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2022 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Basics/ReadWriteLock.h"
#include "Containers/MerkleTree.h"
#include "Futures/Future.h"
#include "Indexes/Index.h"
#include "RocksDBEngine/RocksDBReplicationContext.h"
#include "StorageEngine/ReplicationIterator.h"
#include "Utils/OperationResult.h"
#include "VocBase/Identifiers/IndexId.h"
#include "VocBase/Identifiers/RevisionId.h"
#include "VocBase/Identifiers/TransactionId.h"

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace arangodb {
namespace transaction {
class Methods;
}

namespace velocypack {
class Builder;
class Slice;
}  // namespace velocypack

class LocalDocumentId;
class Index;
class IndexIterator;
class LogicalCollection;
struct OperationOptions;
class Result;

class PhysicalCollection {
 public:
  constexpr static double defaultLockTimeout = 10.0 * 60.0;

  virtual ~PhysicalCollection() = default;

  // path to logical collection
  virtual std::string const& path() const = 0;

  // creation happens atm in engine->createCollection
  virtual Result updateProperties(velocypack::Slice slice) = 0;

  virtual RevisionId revision(transaction::Methods* trx) const = 0;

  /// @brief export properties
  virtual void getPropertiesVPack(velocypack::Builder&) const = 0;

  // @brief Return the number of documents in this collection
  virtual uint64_t numberDocuments(transaction::Methods* trx) const = 0;

  void close();

  void drop();

  /// recalculate counts for collection in case of failure, blocking
  virtual uint64_t recalculateCounts();

  /// @brief whether or not the collection contains any documents. this
  /// function is allowed to return true even if there are no documents
  virtual bool hasDocuments();

  ////////////////////////////////////
  // -- SECTION Indexes --
  ///////////////////////////////////

  /// @brief fetches current index selectivity estimates
  /// if allowUpdate is true, will potentially make a cluster-internal roundtrip
  /// to fetch current values!
  virtual IndexEstMap clusterIndexEstimates(bool allowUpdating,
                                            TransactionId tid);

  /// @brief flushes the current index selectivity estimates
  virtual void flushClusterIndexEstimates();

  virtual void prepareIndexes(velocypack::Slice indexesSlice);

  /// @brief determines order of index execution on collection
  struct IndexOrder {
    bool operator()(std::shared_ptr<Index> const& left,
                    std::shared_ptr<Index> const& right) const;
  };

  using IndexContainerType = std::set<std::shared_ptr<Index>, IndexOrder>;
  /// @brief find index by definition
  static std::shared_ptr<Index> findIndex(velocypack::Slice info,
                                          IndexContainerType const& indexes);
  /// @brief Find index by definition
  std::shared_ptr<Index> lookupIndex(velocypack::Slice info) const;

  /// @brief Find index by iid
  std::shared_ptr<Index> lookupIndex(IndexId idxId) const;

  /// @brief Find index by name
  std::shared_ptr<Index> lookupIndex(std::string_view idxName) const;

  /// @brief get list of all indexes
  std::vector<std::shared_ptr<Index>> getIndexes() const;

  virtual Index* primaryIndex() const;

  void getIndexesVPack(
      velocypack::Builder&,
      std::function<bool(Index const*,
                         std::underlying_type<Index::Serialize>::type&)> const&
          filter) const;

  /// @brief return the figures for a collection
  virtual futures::Future<OperationResult> figures(
      bool details, OperationOptions const& options);

  /// @brief create or restore an index
  /// @param restore utilize specified ID, assume index has to be created
  virtual std::shared_ptr<Index> createIndex(velocypack::Slice info,
                                             bool restore, bool& created) = 0;

  virtual Result dropIndex(IndexId iid);

  virtual std::unique_ptr<IndexIterator> getAllIterator(
      transaction::Methods* trx, ReadOwnWrites readOwnWrites) const = 0;
  virtual std::unique_ptr<IndexIterator> getAnyIterator(
      transaction::Methods* trx) const = 0;

  /// @brief Get an iterator associated with the specified replication batch
  virtual std::unique_ptr<ReplicationIterator> getReplicationIterator(
      ReplicationIterator::Ordering, uint64_t batchId);

  /// @brief Get an iterator associated with the specified transaction
  virtual std::unique_ptr<ReplicationIterator> getReplicationIterator(
      ReplicationIterator::Ordering, transaction::Methods&);

  virtual void adjustNumberDocuments(transaction::Methods&, int64_t);

  ////////////////////////////////////
  // -- SECTION DML Operations --
  ///////////////////////////////////

  virtual Result truncate(transaction::Methods& trx, OperationOptions& options,
                          bool& usedRangeDelete) = 0;

  /// @brief compact-data operation
  virtual void compact() {}

  /// @brief Defer a callback to be executed when the collection
  ///        can be dropped. The callback is supposed to drop
  ///        the collection and it is guaranteed that no one is using
  ///        it at that moment.
  virtual void deferDropCollection(
      std::function<bool(LogicalCollection&)> const& callback) = 0;

  virtual Result lookupKey(transaction::Methods*, std::string_view,
                           std::pair<LocalDocumentId, RevisionId>&,
                           ReadOwnWrites readOwnWrites) const = 0;

  virtual Result lookupKeyForUpdate(
      transaction::Methods*, std::string_view,
      std::pair<LocalDocumentId, RevisionId>&) const = 0;

  virtual Result read(transaction::Methods*, std::string_view key,
                      IndexIterator::DocumentCallback const& cb,
                      ReadOwnWrites readOwnWrites) const = 0;

  virtual Result read(transaction::Methods* trx, LocalDocumentId const& token,
                      IndexIterator::DocumentCallback const& cb,
                      ReadOwnWrites readOwnWrites) const = 0;

  virtual Result lookupDocument(transaction::Methods& trx,
                                LocalDocumentId token,
                                velocypack::Builder& builder, bool readCache,
                                bool fillCache,
                                ReadOwnWrites readOwnWrites) const = 0;

  virtual Result insert(transaction::Methods& trx, RevisionId newRevisionId,
                        velocypack::Slice newDocument,
                        OperationOptions const& options) = 0;

  virtual Result update(transaction::Methods& trx,
                        LocalDocumentId newDocumentId,
                        RevisionId previousRevisionId,
                        velocypack::Slice previousDocument,
                        RevisionId newRevisionId, velocypack::Slice newDocument,
                        OperationOptions const& options) = 0;

  virtual Result replace(transaction::Methods& trx,
                         LocalDocumentId newDocumentId,
                         RevisionId previousRevisionId,
                         velocypack::Slice previousDocument,
                         RevisionId newRevisionId,
                         velocypack::Slice newDocument,
                         OperationOptions const& options) = 0;

  virtual Result remove(transaction::Methods& trx,
                        LocalDocumentId previousDocumentId,
                        RevisionId previousRevisionId,
                        velocypack::Slice previousDocument,
                        OperationOptions const& options) = 0;

  virtual std::unique_ptr<containers::RevisionTree> revisionTree(
      transaction::Methods& trx);
  virtual std::unique_ptr<containers::RevisionTree> revisionTree(
      rocksdb::SequenceNumber trxSeq);
  virtual std::unique_ptr<containers::RevisionTree> computeRevisionTree(
      uint64_t batchId);

  virtual Result rebuildRevisionTree();

  virtual uint64_t placeRevisionTreeBlocker(TransactionId transactionId);
  virtual void removeRevisionTreeBlocker(TransactionId transactionId);

 protected:
  explicit PhysicalCollection(LogicalCollection& collection);

  // callback that is called directly before the index is dropped.
  // the write-lock on all indexes is still held. not called during
  // reocvery!
  virtual Result duringDropIndex(std::shared_ptr<Index> idx);

  // callback that is called directly after the index has been dropped.
  // no locks are held anymore.
  virtual Result afterDropIndex(std::shared_ptr<Index> idx);

  // callback that is called while adding a new index. called under
  // indexes write-lock
  virtual void duringAddIndex(std::shared_ptr<Index> idx);

  /// @brief Inject figures that are specific to StorageEngine
  virtual void figuresSpecific(bool details, velocypack::Builder&) = 0;

  LogicalCollection& _logicalCollection;

  mutable basics::ReadWriteLock _indexesLock;
  // current thread owning '_indexesLock'
  mutable std::atomic<std::thread::id> _indexesLockWriteOwner;
  IndexContainerType _indexes;
};

}  // namespace arangodb
