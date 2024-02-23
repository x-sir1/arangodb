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

#include "Shell/arangosh.h"

namespace arangodb {

class ClientFeature;

class ShellConsoleFeature final : public ArangoshFeature {
 public:
  static constexpr std::string_view name() noexcept { return "Console"; }

  explicit ShellConsoleFeature(Server& server);

  void collectOptions(std::shared_ptr<options::ProgramOptions>) override final;
  void start() override final;
  void unprepare() override final;

  bool quiet() const { return _quiet; }
  void setQuiet(bool value) { _quiet = value; }
  bool colors() const { return _colors; }
  bool useHistory() const { return _useHistory; }
  bool autoComplete() const { return _autoComplete; }
  bool prettyPrint() const { return _prettyPrint; }
  bool pager() const { return _pager; }
  void setPager(bool value) { _pager = value; }
  std::string const& pagerCommand() const { return _pagerCommand; }
  std::string const& prompt() const { return _prompt; }

 private:
  bool _quiet;
  bool _colors;
  bool _useHistory;
  bool _autoComplete;
  bool _prettyPrint;
  std::string _auditFile;
  bool _pager;
  std::string _pagerCommand;
  std::string _prompt;

 public:
  void setPromptError(bool value) { _promptError = value; }
  void setSupportsColors(bool value) { _supportsColors = value; }
  void printWelcomeInfo();
  void printByeBye();
  std::string readPassword(std::string const& message);
  static std::string readPassword();
  void printContinuous(std::string const&);
  void printLine(std::string const&);
  void printErrorLine(std::string const&);
  void print(std::string const&);
  void openLog();
  void closeLog();
  void log(std::string const&);
  void flushLog();
  void setLastDuration(double duration) { _lastDuration = duration; }

  struct Prompt {
    std::string _plain;
    std::string _colored;
  };

  Prompt buildPrompt(ClientFeature*);
  void startPager();
  void stopPager();

 private:
  bool _promptError;
  bool _supportsColors;
  FILE* _toPager;
  FILE* _toAuditFile;
  // amount of time the last executed shell operation took
  double _lastDuration;
  // timestamp of startup time
  double const _startTime;
};

}  // namespace arangodb
