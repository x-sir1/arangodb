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
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Aql/ExecutionState.h"
#include "Aql/InputAqlItemRow.h"
#include "Aql/AqlFunctionsInternalCache.h"
#include "Aql/RegisterInfos.h"
#include "Aql/Stats.h"
#include "Aql/types.h"
#include "Transaction/Methods.h"

#include <vector>

namespace arangodb {
namespace transaction {
class Methods;
}

namespace aql {

struct AqlCall;
class AqlItemBlockInputRange;
class Expression;
class OutputAqlItemRow;
class QueryContext;
template<BlockPassthrough>
class SingleRowFetcher;
struct Variable;

struct CalculationExecutorInfos {
  CalculationExecutorInfos(
      RegisterId outputRegister, QueryContext& query, Expression& expression,
      std::vector<std::pair<VariableId, RegisterId>>&& expInVarToRegs);

  CalculationExecutorInfos() = delete;
  CalculationExecutorInfos(CalculationExecutorInfos&&) = default;
  CalculationExecutorInfos(CalculationExecutorInfos const&) = delete;
  ~CalculationExecutorInfos() = default;

  RegisterId getOutputRegisterId() const noexcept;

  QueryContext& getQuery() const noexcept;

  Expression& getExpression() const noexcept;

  std::vector<std::pair<VariableId, RegisterId>> const& getVarToRegs()
      const noexcept;

 private:
  RegisterId _outputRegisterId;

  QueryContext& _query;
  Expression& _expression;
  // Input variable and register pairs required for the expression
  std::vector<std::pair<VariableId, RegisterId>> _expVarToRegs;
};

enum class CalculationType {
  Condition,
#ifdef USE_V8
  V8Condition,
#endif
  Reference
};

template<CalculationType calculationType>
class CalculationExecutor {
 public:
  struct Properties {
    static constexpr bool preservesOrder = true;
    static constexpr BlockPassthrough allowsBlockPassthrough =
        BlockPassthrough::Enable;
  };
  using Fetcher = SingleRowFetcher<Properties::allowsBlockPassthrough>;
  using Infos = CalculationExecutorInfos;
  using Stats = NoStats;

  CalculationExecutor(Fetcher& fetcher, CalculationExecutorInfos&);
  ~CalculationExecutor();

  /**
   * @brief produce the next Row of Aql Values.
   *
   * @return ExecutorState, the stats, and a new Call that needs to be send to
   * upstream
   */
  [[nodiscard]] std::tuple<ExecutorState, Stats, AqlCall> produceRows(
      AqlItemBlockInputRange& inputRange, OutputAqlItemRow& output);

 private:
  // specialized implementations
  void doEvaluation(InputAqlItemRow& input, OutputAqlItemRow& output);

#ifdef USE_V8
  // Only for V8Conditions
  template<CalculationType U = calculationType,
           typename = std::enable_if_t<U == CalculationType::V8Condition>>
  void enterContext();

  // Only for V8Conditions
  template<CalculationType U = calculationType,
           typename = std::enable_if_t<U == CalculationType::V8Condition>>
  void exitContext() noexcept;
#endif

  [[nodiscard]] bool shouldExitContextBetweenBlocks() const noexcept;

 private:
  CalculationExecutorInfos& _infos;
  transaction::Methods _trx;
  aql::AqlFunctionsInternalCache _aqlFunctionsInternalCache;

  Fetcher& _fetcher;

  InputAqlItemRow _currentRow;
  ExecutionState _rowState;

  // true iff we entered a V8 executor and didn't exit it yet.
  // Necessary for owned executors, which will not be exited when we call
  // exitContext; but only for assertions in maintainer mode.
  bool _hasEnteredExecutor;
};

}  // namespace aql
}  // namespace arangodb
