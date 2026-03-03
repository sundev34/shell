#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace REPL {

struct ReadContinueCode {
  std::vector<std::string> args;
};

struct ReadExitCode {};

using ReadReturnCode = std::variant<ReadContinueCode, ReadExitCode>;

ReadReturnCode read() {
  while (true) {
    std::cout << "$ ";

    std::string input;
    if (!std::getline(std::cin, input)) {
      return ReadExitCode{};
    }

    std::istringstream iss(input);
    std::vector<std::string> args;
    std::string token;
    while (iss >> token) {
      args.push_back(token);
    }

    if (!args.empty()) {
      return ReadContinueCode{std::move(args)};
    }
  }
}

struct UnknownCommandT {
  std::string reason;
};

struct EchoReturnT {
  std::string output;
};

struct ExitCommandT {
};

using EvaluateReturnT =
    std::variant<UnknownCommandT, EchoReturnT, ExitCommandT>;

EvaluateReturnT evaluate(const std::vector<std::string> &args) {
  const std::string &cmd = args[0];

  if (cmd == "exit") {
    return ExitCommandT{};
  }

  if (cmd == "echo") {
    std::string output;
    for (size_t i = 1; i < args.size(); ++i) {
      if (i > 1)
        output += ' ';
      output += args[i];
    }
    return EchoReturnT{std::move(output)};
  }

  return UnknownCommandT{cmd + ": command not found"};
}

void print(const EvaluateReturnT &result) {
  std::visit(
      [](auto &&state) {
        using T = std::decay_t<decltype(state)>;
        if constexpr (std::is_same_v<T, UnknownCommandT>) {
          std::cout << state.reason << '\n';
        } else if constexpr (std::is_same_v<T, EchoReturnT>) {
          std::cout << state.output << '\n';
        }
      },
      result);
}

} // namespace REPL

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    // read
    auto readResult = REPL::read();

    if (std::holds_alternative<REPL::ReadExitCode>(readResult))
      // check whether to exit
      break;

    auto &args = std::get<REPL::ReadContinueCode>(readResult).args; // grab args
    
    // evaulate args
    auto evalResult = REPL::evaluate(args);

    if (std::holds_alternative<REPL::ExitCommandT>(evalResult))
      break;

    // print result from evaluation
    REPL::print(evalResult);
  }

  return 0;
}
